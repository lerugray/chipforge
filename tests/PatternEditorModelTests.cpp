#include <juce_core/juce_core.h>

#include "Data/PatternEditorModel.h"
#include "Engine/TrackerEngine.h"

class PatternEditorModelTests final : public juce::UnitTest
{
public:
    PatternEditorModelTests()
        : juce::UnitTest("PatternEditorModel", "Data")
    {
    }

    void runTest() override
    {
        beginTest("default editor has a playable demo pattern and cursor defaults");
        PatternEditorModel editor;
        expectEquals(editor.getPattern().getNumRows(), PatternEditorModel::DefaultRows);
        expectEquals(editor.getPattern().getNumTracks(), PatternEditorModel::DefaultTracks);
        expectEquals(editor.getCursorRow(), 0);
        expectEquals(editor.getCursorTrack(), 0);
        expectEquals(editor.getOctave(), 4);
        expect(editor.getPattern().getCell(0, 0).hasNote());

        beginTest("cursor movement clamps to pattern bounds");
        editor.setCursor(999, 999);
        expectEquals(editor.getCursorRow(), editor.getPattern().getNumRows() - 1);
        expectEquals(editor.getCursorTrack(), editor.getPattern().getNumTracks() - 1);

        editor.moveCursor(-999, -999);
        expectEquals(editor.getCursorRow(), 0);
        expectEquals(editor.getCursorTrack(), 0);

        beginTest("note entry writes the selected cell and advances by edit step");
        editor.setCursor(2, 3);
        editor.setEditStep(2);
        const auto midiNote = editor.insertNoteFromSemitone(0);
        expect(midiNote.has_value());
        expectEquals(*midiNote, 60);
        expectEquals(editor.getPattern().getCell(2, 3).note, static_cast<uint8_t>(60));
        expectEquals(editor.getCursorRow(), 4);
        expectEquals(editor.getCursorTrack(), 3);

        beginTest("upper keyboard row maps to the next octave");
        editor.setCursor(5, 0);
        const auto upperNote = editor.insertNoteFromSemitone(12);
        expect(upperNote.has_value());
        expectEquals(*upperNote, 72);
        expectEquals(editor.getPattern().getCell(5, 0).note, static_cast<uint8_t>(72));

        beginTest("note-off entry advances and clear does not advance");
        editor.setCursor(6, 1);
        editor.insertNoteOff();
        expect(editor.getPattern().getCell(6, 1).isNoteOff());
        expectEquals(editor.getCursorRow(), 8);

        editor.clearCurrentCell();
        expect(editor.getPattern().getCell(8, 1).isEmpty());
        expectEquals(editor.getCursorRow(), 8);

        beginTest("note entry wraps at the pattern bottom");
        PatternEditorModel wrappingEditor(Pattern(4, 1));
        wrappingEditor.setCursor(3, 0);
        wrappingEditor.setEditStep(1);
        wrappingEditor.insertNoteMidi(67);
        expectEquals(wrappingEditor.getCursorRow(), 0);

        beginTest("edited pattern snapshot drives tracker playback");
        PatternEditorModel playbackEditor(Pattern(4, 1));
        playbackEditor.setCursor(1, 0);
        playbackEditor.insertNoteMidi(65);

        TrackerEngine engine(playbackEditor.getPattern());
        TransportState transport;
        std::vector<TrackerNoteEvent> events;
        engine.prepare(48000.0);
        transport.play();

        engine.processBlock(1,
                            transport,
                            [&events](const TrackerNoteEvent& event)
                            {
                                events.push_back(event);
                            });
        expect(events.empty());

        engine.processBlock(engine.getSamplesPerRow() - 1,
                            transport,
                            [&events](const TrackerNoteEvent& event)
                            {
                                events.push_back(event);
                            });

        expectEquals(static_cast<int>(events.size()), 1);
        if (!events.empty())
        {
            expectEquals(events.front().row, 1);
            expectEquals(events.front().cell.note, static_cast<uint8_t>(65));
        }
    }
};

static PatternEditorModelTests patternEditorModelTests;
