#pragma once

#include <juce_gui_extra/juce_gui_extra.h>

#include "Data/Pattern.h"

class TrackerView final : public juce::Component
{
public:
    explicit TrackerView(const Pattern& patternToShow);

    void paint(juce::Graphics& g) override;

private:
    const Pattern& pattern;

    static juce::String formatCell(const PatternCell& cell);
};
