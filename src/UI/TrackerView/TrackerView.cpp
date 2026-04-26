#include "TrackerView.h"

#include "UI/LookAndFeel/ChipForgeLookAndFeel.h"

TrackerView::TrackerView(const Pattern& patternToShow)
    : pattern(patternToShow)
{
}

void TrackerView::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(ChipForgeLookAndFeel::ColourBackground));

    auto area = getLocalBounds().reduced(24, 18);
    g.setColour(juce::Colour(ChipForgeLookAndFeel::ColourTextPrimary));
    g.setFont(18.0f);
    g.drawText("TRACKER", area.removeFromTop(28), juce::Justification::centredLeft);

    g.setFont(13.0f);
    g.setColour(juce::Colour(ChipForgeLookAndFeel::ColourTextSecondary));
    g.drawText("Play starts the demo pattern; QWERTY keys still audition the NES pulse synth.",
               area.removeFromTop(24),
               juce::Justification::centredLeft);

    area.removeFromTop(10);

    const int rowLabelWidth = 54;
    const int rowHeight = 24;
    const int headerHeight = 26;
    const int trackWidth = juce::jmax(80, (area.getWidth() - rowLabelWidth) / pattern.getNumTracks());

    auto headerArea = area.removeFromTop(headerHeight);
    headerArea.removeFromLeft(rowLabelWidth);

    for (int track = 0; track < pattern.getNumTracks(); ++track)
    {
        auto cellArea = headerArea.removeFromLeft(trackWidth).reduced(2);
        g.setColour(juce::Colour(ChipForgeLookAndFeel::ColourActive));
        g.drawRect(cellArea);
        g.setColour(juce::Colour(ChipForgeLookAndFeel::ColourTextPrimary));
        g.drawText("TRK " + juce::String(track + 1), cellArea, juce::Justification::centred);
    }

    for (int row = 0; row < pattern.getNumRows(); ++row)
    {
        auto rowArea = area.removeFromTop(rowHeight);
        if (rowArea.isEmpty())
            break;

        const auto rowColour = (row % 4 == 0)
            ? juce::Colour(ChipForgeLookAndFeel::ColourPanel).brighter(0.08f)
            : juce::Colour(ChipForgeLookAndFeel::ColourPanel);

        g.setColour(rowColour);
        g.fillRect(rowArea);

        g.setColour(juce::Colour(ChipForgeLookAndFeel::ColourTextSecondary));
        g.drawText(juce::String::formatted("%02X", row),
                   rowArea.removeFromLeft(rowLabelWidth),
                   juce::Justification::centred);

        for (int track = 0; track < pattern.getNumTracks(); ++track)
        {
            auto cellArea = rowArea.removeFromLeft(trackWidth).reduced(2);
            const auto& cell = pattern.getCell(row, track);

            g.setColour(cell.hasNote() || cell.isNoteOff()
                ? juce::Colour(ChipForgeLookAndFeel::ColourNoteOn)
                : juce::Colour(ChipForgeLookAndFeel::ColourTextSecondary));
            g.drawText(formatCell(cell), cellArea, juce::Justification::centred);
        }
    }
}

juce::String TrackerView::formatCell(const PatternCell& cell)
{
    if (cell.isNoteOff())
        return "===";

    if (!cell.hasNote())
        return "---";

    static constexpr const char* noteNames[] {
        "C-", "C#", "D-", "D#", "E-", "F-", "F#", "G-", "G#", "A-", "A#", "B-"
    };

    const int midiNote = static_cast<int>(cell.note);
    const int octave = midiNote / 12 - 1;
    return juce::String(noteNames[midiNote % 12]) + juce::String(octave);
}
