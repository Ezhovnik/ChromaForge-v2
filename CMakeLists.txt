cmake_minimum_required(VERSION 3.21)
project(ChromaForge VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(PkgConfig REQUIRED)

find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(OpenAL REQUIRED)
find_package(ZLIB REQUIRED)
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
)

target_include_directories(ChromaForge PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/include
)

file(GLOB_RECURSE RESOURCE_FILES CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/res/*")
set(RESOURCE_STAMP "${CMAKE_CURRENT_BINARY_DIR}/res_copied.stamp")
add_custom_command(
    OUTPUT ${RESOURCE_STAMP}
    DEPENDS ${RESOURCE_FILES}
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/res"
        "$<TARGET_FILE_DIR:ChromaForge>/res"
    COMMAND ${CMAKE_COMMAND} -E touch ${RESOURCE_STAMP}
    COMMENT "Copying resources to $<TARGET_FILE_DIR:ChromaForge>"
)
add_custom_target(copy_resources DEPENDS ${RESOURCE_STAMP})
add_dependencies(ChromaForge copy_resources)

add_custom_command(TARGET ChromaForge POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E $<IF:$<CONFIG:Release>,copy_if_different,echo>
        $<$<CONFIG:Release>:$<TARGET_RUNTIME_DLLS:ChromaForge>>
        $<$<CONFIG:Release>:$<TARGET_FILE_DIR:ChromaForge>>
    COMMAND_EXPAND_LISTS
    COMMAND ${CMAKE_COMMAND} -E $<IF:$<CONFIG:Release>,echo,true>
        $<$<CONFIG:Release>:"Copied runtime DLLs to $<TARGET_FILE_DIR:ChromaForge>">
)
