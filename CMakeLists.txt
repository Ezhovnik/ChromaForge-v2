cmake_minimum_required(VERSION 3.21)
project(ChromaForge VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(ChromaForge_BUILD_TESTS "Build unit tests" OFF)
option(ChromaForge_BUILD_APPDIR "Package Linux build as AppDir" OFF)

add_subdirectory(src)
add_executable(ChromaForge src/main.cpp)

if(MINGW AND CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    target_link_options(ChromaForge PRIVATE
        "LINKER:--no-export-all-symbols"
        "LINKER:--export-all-symbols"
    )
endif()

target_link_libraries(ChromaForge
    PRIVATE
        ChromaForgeSrc
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
