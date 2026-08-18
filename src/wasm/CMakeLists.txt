# Ref: MSFS 2024 SDK - WASM Module Build (Core de Sistemas)
#
# Este CMakeLists compila o núcleo de sistemas físicos (Elétrico, Hidráulico,
# Combustível, Pneumático, Fly-By-Wire, FADEC, Trem de Pouso) para WebAssembly,
# a ser carregado pelo MSFS como módulo de simulação em tempo real.
#
# Regra de conduta do projeto: nenhuma flag de compilação "silenciosa" — todo
# warning é tratado como erro (-Werror), forçando que qualquer comportamento
# indefinido ou conversão perigosa seja resolvido antes de chegar em produção
# (dado que este código calcula variáveis físicas reais usadas no flight loop).

cmake_minimum_required(VERSION 3.20)

project(e195e2_systems_core
    VERSION 0.1.0
    DESCRIPTION "Embraer E195-E2 - Core de Sistemas (WASM)"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# --- Flags de compilação seguras (Clang/GCC) --------------------------------
# -Wall -Wextra: cobertura ampla de warnings.
# -Werror: warning vira erro de build — nenhum aviso passa despercebido.
# -pedantic: exige conformidade estrita com o padrão C++, sem extensões
#            proprietárias do compilador que possam gerar comportamento
#            diferente entre toolchains (crítico para reprodutibilidade
#            do cálculo físico entre máquinas de desenvolvimento distintas).
set(E195E2_SAFE_COMPILE_FLAGS
    -Wall
    -Wextra
    -Werror
    -pedantic
    -Wshadow
    -Wconversion
    -Wsign-conversion
    -Wnon-virtual-dtor
    -Woverloaded-virtual
)

# --- Alvo: módulo WASM do core de sistemas ----------------------------------
add_library(e195e2_systems_core STATIC
    src/SafetyGuards.cpp
    src/ElectricalSystem.cpp
    src/HydraulicSystem.cpp
    src/FuelSystem.cpp
    src/PneumaticSystem.cpp
    src/PowerplantSystem.cpp
    src/APUSystem.cpp
    src/LandingGearSystem.cpp
    src/FlyByWire.cpp
    src/SimBus.cpp
    src/SimVarPublisher.cpp
)

target_include_directories(e195e2_systems_core
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_compile_options(e195e2_systems_core PRIVATE ${E195E2_SAFE_COMPILE_FLAGS})

# --- Sanitizers em build de desenvolvimento ---------------------------------
# Em Debug, habilita UBSan/ASan para pegar overflow, divisão por zero e
# acesso de memória inválido em tempo de execução — essencial num core que
# processa dados de voo continuamente a cada frame.
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_options(e195e2_systems_core PUBLIC
        -fsanitize=address,undefined
        -fno-omit-frame-pointer
    )
    target_link_options(e195e2_systems_core PUBLIC
        -fsanitize=address,undefined
    )
endif()

# --- Testes (GoogleTest) ----------------------------------------------------
# Suíte de testes unitários será conectada aqui na Fase 1 (ver TaskCreate
# "Configurar testes unitários com GoogleTest e Vitest no pipeline de
# segurança" — pendente de confirmação do usuário).
option(E195E2_BUILD_TESTS "Compilar suíte de testes GoogleTest" OFF)
if(E195E2_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
