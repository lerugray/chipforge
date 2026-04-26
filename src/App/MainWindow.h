#pragma once

#include <juce_gui_extra/juce_gui_extra.h>
#include "Engine/AudioEngine.h"
#include "Engine/TransportState.h"
#include "Data/PatternEditorModel.h"
#include "UI/LookAndFeel/ChipForgeLookAndFeel.h"
#include "UI/Shared/Toolbar.h"
#include "UI/TrackerView/TrackerView.h"

#if MELATONIN_INSPECTOR
  #include "melatonin_inspector/melatonin_inspector.h"
#endif

//==============================================================================
// MainContentComponent
//
// Root component hosted inside the DocumentWindow.
// Phase 1: Toolbar at top + QWERTY keyboard note input + placeholder panel.
//==============================================================================
class MainContentComponent : public juce::Component,
                              private juce::KeyListener,
                              private juce::Timer
{
public:
    MainContentComponent(TransportState& transport, AudioEngine& audio);
    ~MainContentComponent() override;

    void resized() override;
    void paint(juce::Graphics&) override;

    bool keyPressed(const juce::KeyPress& key, juce::Component* originatingComponent) override;
    bool keyStateChanged(bool isKeyDown, juce::Component* originatingComponent) override;
    void timerCallback() override;

private:
    TransportState& transportState;
    AudioEngine&    audioEngine;

    PatternEditorModel patternEditor;
    Toolbar toolbar;
    TrackerView trackerView;

    int activeKeyCode  { -1 };  // key code of the currently held note key
    bool spaceKeyDown  { false };

    void playEditedPattern();
    void stopTransport();
    void stopTransportForEdit();
    void stopAuditionNote();
    void repaintTracker();

#if MELATONIN_INSPECTOR
    std::unique_ptr<melatonin::Inspector> inspector;
#endif

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainContentComponent)
};

//==============================================================================
// MainWindow
//==============================================================================
class MainWindow : public juce::DocumentWindow
{
public:
    explicit MainWindow(const juce::String& name,
                        TransportState& transport,
                        AudioEngine& audio);

    void closeButtonPressed() override;

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainWindow)
};
