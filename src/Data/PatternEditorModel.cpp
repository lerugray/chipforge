#include "PatternEditorModel.h"

#include <algorithm>
#include <utility>

PatternEditorModel::PatternEditorModel()
    : pattern(createDemoPattern())
{
}

PatternEditorModel::PatternEditorModel(Pattern patternToEdit)
    : pattern(std::move(patternToEdit))
{
    setCursor(0, 0);
}

void PatternEditorModel::setCursor(int row, int track) noexcept
{
    cursorRow = std::clamp(row, 0, pattern.getNumRows() - 1);
    cursorTrack = std::clamp(track, 0, pattern.getNumTracks() - 1);
}

void PatternEditorModel::moveCursor(int rowDelta, int trackDelta) noexcept
{
    setCursor(cursorRow + rowDelta, cursorTrack + trackDelta);
}

void PatternEditorModel::setOctave(int newOctave) noexcept
{
    octave = std::clamp(newOctave, MinOctave, MaxOctave);
}

void PatternEditorModel::adjustOctave(int delta) noexcept
{
    setOctave(octave + delta);
}

void PatternEditorModel::setEditStep(int newEditStep) noexcept
{
    editStep = std::clamp(newEditStep, 0, pattern.getNumRows() - 1);
}

std::optional<int> PatternEditorModel::insertNoteFromSemitone(int semitone)
{
    if (semitone < 0 || semitone > 23)
        return std::nullopt;

    const int noteOctave = octave + (semitone >= 12 ? 1 : 0);
    const int midiNote = (noteOctave + 1) * 12 + (semitone % 12);
    return insertNoteMidi(midiNote);
}

std::optional<int> PatternEditorModel::insertNoteMidi(int midiNote)
{
    if (midiNote < MinMidiNote || midiNote > MaxMidiNote)
        return std::nullopt;

    pattern.setCell(cursorRow, cursorTrack, PatternCell::makeNote(midiNote));
    advanceCursor();
    return midiNote;
}

void PatternEditorModel::insertNoteOff()
{
    pattern.setCell(cursorRow, cursorTrack, PatternCell::makeNoteOff());
    advanceCursor();
}

void PatternEditorModel::clearCurrentCell()
{
    pattern.setCell(cursorRow, cursorTrack, PatternCell::makeEmpty());
}

Pattern PatternEditorModel::createDemoPattern()
{
    Pattern demo(DefaultRows, DefaultTracks);
    demo.setCell(0, 0, PatternCell::makeNote(60));
    demo.setCell(4, 0, PatternCell::makeNote(64));
    demo.setCell(8, 0, PatternCell::makeNote(67));
    demo.setCell(12, 0, PatternCell::makeNoteOff());
    return demo;
}

void PatternEditorModel::advanceCursor() noexcept
{
    if (editStep == 0)
        return;

    const int nextRow = (cursorRow + editStep) % pattern.getNumRows();
    setCursor(nextRow, cursorTrack);
}
