# GoogleTest (vcpkg: gtest). See root CMake and vcpkg.json.
# cf-001: gtest + smoke. cf-002: chipforge_core (TwoPole) + JUCE, effect tests.
# Configure: cmake -B build -DCMAKE_TOOLCHAIN_FILE=[vcpkg]/scripts/buildsystems/vcpkg.cmake

cmake_minimum_required(VERSION 3.22 FATAL_ERROR)

if(NOT TARGET juce::juce_dsp)
    message(FATAL_ERROR "tests/ must be added after libs/JUCE in the root project.")
endif()

enable_testing()
include(GoogleTest)

find_package(GTest CONFIG REQUIRED)

add_library(chipforge_core STATIC
    ${CMAKE_SOURCE_DIR}/src/Effects/TwoPoleResonantLowPass.cpp
)
target_include_directories(chipforge_core PUBLIC
    ${CMAKE_SOURCE_DIR}/src
)
target_link_libraries(chipforge_core PUBLIC
    juce::juce_audio_basics
    juce::juce_dsp
    juce::juce_core
    juce::juce_recommended_config_flags
)
if(MSVC)
    target_compile_definitions(chipforge_core PRIVATE NOMINMAX)
endif()

add_executable(chipforge_tests
    test_smoke.cpp
    effects/test_two_pole_lowpass.cpp
)
target_link_libraries(chipforge_tests PRIVATE
    chipforge_core
    GTest::gtest
    GTest::gtest_main
    juce::juce_audio_basics
    juce::juce_dsp
    juce::juce_core
    juce::juce_recommended_config_flags
)
if(MSVC)
    target_compile_definitions(chipforge_tests PRIVATE NOMINMAX)
endif()

target_compile_definitions(chipforge_tests PRIVATE
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
)

gtest_discover_tests(chipforge_tests
    DISCOVERY_MODE PRE_TEST
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)
