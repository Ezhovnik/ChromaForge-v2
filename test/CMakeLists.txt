project(ChromaForgeTest)

find_package(GTest QUIET)
if(NOT GTest_FOUND)
    message(STATUS "GTest not found - skipping unit tests")
    return()
endif()

file(GLOB_RECURSE sources CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)

add_executable(ChromaForgeTest ${sources})

target_link_libraries(ChromaForgeTest PRIVATE ChromaForgeSrc GTest::gtest_main
    $<$<PLATFORM_ID:Windows>:winmm>
    $<$<PLATFORM_ID:Windows>:ws2_32>
)

add_custom_command(
    TARGET ChromaForgeTest
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory_if_different
            ${CMAKE_SOURCE_DIR}/res ${CMAKE_CURRENT_BINARY_DIR}/res
    COMMENT "Copying resources for tests"
)

include(GoogleTest)
gtest_discover_tests(ChromaForgeTest)
