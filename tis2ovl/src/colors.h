#ifndef COLORS_H_INCLUDED
#define COLORS_H_INCLUDED

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

/**
 * Return either unused color entry (if color1 == color2).
 * or two similar color entries that can be merged (if color1 != color2).
 * Assumes a color map with 256 entries in BGRA format and bitmap of 64x64 pixels.
 * \param tile      Buffer containing palette and pixel data.
 * \param color1    Storage for color entry 1.
 * \param color2    Storage for color entry 2.
 * \return whether operation was successful.
 */
bool getMergeableColors(const uint8_t *tile, uint8_t *color1, uint8_t *color2);

/**
 * Remove color entry "search" and add freed entry as transparent color at entry 0.
 * Use "replace" as color replacement for "search".
 * \param data      Buffer containing palette and pixel data.
 * \param search    Color index to be replaced.
 * \param replace   Color index to replace "search".
 */
void adjustTileColors(uint8_t *data, uint8_t search, uint8_t replace);

/// Return palette index of matching color. -1 if not found.
int colorIndex(const void *data, size_t palSize, uint32_t color);

/**
 * Create a new paletted tile from the specified source tile.
 * \param srcTile   Source tile converted to RGBA truecolor format (0xaabbggrr).
 * \param dstTile   Storage for resulting tile with new palette.
 * \param useTransparent    Whether tile contains transparent pixel regions.
 * \return whether remapping operation was successful.
 */
bool createRemappedTile(const uint32_t *srcTile, uint8_t *dstTile, bool useTransparent);

#endif // COLORS_H_INCLUDED
