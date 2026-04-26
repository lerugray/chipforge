#include <juce_core/juce_core.h>

#include "Engine/TrackerEngine.h"

class TrackerEngineTests final : public juce::UnitTest
{
public:
    TrackerEngineTests()
        : juce::UnitTest("TrackerEngine", "Engine")
    {
    }

    void runTest() override
    {
        auto collectEvents = [](TrackerEngine& engine,
                                int numSamples,
                                const TransportState& transport,
                                std::vector<TrackerNoteEvent>& events)
        {
            events.clear();
            engine.processBlock(numSamples,
                                transport,
                                [&events](const TrackerNoteEvent& event)
                                {
                                    events.push_back(event);
                                });
        };

        auto hasEventCount = [this](const std::vector<TrackerNoteEvent>& events, int expected)
        {
            expectEquals(static_cast<int>(events.size()), expected);
            return static_cast<int>(events.size()) == expected;
        };

        beginTest("stopped transport emits no events");
        Pattern pattern(4, 2);
        pattern.setCell(0, 0, PatternCell::makeNote(60));

        TransportState transport;
        TrackerEngine engine(pattern);
        engine.prepare(48000.0);

        std::vector<TrackerNoteEvent> events;
        collectEvents(engine, 128, transport, events);
        expect(events.empty());
        expectEquals(engine.getCurrentRow(), 0);

        beginTest("playback emits the first row immediately");
        transport.play();
        collectEvents(engine, 1, transport, events);
        if (!hasEventCount(events, 1))
            return;
        expectEquals(events.front().sampleOffset, 0);
        expectEquals(events.front().row, 0);
        expectEquals(events.front().track, 0);
        expectEquals(events.front().cell.note, static_cast<uint8_t>(60));

        beginTest("row timing follows BPM");
        pattern.setCell(1, 1, PatternCell::makeNote(62));
        engine.setPattern(pattern);
        engine.prepare(48000.0);
        engine.reset();
        transport.play();

        collectEvents(engine, 1, transport, events);
        if (!hasEventCount(events, 1))
            return;
        expectEquals(events.front().sampleOffset, 0);
        expectEquals(events.front().row, 0);

        collectEvents(engine, engine.getSamplesPerRow() - 1, transport, events);
        if (!hasEventCount(events, 1))
            return;
        expectEquals(events.front().sampleOffset, engine.getSamplesPerRow() - 1);
        expectEquals(events.front().row, 1);
        expectEquals(events.front().track, 1);
        expectEquals(events.front().cell.note, static_cast<uint8_t>(62));
        expectEquals(engine.getCurrentRow(), 1);

        beginTest("ticks per row divide row advancement without changing row duration");
        engine.setTicksPerRow(3);
        engine.reset();
        transport.play();

        const int samplesPerTick = engine.getSamplesPerTick();
        collectEvents(engine, 1, transport, events);
        if (!hasEventCount(events, 1))
            return;
        expectEquals(events.front().sampleOffset, 0);
        expectEquals(events.front().row, 0);

        collectEvents(engine, samplesPerTick - 1, transport, events);
        expect(events.empty());

        collectEvents(engine, samplesPerTick, transport, events);
        expect(events.empty());

        collectEvents(engine, samplesPerTick, transport, events);
        if (!hasEventCount(events, 1))
            return;
        expectEquals(events.front().sampleOffset, samplesPerTick);
        expectEquals(events.front().row, 1);

        beginTest("pattern playback wraps at the end");
        Pattern twoRows(2, 1);
        twoRows.setCell(0, 0, PatternCell::makeNote(60));
        twoRows.setCell(1, 0, PatternCell::makeNote(64));
        engine.setPattern(twoRows);
        engine.setTicksPerRow(TrackerEngine::DefaultTicksPerRow);
        engine.reset();
        transport.play();

        collectEvents(engine, 1, transport, events);
        if (!hasEventCount(events, 1))
            return;
        expectEquals(events.front().sampleOffset, 0);
        expectEquals(events.front().row, 0);

        collectEvents(engine, engine.getSamplesPerRow() - 1, transport, events);
        if (!hasEventCount(events, 1))
            return;
        expectEquals(events.front().sampleOffset, engine.getSamplesPerRow() - 1);
        expectEquals(events.front().row, 1);

        collectEvents(engine, engine.getSamplesPerRow(), transport, events);
        if (!hasEventCount(events, 1))
            return;
        expectEquals(events.front().sampleOffset, engine.getSamplesPerRow());
        expectEquals(events.front().row, 0);
    }
};

static TrackerEngineTests trackerEngineTests;
