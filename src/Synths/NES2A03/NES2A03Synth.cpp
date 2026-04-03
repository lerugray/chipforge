#include "NES2A03Synth.h"
#include <algorithm>

//==============================================================================
// NES APU period lookup table
// Maps note index to 11-bit timer period: period = CPU_CLOCK / (16 * freq) - 1
// CPU_CLOCK = 1789773 Hz (NTSC). Index 0 = MIDI 33 (A1, ~55Hz).
// Period is split into lo (8 bits) and hi (3 bits).
// Derived from NES hardware documentation / FamiTracker reference tables.
//==============================================================================
const uint8_t NES2A03Synth::kPeriodLo[80] = {
    0xf1, 0x7f, 0x13, 0xad, 0x4d, 0xf3, 0x9d, 0x4c, 0x00, 0xb8, 0x74, 0x34,
    0xf8, 0xbf, 0x89, 0x56, 0x26, 0xf9, 0xce, 0xa6, 0x80, 0x5c, 0x3a, 0x1a,
    0xfb, 0xdf, 0xc4, 0xab, 0x93, 0x7c, 0x67, 0x52, 0x3f, 0x2d, 0x1c, 0x0c,
    0xfd, 0xef, 0xe1, 0xd5, 0xc9, 0xbd, 0xb3, 0xa9, 0x9f, 0x96, 0x8e, 0x86,
    0x7e, 0x77, 0x70, 0x6a, 0x64, 0x5e, 0x59, 0x54, 0x4f, 0x4b, 0x46, 0x42,
    0x3f, 0x3b, 0x38, 0x34, 0x31, 0x2f, 0x2c, 0x29, 0x27, 0x25, 0x23, 0x21,
    0x1f, 0x1d, 0x1b, 0x1a, 0x18, 0x17, 0x15, 0x14
};

const uint8_t NES2A03Synth::kPeriodHi[80] = {
    0x07, 0x07, 0x07, 0x06, 0x06, 0x05, 0x05, 0x05, 0x05, 0x04, 0x04, 0x04,
    0x03, 0x03, 0x03, 0x03, 0x03, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02,
    0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

//==============================================================================

NES2A03Synth::NES2A03Synth()
{
    // Silence DMC (we don't use it) with a flat sample reader
    apu.dmc_reader = [](int) { return 0x55; };
}

void NES2A03Synth::prepare(double sampleRate, int maxBlockSize)
{
    blipBuf.clock_rate(1789773);
    blipBuf.set_sample_rate((long)sampleRate);
    apu.set_output(&blipBuf);
    apu.reset();

    // Pre-allocate temp buffer: one full NES frame (735 samples @44.1kHz) plus
    // enough for the largest JUCE block we'll ever be asked for.
    tempBuffer.resize((size_t)(maxBlockSize + 1024));

    // Enable all standard channels, then silence them
    writeReg(0x4015, 0x0F);  // enable pulse1, pulse2, triangle, noise
    writeReg(0x4000, 0x30);  // pulse1 silent (constant vol=0)
    writeReg(0x4004, 0x30);  // pulse2 silent
    writeReg(0x4008, 0x00);  // triangle silent
    writeReg(0x400C, 0x30);  // noise silent

    // Flush initial state through a first frame
    apu.end_frame(frameLength);
    blipBuf.end_frame(frameLength);
    blipBuf.remove_silence(blipBuf.samples_avail());
    apuTime = 0;

    prepared = true;
}

void NES2A03Synth::renderBlock(const juce::AudioSourceChannelInfo& bufferToFill)
{
    if (!prepared)
    {
        bufferToFill.clearActiveBufferRegion();
        return;
    }

    // Apply any pending note command from the UI thread (atomic exchange)
    int note = pendingNote.exchange(-2, std::memory_order_acq_rel);
    if (note == -1)
        applyNoteOff();
    else if (note >= 0)
        applyNoteOn(note);

    const int samplesNeeded = bufferToFill.numSamples;

    // Generate NES frames until we have enough samples in the Blip_Buffer
    while (blipBuf.samples_avail() < samplesNeeded)
    {
        apu.end_frame(frameLength);
        blipBuf.end_frame(frameLength);
        frameLength ^= 1;   // alternate 29780/29781 for NTSC timing accuracy
        apuTime = 0;        // reset register write clock for next frame
    }

    // Read exactly samplesNeeded int16 samples, convert to float, write stereo
    blipBuf.read_samples(tempBuffer.data(), samplesNeeded);

    const int startSample = bufferToFill.startSample;
    auto* buf = bufferToFill.buffer;

    for (int i = 0; i < samplesNeeded; ++i)
    {
        const float s = (float)tempBuffer[i] * (1.0f / 32768.0f);
        buf->setSample(0, startSample + i, s);
        if (buf->getNumChannels() > 1)
            buf->setSample(1, startSample + i, s);
    }
}

void NES2A03Synth::noteOn(int midiNote)
{
    pendingNote.store(midiNote, std::memory_order_release);
}

void NES2A03Synth::noteOff()
{
    pendingNote.store(-1, std::memory_order_release);
}

//==============================================================================
// Private — called from audio thread only

void NES2A03Synth::applyNoteOn(int midiNote)
{
    const int tableIndex = std::clamp(midiNote - kNoteTableOffset, 0, 79);
    const uint8_t lo = kPeriodLo[tableIndex];
    const uint8_t hi = kPeriodHi[tableIndex];

    // Enable pulse 1 channel only
    writeReg(0x4015, 0x01);

    // 0x4000: duty=50% (10), length halt (1), constant vol (1), vol=12 (1100)
    //   = 1011 1100 = 0xBC
    writeReg(0x4000, 0xBC);
    writeReg(0x4001, 0x00);           // sweep off
    writeReg(0x4002, lo);             // period low
    writeReg(0x4003, 0xF8 | hi);      // length counter max + period high

    activeNote.store(midiNote, std::memory_order_relaxed);
}

void NES2A03Synth::applyNoteOff()
{
    // Silence by setting constant volume to 0
    writeReg(0x4000, 0x30);           // duty=0, halt, const vol, vol=0
    activeNote.store(-1, std::memory_order_relaxed);
}

void NES2A03Synth::writeReg(int addr, int data)
{
    apu.write_register(apuTime, (uint16_t)addr, (uint8_t)data);
    apuTime += 4;
}
