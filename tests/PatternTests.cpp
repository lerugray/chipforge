#include <juce_core/juce_core.h>

#include "Data/Pattern.h"

class PatternTests final : public juce::UnitTest
{
public:
    PatternTests()
        : juce::UnitTest("Pattern", "Data")
    {
    }

    void runTest() override
    {
        beginTest("PatternCell defaults to an empty tracker cell");
        const PatternCell empty;
        expect(empty.isEmpty());
        expect(!empty.hasNote());
        expect(!empty.isNoteOff());
        expectEquals(empty.note, PatternCell::EmptyNote);
        expectEquals(empty.instrument, PatternCell::EmptyInstrument);
        expectEquals(empty.volume, PatternCell::EmptyVolume);
        expectEquals(empty.effectCommand, PatternCell::NoEffect);

        beginTest("PatternCell can represent notes and note-off events");
        const auto note = PatternCell::makeNote(60, 2, 48, 0x0C30);
        expect(note.hasNote());
        expect(!note.isEmpty());
        expect(!note.isNoteOff());
        expectEquals(note.note, static_cast<uint8_t>(60));
        expectEquals(note.instrument, static_cast<uint8_t>(2));
        expectEquals(note.volume, static_cast<uint8_t>(48));
        expectEquals(note.effectCommand, static_cast<uint16_t>(0x0C30));

        const auto noteOff = PatternCell::makeNoteOff();
        expect(noteOff.isNoteOff());
        expect(!noteOff.hasNote());

        beginTest("Pattern defaults to 64 rows and 8 tracks");
        const Pattern defaultPattern;
        expectEquals(defaultPattern.getNumRows(), Pattern::DefaultRows);
        expectEquals(defaultPattern.getNumTracks(), Pattern::DefaultTracks);
        expect(defaultPattern.isValidPosition(0, 0));
        expect(defaultPattern.isValidPosition(63, 7));
        expect(!defaultPattern.isValidPosition(64, 0));
        expect(!defaultPattern.isValidPosition(0, 8));
        expect(defaultPattern.getCell(0, 0).isEmpty());

        beginTest("Pattern supports configurable row counts");
        const Pattern shortPattern(16);
        expectEquals(shortPattern.getNumRows(), 16);
        expectEquals(shortPattern.getNumTracks(), Pattern::DefaultTracks);
        expect(shortPattern.isValidPosition(15, 7));
        expect(!shortPattern.isValidPosition(16, 0));

        beginTest("Pattern stores cells and rejects out-of-bounds writes");
        Pattern pattern(4, 2);
        const auto c4 = PatternCell::makeNote(60);
        expect(pattern.setCell(2, 1, c4));
        expect(pattern.getCell(2, 1).hasNote());
        expectEquals(pattern.getCell(2, 1).note, static_cast<uint8_t>(60));

        const auto invalidWrite = pattern.setCell(4, 0, PatternCell::makeNote(62));
        expect(!invalidWrite);
        expect(pattern.getCell(2, 1).hasNote());

        beginTest("Pattern constructor clamps unsupported dimensions");
        const Pattern tooSmall(0, 0);
        expectEquals(tooSmall.getNumRows(), Pattern::MinRows);
        expectEquals(tooSmall.getNumTracks(), Pattern::MinTracks);

        const Pattern tooLarge(999, 99);
        expectEquals(tooLarge.getNumRows(), Pattern::MaxRows);
        expectEquals(tooLarge.getNumTracks(), Pattern::MaxTracks);
    }
};

static PatternTests patternTests;
