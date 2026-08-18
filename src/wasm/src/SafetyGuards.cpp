// Implementação de SafetyGuards.h.
//
// As funções de sanitização são todas `constexpr`/`inline` e residem no
// cabeçalho por serem operações triviais chamadas em hot-path (a cada
// update() de sistema, a cada frame). Este .cpp existe como unidade de
// tradução âncora do módulo `SafetyGuards` dentro do CMakeLists (biblioteca
// e195e2_systems_core) e é o local reservado para futuras funções de
// sanitização não-triviais (ex: validação cruzada entre múltiplos sensores
// redundantes do FBW) que não se prestem a inline.
#include "SafetyGuards.h"

namespace e195e2::safety {

// Âncora de versão do módulo de segurança — incrementar ao alterar
// contratos de SanitizeDouble/RateLimit/SanitizeDeltaTime de forma que
// quebre compatibilidade com sistemas já integrados.
//
// static_assert abaixo "consome" a constante em tempo de compilação para
// que ela não dispare -Wunused-variable sob -Werror (build seguro exige
// zero warnings silenciados via atributo — preferimos uso real a
// [[maybe_unused]]).
constexpr std::uint32_t kSafetyGuardsAbiVersion = 1;
static_assert(kSafetyGuardsAbiVersion > 0, "SafetyGuards ABI version deve ser positiva.");

} // namespace e195e2::safety
