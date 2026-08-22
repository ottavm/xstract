cmake_minimum_required(VERSION 3.16)

project(
    xstract
    VERSION "1.0"
    SPDX_LICENSE "BSD-3-Clause"
    HOMEPAGE_URL "github.com/ottavm/xstract"
    LANGUAGES C
)

set(PROJECT_AUTHOR "Murilo Reis <github.com/ottavm>")
configure_file(config.h.in config.h @ONLY)

add_executable(xstract)

target_sources(xstract
    PRIVATE
        src/main.c
        src/extract.c
)

target_include_directories(xstract
    PRIVATE
        "${CMAKE_CURRENT_BINARY_DIR}"
        "${CMAKE_SOURCE_DIR}/include"
)

set_target_properties(xstract
    PROPERTIES
        C_STANDARD 99
        C_STANDARD_REQUIRED ON
        C_EXTENSIONS ON
)

target_compile_options(xstract
    PRIVATE
        "-Wall"
        "-Wextra"
)

