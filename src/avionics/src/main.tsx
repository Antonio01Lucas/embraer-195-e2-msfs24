// Ponto de entrada da suite de aviônicos Honeywell Primus Epic 2.0.
// Fase atual: placeholder de bootstrap — os componentes reais de cada DU
// (PFD, MFD, EICAS/Synoptics) serão adicionados na Fase 2 do roadmap
// (ver Projeto: roadmap-desenvolvimento-fases.md).
import { StrictMode } from "react";
import { createRoot } from "react-dom/client";

function E195E2Root(): JSX.Element {
  // TODO(Fase 2 - ATA 31): renderizar os 5 DUs (PFD L/R, MFD L/R, EICAS)
  // conforme a página atribuída via parâmetro do panel.xml.
  return <div className="e195e2-placeholder">E195-E2 Avionics — bootstrap</div>;
}

const container = document.getElementById("e195e2-root");
if (container === null) {
  throw new Error("e195e2-root não encontrado no DOM do painel.");
}

createRoot(container).render(
  <StrictMode>
    <E195E2Root />
  </StrictMode>,
);
