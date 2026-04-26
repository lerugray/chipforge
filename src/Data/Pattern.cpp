#include "Pattern.h"

#include <algorithm>
#include <cstddef>

Pattern::Pattern(int rows, int tracks)
    : numRows(std::clamp(rows, MinRows, MaxRows)),
      numTracks(std::clamp(tracks, MinTracks, MaxTracks)),
      cells(static_cast<std::size_t>(numRows * numTracks))
{
}

bool Pattern::isValidPosition(int row, int track) const noexcept
{
    return row >= 0 && row < numRows
        && track >= 0 && track < numTracks;
}

const PatternCell& Pattern::getCell(int row, int track) const noexcept
{
    static const PatternCell emptyCell;

    if (!isValidPosition(row, track))
        return emptyCell;

    return cells[indexFor(row, track)];
}

bool Pattern::setCell(int row, int track, PatternCell cell)
{
    if (!isValidPosition(row, track))
        return false;

    cells[indexFor(row, track)] = cell;
    return true;
}

void Pattern::clear()
{
    std::fill(cells.begin(), cells.end(), PatternCell::makeEmpty());
}

std::size_t Pattern::indexFor(int row, int track) const noexcept
{
    const auto linearIndex = row * numTracks + track;
    return static_cast<std::size_t>(linearIndex);
}
