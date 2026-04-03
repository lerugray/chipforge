#pragma once

#include <juce_gui_extra/juce_gui_extra.h>
#include "MainWindow.h"
#include "Engine/TransportState.h"
#include "Engine/AudioEngine.h"
#include "UI/LookAndFeel/ChipForgeLookAndFeel.h"

//==============================================================================
// ChipForgeApp
//
// Top-level JUCE application class. Owns the global state objects and the main
// window. Lifetime: from startup to shutdown.
//==============================================================================
class ChipForgeApp : public juce::JUCEApplication
{
public:
    ChipForgeApp() = default;

    const juce::String getApplicationName() override
    {
        return JUCE_APPLICATION_NAME_STRING;
    }

    const juce::String getApplicationVersion() override
    {
        return JUCE_APPLICATION_VERSION_STRING;
    }

    bool moreThanOneInstanceAllowed() override { return false; }

    void initialise(const juce::String& commandLine) override;
    void shutdown() override;
    void systemRequestedQuit() override;

private:
    ChipForgeLookAndFeel lookAndFeel;
    TransportState       transportState;
    std::unique_ptr<AudioEngine>  audioEngine;
    std::unique_ptr<MainWindow>   mainWindow;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ChipForgeApp)
};
