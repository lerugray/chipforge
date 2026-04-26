#pragma once

#include "PatternCell.h"

#include <vector>

//==============================================================================
// Pattern
//
// Fixed-size tracker pattern data: rows x tracks, stored independently from UI
// and audio so editing/playback can be tested without device dependencies.
class Pattern
{
public:
    static constexpr int MinRows       { 1 };
    static constexpr int MaxRows       { 256 };
    static constexpr int DefaultRows   { 64 };
    static constexpr int MinTracks     { 1 };
    static constexpr int MaxTracks     { 8 };
    static constexpr int DefaultTracks { 8 };

    explicit Pattern(int rows = DefaultRows, int tracks = DefaultTracks);

    int getNumRows() const noexcept { return numRows; }
    int getNumTracks() const noexcept { return numTracks; }

    bool isValidPosition(int row, int track) const noexcept;

    const PatternCell& getCell(int row, int track) const noexcept;
    bool setCell(int row, int track, PatternCell cell);

    void clear();

private:
    int numRows   { DefaultRows };
    int numTracks { DefaultTracks };
    std::vector<PatternCell> cells;

    std::size_t indexFor(int row, int track) const noexcept;
};
