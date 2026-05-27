cmake_minimum_required(VERSION 3.21)
project(ChromaForge VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(PkgConfig REQUIRED)

find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(OpenAL REQUIRED)
find_package(ZLIB REQUIRED)
find_package(CURL REQUIRED)
find_package(fmt CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(entt CONFIG REQUIRED)

pkg_check_modules(VORBIS REQUIRED IMPORTED_TARGET vorbis vorbisfile vorbisenc)
pkg_check_modules(LUAJIT REQUIRED IMPORTED_TARGET luajit)

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS "src/*.cpp")
add_executable(ChromaForge ${SOURCES})

target_link_libraries(ChromaForge
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
        $<$<PLATFORM_ID:Windows>:winmm>
        $<$<PLATFORM_ID:Windows>:ws2_32>
)

target_include_directories(ChromaForge PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/include
)

file(GLOB_RECURSE RESOURCE_FILES CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/res/*")

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
