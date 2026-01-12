# No necesita project() aquí
set(CMAKE_CXX_STANDARD 11)  # use when testing

# Fuentes de tests (puedes usar GLOB o listarlos manualmente)
file(GLOB TEST_SOURCES "*.cpp")

# Executable
add_executable(unit_tests ${TEST_SOURCES})

# Important: mismo include que el servidor + catch2
target_include_directories(unit_tests PRIVATE
        ${CMAKE_SOURCE_DIR}/src/config
        ${CMAKE_SOURCE_DIR}/lib/catch2
)

# Si más adelante tienes muchas clases → puedes crear una lib estática/objetos
# (muy buena práctica en Webserv para evitar duplicar compilación)
# Por ahora con GLOB está bien para empezar

# Opcional: flags de warnings/debug
#if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
#    target_compile_options(unit_tests PRIVATE -Wall -Wextra -Werror -g)
#endif()

# Añade tests a CTest (y CLion los detecta automáticamente)
#include(CTest)
#include(Catch)
# si usas Catch2 v3+ → si no, omítelo o copia el .cmake de Catch2
#catch_discover_tests(unit_tests)

# Si usas v2 (single-header) y #define CATCH_CONFIG_MAIN → no necesitas Catch.cmake