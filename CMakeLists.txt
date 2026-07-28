cmake_minimum_required(VERSION 3.21)
project(ChromaForge VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(ChromaForge_BUILD_TESTS "Build unit tests" OFF)
option(ChromaForge_BUILD_APPDIR "Package Linux build as AppDir" OFF)

# ---- find all upstream packages at top level ----
find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    find_package(OpenAL CONFIG REQUIRED)
else()
    find_package(OpenAL REQUIRED)
endif()
find_package(ZLIB REQUIRED)
find_package(CURL REQUIRED)
find_package(fmt CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(PkgConfig REQUIRED)

pkg_check_modules(VORBIS REQUIRED IMPORTED_TARGET vorbis vorbisfile vorbisenc)
pkg_check_modules(LUAJIT REQUIRED IMPORTED_TARGET luajit)

# ---- LLD doesn't support -Wl,-E (short for --export-all-symbols on PE/COFF) ----
if(MINGW AND CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    get_target_property(_luajit_opts PkgConfig::LUAJIT INTERFACE_LINK_OPTIONS)
    if(_luajit_opts)
        string(REPLACE "-Wl,-E" "-Wl,--export-all-symbols" _luajit_opts "${_luajit_opts}")
        set_target_properties(PkgConfig::LUAJIT PROPERTIES INTERFACE_LINK_OPTIONS "${_luajit_opts}")
    endif()
endif()

# ---- build targets ----
add_subdirectory(src)
add_executable(ChromaForge src/main.cpp)

target_link_libraries(ChromaForge
    PRIVATE
        ChromaForgeSrc
        fmt::fmt
        spdlog::spdlog
        glfw
        PkgConfig::VORBIS
        PkgConfig::LUAJIT
        CURL::libcurl
        OpenGL::GL
        GLEW::GLEW
        OpenAL::OpenAL
        ZLIB::ZLIB
        $<$<PLATFORM_ID:Windows>:winmm>
        $<$<PLATFORM_ID:Windows>:ws2_32>
)

add_custom_target(copy_resources ALL
    COMMAND ${CMAKE_COMMAND} -E remove_directory "$<TARGET_FILE_DIR:ChromaForge>/res"
    COMMAND ${CMAKE_COMMAND} -E remove_directory "$<TARGET_FILE_DIR:ChromaForge>/content"
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/res"
        "$<TARGET_FILE_DIR:ChromaForge>/res"
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/res/content"
        "$<TARGET_FILE_DIR:ChromaForge>/content"
    COMMAND ${CMAKE_COMMAND} -E remove_directory
        "$<TARGET_FILE_DIR:ChromaForge>/res/content"
    COMMENT "Copying resources to build directory"
)
add_dependencies(ChromaForge copy_resources)

if(ChromaForge_BUILD_APPDIR)
    include(${CMAKE_CURRENT_SOURCE_DIR}/dev/cmake/BuildAppdir.cmake)
endif()

if(ChromaForge_BUILD_TESTS)
    enable_testing()
    add_subdirectory(test)
    add_subdirectory(vctest)
endif()
