#pragma once

#include <juce_gui_extra/juce_gui_extra.h>

#include "Data/PatternEditorModel.h"
#include "Engine/AudioEngine.h"

class TrackerView final : public juce::Component
{
public:
    TrackerView(const PatternEditorModel& editorToShow, const AudioEngine& audioEngineToShow);

    void paint(juce::Graphics& g) override;

private:
    const PatternEditorModel& editor;
    const AudioEngine& audioEngine;

    static juce::String formatCell(const PatternCell& cell);
};
