cmake_minimum_required(VERSION 3.20)

project(Forge6502 VERSION 0.1.0 LANGUAGES CXX)

include(CTest)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(Forge6502Core src/cpu6502.cpp)
add_library(Forge6502::Core ALIAS Forge6502Core)

target_include_directories(Forge6502Core PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

if(BUILD_TESTING AND PROJECT_IS_TOP_LEVEL)
    add_executable(Forge6502Tests
        tests/test_main.cpp
        tests/cpu_bus_cycle_tests.cpp
        tests/cpu_modularity_tests.cpp
        tests/klaus_functional_tests.cpp
        tests/ready_line_tests.cpp
    )
    target_link_libraries(Forge6502Tests PRIVATE Forge6502::Core)
    target_include_directories(Forge6502Tests PRIVATE tests/third_party)
    target_compile_definitions(Forge6502Tests PRIVATE
        FORGE6502_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}"
    )
    add_test(NAME Forge6502Tests COMMAND Forge6502Tests)

    if(MINGW)
        get_filename_component(FORGE6502_MINGW_BIN_DIR "${CMAKE_CXX_COMPILER}" DIRECTORY)
        foreach(runtimeDll libgcc_s_seh-1.dll libstdc++-6.dll libwinpthread-1.dll)
            add_custom_command(TARGET Forge6502Tests POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    "${FORGE6502_MINGW_BIN_DIR}/${runtimeDll}"
                    "$<TARGET_FILE_DIR:Forge6502Tests>"
            )
        endforeach()
    endif()
endif()
