#pragma once

#include <atomic>

//==============================================================================
// TransportState
//
// Holds the global playback state. Shared between the UI thread (reads/writes
// via play/stop) and the audio thread (reads isPlaying, bpm). All shared fields
// are atomic to avoid data races.
//==============================================================================
class TransportState
{
public:
    TransportState() = default;

    void play()  { isPlaying.store(true,  std::memory_order_release); }
    void stop()  { isPlaying.store(false, std::memory_order_release); }
    void toggle() { isPlaying ? stop() : play(); }

    bool getIsPlaying() const { return isPlaying.load(std::memory_order_acquire); }
    int  getBpm()       const { return bpm.load(std::memory_order_relaxed); }
    void setBpm(int newBpm)   { bpm.store(newBpm, std::memory_order_relaxed); }

private:
    std::atomic<bool> isPlaying { false };
    std::atomic<int>  bpm       { 120 };

    TransportState(const TransportState&) = delete;
    TransportState& operator=(const TransportState&) = delete;
};
