#include "TrackerEngine.h"

#include <algorithm>
#include <cmath>

TrackerEngine::TrackerEngine(const Pattern& patternToUse)
    : pattern(patternToUse)
{
}

void TrackerEngine::prepare(double newSampleRate) noexcept
{
    sampleRate = std::max(newSampleRate, 1.0);
    samplesUntilNextTick = getSamplesPerTick();
}

void TrackerEngine::reset() noexcept
{
    currentRow = 0;
    currentTick = 0;
    samplesUntilNextTick = getSamplesPerTick();
    wasPlaying = false;
}

void TrackerEngine::setPattern(const Pattern& patternToUse)
{
    pattern = patternToUse;
    if (currentRow >= pattern.getNumRows())
        currentRow = 0;
}

void TrackerEngine::setTicksPerRow(int newTicksPerRow) noexcept
{
    ticksPerRow = std::clamp(newTicksPerRow, MinTicksPerRow, MaxTicksPerRow);
    samplesUntilNextTick = std::min(samplesUntilNextTick, getSamplesPerTick());
}

int TrackerEngine::getSamplesPerTick() const noexcept
{
    const auto samplesPerRow = sampleRate * 60.0 / (static_cast<double>(bpm) * RowsPerBeat);
    const auto samplesPerTick = samplesPerRow / static_cast<double>(ticksPerRow);
    return std::max(1, static_cast<int>(std::lround(samplesPerTick)));
}

int TrackerEngine::getSamplesPerRow() const noexcept
{
    return getSamplesPerTick() * ticksPerRow;
}
