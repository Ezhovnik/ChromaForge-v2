project(ChromaForgeSrc)

file(GLOB_RECURSE headers CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/*.h ${CMAKE_CURRENT_SOURCE_DIR}/*.hpp)
file(GLOB_RECURSE sources CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
list(REMOVE_ITEM sources ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp)

add_library(ChromaForgeSrc STATIC ${sources} ${headers})

target_include_directories(ChromaForgeSrc PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)
target_include_directories(ChromaForgeSrc SYSTEM PUBLIC
    ${CMAKE_SOURCE_DIR}/include
)

find_package(EnTT CONFIG QUIET)
find_package(entt CONFIG QUIET)
if(NOT EnTT_FOUND AND NOT entt_FOUND)
    message(STATUS "EnTT CMake config not found, assuming compiler finds headers directly")
endif()

target_link_libraries(ChromaForgeSrc
    PRIVATE
        OpenGL::GL
        GLEW::GLEW
        OpenAL::OpenAL
        ZLIB::ZLIB
        fmt::fmt
        spdlog::spdlog
        glfw
        PkgConfig::VORBIS
        PkgConfig::LUAJIT
        CURL::libcurl
)

target_compile_options(ChromaForgeSrc
    PUBLIC
    $<$<CXX_COMPILER_ID:MSVC>:
        /utf-8
        /W4
        /wd4244
        /wd4267
        /wd4100
        /wd4457
        /wd4458
        /wd4459
        /wd4101
    >
    $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:
        -Wall
        -Wextra
        -Wformat-nonliteral
        -Wcast-align
        -Wpointer-arith
        -Wundef
        -Wwrite-strings
        -Wno-unused-parameter
        -Wno-sign-compare
        -Wno-unknown-pragmas
    >
    $<$<CXX_COMPILER_ID:GNU>:
        -Wduplicated-branches
        -Wduplicated-cond
    >)
