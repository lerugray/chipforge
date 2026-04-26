#pragma once

#include "Pattern.h"

#include <optional>

class PatternEditorModel
{
public:
    static constexpr int DefaultRows   { 16 };
    static constexpr int DefaultTracks { Pattern::DefaultTracks };
    static constexpr int MinOctave     { 0 };
    static constexpr int MaxOctave     { 8 };
    static constexpr int MinMidiNote   { 33 };
    static constexpr int MaxMidiNote   { 112 };

    PatternEditorModel();
    explicit PatternEditorModel(Pattern patternToEdit);

    const Pattern& getPattern() const noexcept { return pattern; }

    int getCursorRow() const noexcept { return cursorRow; }
    int getCursorTrack() const noexcept { return cursorTrack; }
    int getOctave() const noexcept { return octave; }
    int getEditStep() const noexcept { return editStep; }

    void setCursor(int row, int track) noexcept;
    void moveCursor(int rowDelta, int trackDelta) noexcept;

    void setOctave(int newOctave) noexcept;
    void adjustOctave(int delta) noexcept;

    void setEditStep(int newEditStep) noexcept;

    std::optional<int> insertNoteFromSemitone(int semitone);
    std::optional<int> insertNoteMidi(int midiNote);
    void insertNoteOff();
    void clearCurrentCell();

    static Pattern createDemoPattern();

private:
    Pattern pattern;
    int cursorRow   { 0 };
    int cursorTrack { 0 };
    int octave      { 4 };
    int editStep    { 1 };

    void advanceCursor() noexcept;
};
