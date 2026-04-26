#pragma once

#include <algorithm>
#include <cstdint>

//==============================================================================
// One tracker grid cell. A cell can hold a note, a note-off marker, instrument
// and volume changes, plus one packed tracker effect command.
struct PatternCell
{
    static constexpr uint8_t EmptyNote       { 0xFF };
    static constexpr uint8_t NoteOff         { 0xFE };
    static constexpr uint8_t MaxMidiNote     { 127 };
    static constexpr uint8_t EmptyInstrument { 0xFF };
    static constexpr uint8_t EmptyVolume     { 0xFF };
    static constexpr uint16_t NoEffect       { 0x0000 };

    uint8_t note          { EmptyNote };
    uint8_t instrument    { EmptyInstrument };
    uint8_t volume        { EmptyVolume };
    uint16_t effectCommand { NoEffect };

    constexpr bool isEmpty() const noexcept
    {
        return note == EmptyNote
            && instrument == EmptyInstrument
            && volume == EmptyVolume
            && effectCommand == NoEffect;
    }

    constexpr bool hasNote() const noexcept
    {
        return note <= MaxMidiNote;
    }

    constexpr bool isNoteOff() const noexcept
    {
        return note == NoteOff;
    }

    static constexpr PatternCell makeEmpty() noexcept
    {
        return {};
    }

    static constexpr PatternCell makeNoteOff(uint8_t instrumentValue = EmptyInstrument,
                                             uint8_t volumeValue = EmptyVolume,
                                             uint16_t effectValue = NoEffect) noexcept
    {
        return { NoteOff, instrumentValue, volumeValue, effectValue };
    }

    static constexpr PatternCell makeNote(int midiNote,
                                          uint8_t instrumentValue = EmptyInstrument,
                                          uint8_t volumeValue = EmptyVolume,
                                          uint16_t effectValue = NoEffect) noexcept
    {
        const auto clampedNote = std::clamp(midiNote, 0, static_cast<int>(MaxMidiNote));
        return { static_cast<uint8_t>(clampedNote), instrumentValue, volumeValue, effectValue };
    }
};
