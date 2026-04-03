#include "ChipForgeApp.h"

void ChipForgeApp::initialise(const juce::String& /*commandLine*/)
{
    juce::LookAndFeel::setDefaultLookAndFeel(&lookAndFeel);

    audioEngine = std::make_unique<AudioEngine>(transportState);
    mainWindow  = std::make_unique<MainWindow>(getApplicationName(),
                                               transportState,
                                               *audioEngine);
}

void ChipForgeApp::shutdown()
{
    mainWindow.reset();
    audioEngine.reset();
    juce::LookAndFeel::setDefaultLookAndFeel(nullptr);
}

void ChipForgeApp::systemRequestedQuit()
{
    quit();
}
