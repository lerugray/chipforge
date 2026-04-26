#pragma once

#include "Data/Pattern.h"
#include "TransportState.h"

#include <algorithm>
#include <utility>

struct TrackerNoteEvent
{
    int sampleOffset { 0 };
    int row          { 0 };
    int track        { 0 };
    PatternCell cell;
};

//==============================================================================
// TrackerEngine
//
// Headless pattern sequencer. It advances through Pattern rows according to
// TransportState BPM and emits row events for notes/note-offs.
class TrackerEngine
{
public:
    static constexpr int DefaultTicksPerRow { 6 };
    static constexpr int MinTicksPerRow     { 1 };
    static constexpr int MaxTicksPerRow     { 32 };
    static constexpr int RowsPerBeat        { 4 };

    TrackerEngine() = default;
    explicit TrackerEngine(const Pattern& patternToUse);

    void prepare(double newSampleRate) noexcept;
    void reset() noexcept;

    void setPattern(const Pattern& patternToUse);
    const Pattern& getPattern() const noexcept { return pattern; }

    void setTicksPerRow(int newTicksPerRow) noexcept;
    int getTicksPerRow() const noexcept { return ticksPerRow; }

    int getCurrentRow() const noexcept { return currentRow; }
    int getSamplesPerTick() const noexcept;
    int getSamplesPerRow() const noexcept;

    template <typename EventCallback>
    void processBlock(int numSamples,
                      const TransportState& transport,
                      EventCallback&& onEvent)
    {
        if (numSamples <= 0)
            return;

        const bool isPlaying = transport.getIsPlaying();
        if (!isPlaying)
        {
            wasPlaying = false;
            return;
        }

        const int nextBpm = std::clamp(transport.getBpm(), 1, 300);
        if (nextBpm != bpm)
        {
            bpm = nextBpm;
            samplesUntilNextTick = std::min(samplesUntilNextTick, getSamplesPerTick());
        }

        if (!wasPlaying)
        {
            samplesUntilNextTick = getSamplesPerTick();
            emitCurrentRow(0, onEvent);
            wasPlaying = true;
        }

        int samplesProcessed = 0;
        int samplesRemaining = numSamples;
        while (samplesRemaining >= samplesUntilNextTick)
        {
            samplesProcessed += samplesUntilNextTick;
            samplesRemaining -= samplesUntilNextTick;
            advanceTick(samplesProcessed, onEvent);
            samplesUntilNextTick = getSamplesPerTick();
        }

        samplesUntilNextTick -= samplesRemaining;
    }

private:
    Pattern pattern;
    double sampleRate { 44100.0 };
    int bpm { 120 };
    int ticksPerRow { DefaultTicksPerRow };
    int currentRow { 0 };
    int currentTick { 0 };
    int samplesUntilNextTick { 0 };
    bool wasPlaying { false };

    template <typename EventCallback>
    void emitCurrentRow(int sampleOffset, EventCallback& onEvent) const
    {
        for (int track = 0; track < pattern.getNumTracks(); ++track)
        {
            const auto& cell = pattern.getCell(currentRow, track);
            if (cell.hasNote() || cell.isNoteOff())
                onEvent(TrackerNoteEvent { sampleOffset, currentRow, track, cell });
        }
    }

    template <typename EventCallback>
    void advanceTick(int sampleOffset, EventCallback& onEvent)
    {
        ++currentTick;

        if (currentTick < ticksPerRow)
            return;

        currentTick = 0;
        currentRow = (currentRow + 1) % pattern.getNumRows();
        emitCurrentRow(sampleOffset, onEvent);
    }
};
