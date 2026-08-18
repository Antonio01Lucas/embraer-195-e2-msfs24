# Ref: Regra de Conduta do Projeto - build seguro e verificável.
#
# Suíte de testes unitários do core de sistemas. Baixa o GoogleTest via
# FetchContent (padrão moderno de CMake) — requer acesso de rede ao GitHub
# no momento do configure. Em CI (.github/workflows/security-audit.yml)
# isso é normal; em ambiente de desenvolvimento local, a primeira
# configuração também precisa de rede (downloads subsequentes usam cache
# local do CMake).

include(FetchContent)
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG        v1.14.0
)
# Evita que o GoogleTest sobrescreva as flags de compilação/linkagem do
# projeto pai (ex: força uso da mesma runtime library em todas as
# plataformas) — recomendação oficial do FetchContent para GoogleTest.
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

add_executable(e195e2_systems_core_tests
    SafetyGuardsTests.cpp
    ElectricalSystemTests.cpp
    HydraulicSystemTests.cpp
    FuelSystemTests.cpp
    PneumaticSystemTests.cpp
    PowerplantSystemTests.cpp
    APUSystemTests.cpp
    LandingGearSystemTests.cpp
    FlyByWireTests.cpp
    SimBusTests.cpp
    SimVarPublisherTests.cpp
)

target_link_libraries(e195e2_systems_core_tests
    PRIVATE
        e195e2_systems_core
        GTest::gtest_main
)

target_compile_options(e195e2_systems_core_tests PRIVATE ${E195E2_SAFE_COMPILE_FLAGS})

include(GoogleTest)
gtest_discover_tests(e195e2_systems_core_tests)
