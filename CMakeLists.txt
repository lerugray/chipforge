cmake_minimum_required(VERSION 3.22 FATAL_ERROR)

project(ChipForge VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# MSVC: increase compiler heap to handle JUCE's large template-heavy headers,
# and disable within-file parallel compilation (/MP) to avoid C1060 OOM errors.
if(MSVC)
    add_compile_options(/W4 /Zm300 /MP1)
else()
    add_compile_options(-Wall -Wextra)
endif()

# ============================================================
# Options
# ============================================================
option(CHIPFORGE_ENABLE_PERFETTO "Enable Melatonin Perfetto tracing" OFF)

# ============================================================
# JUCE
# ============================================================
add_subdirectory(libs/JUCE)

# ============================================================
# Melatonin Inspector (debug UI tool — debug builds only)
# ============================================================
juce_add_module(libs/melatonin_inspector)

# ============================================================
# Melatonin Perfetto (optional audio profiling)
# ============================================================
if(CHIPFORGE_ENABLE_PERFETTO)
    add_subdirectory(libs/melatonin_perfetto)
endif()

# ============================================================
# Embedded assets (fonts)
# ============================================================
juce_add_binary_data(ChipForgeAssets
    SOURCES
        assets/fonts/PressStart2P-Regular.ttf
        assets/fonts/PixelifySans-Regular.ttf
)

# ============================================================
# Main application target
# ============================================================
juce_add_gui_app(ChipForge
    PRODUCT_NAME "ChipForge"
    COMPANY_NAME "ChipForge"
    VERSION "0.1.0"
    BUNDLE_ID "com.chipforge.chipforge"
    MICROPHONE_PERMISSION_ENABLED FALSE
)

target_sources(ChipForge PRIVATE
    src/main.cpp
    src/App/ChipForgeApp.cpp
    src/App/MainWindow.cpp
    src/Data/Pattern.cpp
    src/Data/PatternCell.cpp
    src/Data/PatternEditorModel.cpp
    src/Engine/AudioEngine.cpp
    src/Engine/TrackerEngine.cpp
    src/Engine/TransportState.cpp
    src/UI/LookAndFeel/ChipForgeLookAndFeel.cpp
    src/UI/Shared/Toolbar.cpp
    src/UI/TrackerView/TrackerView.cpp
    src/Synths/NES2A03/NES2A03Synth.cpp
    # nes_snd_emu — compiled directly (no DLL; DLLEXPORT expands to nothing without NES_SND_DYNAMIC)
    libs/nes_snd_emu/nes_apu/Blip_Buffer.cpp
    libs/nes_snd_emu/nes_apu/Nes_Apu.cpp
    libs/nes_snd_emu/nes_apu/Nes_Oscs.cpp
)

target_include_directories(ChipForge PRIVATE
    src
    libs/nes_snd_emu   # exposes nes_apu/Nes_Apu.h, nes_apu/Blip_Buffer.h etc.
)

# NOMINMAX: avoid Windows min/max macro conflicts (needed by nes_snd_emu on MSVC)
if(MSVC)
    target_compile_definitions(ChipForge PRIVATE NOMINMAX)
endif()

target_compile_definitions(ChipForge PRIVATE
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_APPLICATION_NAME_STRING="$<TARGET_PROPERTY:ChipForge,JUCE_PRODUCT_NAME>"
    JUCE_APPLICATION_VERSION_STRING="$<TARGET_PROPERTY:ChipForge,JUCE_VERSION>"
    # Enable Melatonin Inspector in Debug builds
    $<$<CONFIG:Debug>:MELATONIN_INSPECTOR=1>
)

target_link_libraries(ChipForge PRIVATE
    ChipForgeAssets
    melatonin_inspector
    juce::juce_audio_utils
    juce::juce_audio_devices
    juce::juce_dsp
    juce::juce_gui_extra
    juce::juce_recommended_config_flags
)

if(CHIPFORGE_ENABLE_PERFETTO)
    target_link_libraries(ChipForge PRIVATE MelatoninPerfetto)
    target_compile_definitions(ChipForge PRIVATE PERFETTO=1)
endif()

# ============================================================
# Tests
# ============================================================
enable_testing()

juce_add_console_app(chipforge_tests
    PRODUCT_NAME "ChipForge Tests"
    COMPANY_NAME "ChipForge"
    VERSION "0.1.0"
)

target_sources(chipforge_tests PRIVATE
    tests/Main.cpp
    tests/NES2A03SynthTests.cpp
    tests/PatternEditorModelTests.cpp
    tests/PatternTests.cpp
    tests/TrackerEngineTests.cpp
    tests/TransportStateTests.cpp
    src/Data/Pattern.cpp
    src/Data/PatternCell.cpp
    src/Data/PatternEditorModel.cpp
    src/Engine/TrackerEngine.cpp
    src/Engine/TransportState.cpp
    src/Synths/NES2A03/NES2A03Synth.cpp
    libs/nes_snd_emu/nes_apu/Blip_Buffer.cpp
    libs/nes_snd_emu/nes_apu/Nes_Apu.cpp
    libs/nes_snd_emu/nes_apu/Nes_Oscs.cpp
)

target_include_directories(chipforge_tests PRIVATE
    src
    libs/nes_snd_emu
)

if(MSVC)
    target_compile_definitions(chipforge_tests PRIVATE NOMINMAX)
endif()

target_compile_definitions(chipforge_tests PRIVATE
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
)

target_link_libraries(chipforge_tests PRIVATE
    juce::juce_audio_basics
    juce::juce_core
    juce::juce_recommended_config_flags
)

add_test(NAME chipforge_tests COMMAND chipforge_tests)
