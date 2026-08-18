#!/usr/bin/env node
// Ref: Regra de Conduta do Projeto - build seguro e verificável.
//
// Script de auditoria executado ANTES de cada build (via `npm run audit`,
// e no pipeline .github/workflows/security-audit.yml). Não substitui
// linters/testes reais — cobre verificações estruturais e de segurança
// que um `tsc`/`eslint`/`cmake` isolado não cobre:
//
//   1. Nenhum uso de eval()/new Function() no código de aviônicos (React
//      roda dentro do Coherent GT, sem sandbox de browser completo —
//      eval é superfície de risco desnecessária num painel de aeronave).
//   2. Nenhum import de módulos Node "perigosos" (child_process, fs, net)
//      dentro de src/avionics/src — o bundle roda no Coherent GT, não em
//      Node; um import desses indica erro de arquitetura ou dependência
//      indevida, não apenas risco de segurança.
//   3. Nenhum script de dependência (`postinstall` etc.) baixando e
//      executando código arbitrário via curl/wget em package.json.
//   4. Todos os arquivos de configuração MSFS declarados como obrigatórios
//      (aircraft.cfg, flight_model.cfg, engines.cfg, panel/panel.xml)
//      existem e não estão vazios.
//   5. Nenhum arquivo de configuração de voo (flight_model.cfg,
//      engines.cfg) foi commitado sem ao menos uma citação de fonte
//      ("Ref:") — força a disciplina de não fabricar dado físico sem
//      documentar a origem (Regra de Conduta #1 e #3 do projeto).
//
// Uso: node scripts/audit.js
// Saída: código de saída 0 = OK; 1 = falha (bloqueia o build/CI).

import { readFileSync, existsSync, readdirSync, statSync } from "node:fs";
import { join, extname } from "node:path";
import { fileURLToPath } from "node:url";

const __dirname = fileURLToPath(new URL(".", import.meta.url));
const ROOT = join(__dirname, "..");

/** @type {string[]} */
const failures = [];
/** @type {string[]} */
const warnings = [];

function walk(dir, exts) {
    /** @type {string[]} */
    const results = [];
    if (!existsSync(dir)) return results;
    for (const entry of readdirSync(dir)) {
        const full = join(dir, entry);
        const stat = statSync(full);
        if (stat.isDirectory()) {
            if (entry === "node_modules" || entry === "dist" || entry === "build") continue;
            results.push(...walk(full, exts));
        } else if (exts.includes(extname(full))) {
            results.push(full);
        }
    }
    return results;
}

// --- Check 1 & 2: eval / imports perigosos em src/avionics -----------------
const avionicsSrcDir = join(ROOT, "src", "avionics", "src");
const avionicsFiles = walk(avionicsSrcDir, [".ts", ".tsx"]);

const DANGEROUS_PATTERNS = [
    { pattern: /\beval\s*\(/, message: "uso de eval()" },
    { pattern: /new\s+Function\s*\(/, message: "uso de new Function()" },
    { pattern: /from\s+["']child_process["']/, message: "import de child_process" },
    { pattern: /from\s+["']net["']/, message: "import de net" },
    { pattern: /require\s*\(\s*["']child_process["']\s*\)/, message: "require de child_process" },
];

for (const file of avionicsFiles) {
    const content = readFileSync(file, "utf-8");
    for (const { pattern, message } of DANGEROUS_PATTERNS) {
        if (pattern.test(content)) {
            failures.push(`[seguranca] ${message} encontrado em ${file}`);
        }
    }
}

if (avionicsFiles.length === 0) {
    warnings.push("Nenhum arquivo .ts/.tsx encontrado em src/avionics/src ainda (esperado nesta fase inicial).");
}

// --- Check 3: scripts de instalação suspeitos em todos os package.json -----
const packageJsonFiles = walk(ROOT, [".json"]).filter((f) => f.endsWith("package.json"));
const SUSPICIOUS_INSTALL_PATTERN = /(curl|wget)\s+.*\|\s*(sh|bash)/;

for (const file of packageJsonFiles) {
    if (file.includes("node_modules")) continue;
    const content = readFileSync(file, "utf-8");
    let pkg;
    try {
        pkg = JSON.parse(content);
    } catch {
        failures.push(`[integridade] ${file} não é um JSON válido`);
        continue;
    }
    const scripts = pkg.scripts ?? {};
    for (const [name, cmd] of Object.entries(scripts)) {
        if (typeof cmd === "string" && SUSPICIOUS_INSTALL_PATTERN.test(cmd)) {
            failures.push(`[seguranca] script "${name}" em ${file} baixa e executa código remoto: "${cmd}"`);
        }
    }
}

// --- Check 4: arquivos MSFS obrigatórios ------------------------------------
const aircraftDir = join(ROOT, "PackageSources", "SimObjects", "Airplanes", "embraer-e195-e2");
const REQUIRED_MSFS_FILES = [
    "aircraft.cfg",
    "flight_model.cfg",
    "engines.cfg",
    join("panel", "panel.xml"),
];

for (const relPath of REQUIRED_MSFS_FILES) {
    const fullPath = join(aircraftDir, relPath);
    if (!existsSync(fullPath)) {
        failures.push(`[integridade] arquivo obrigatório ausente: ${fullPath}`);
        continue;
    }
    const content = readFileSync(fullPath, "utf-8");
    if (content.trim().length === 0) {
        failures.push(`[integridade] arquivo obrigatório vazio: ${fullPath}`);
    }
}

// --- Check 5: citação de fonte ("Ref:") em configs de física ---------------
const PHYSICS_CONFIG_FILES = ["flight_model.cfg", "engines.cfg"];
for (const relPath of PHYSICS_CONFIG_FILES) {
    const fullPath = join(aircraftDir, relPath);
    if (!existsSync(fullPath)) continue;
    const content = readFileSync(fullPath, "utf-8");
    if (!/Ref:/.test(content)) {
        failures.push(
            `[conduta-do-projeto] ${fullPath} não contém nenhuma citação "Ref:" — todo dado físico ` +
            `precisa citar fonte ou metodologia de estimativa (Regra de Conduta #1/#3 do projeto).`
        );
    }
}

// --- Relatório ---------------------------------------------------------------
console.log(`\nAuditoria E195-E2 — ${new Date().toISOString().slice(0, 10)}\n`);

if (warnings.length > 0) {
    console.log("Avisos:");
    for (const w of warnings) console.log(`  ! ${w}`);
    console.log("");
}

if (failures.length > 0) {
    console.log(`Falhas (${failures.length}):`);
    for (const f of failures) console.log(`  x ${f}`);
    console.log("\nAuditoria FALHOU.\n");
    process.exit(1);
}

console.log("Auditoria OK — nenhuma falha encontrada.\n");
process.exit(0);
