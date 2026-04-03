#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <functional>

//==============================================================================
// Toolbar
//
// Top bar of the main window. Contains:
//   - Play / Stop transport buttons
//   - BPM display label
//   - View-switcher tabs (F1 Tracker | F2 Mixer | F3 Arrange) — stubs for Phase 0
//
// Callbacks are used instead of listeners to avoid coupling to engine types.
//==============================================================================
class Toolbar : public juce::Component
{
public:
    static constexpr int Height = 48;

    std::function<void()> onPlayClicked;
    std::function<void()> onStopClicked;

    Toolbar();

    void setIsPlaying(bool playing);
    void setBpm(int bpm);

    void resized() override;
    void paint(juce::Graphics&) override;

private:
    juce::TextButton playButton  { "PLAY" };
    juce::TextButton stopButton  { "STOP" };

    juce::TextButton trackerTab  { "F1 TRACKER" };
    juce::TextButton mixerTab    { "F2 MIXER" };
    juce::TextButton arrangeTab  { "F3 ARRANGE" };

    juce::Label bpmLabel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Toolbar)
};
