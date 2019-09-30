#include <limits.h>
#include <string.h>
#include "compat.h"
#include "colors.h"
#include "functions.h"
#include "libimagequant.h"

// Definition of a colormap entry
typedef struct {
    uint32_t color; // the BGRA value
    int weight;     // how often used by pixels
} color_t;
def_cleanFunc(cleanColor, color_t*)

// Stores two similar color entries.
typedef struct {
    color_t color1, color2; // weighted colors
    int dist;               // weighted distance between colors
} colordiff_t;
def_cleanFunc(cleanColorDiff, colordiff_t*)

// Convenience structure: Simplify cleanup of pngquant memory.
typedef struct {
    liq_attr *attr;
    liq_result *result;
} quantize_t;

// Clean up pngquant memory
void cleanQuantizer(quantize_t **pquant);

// Returns weighted color distance
int colorDistance(uint32_t color1, uint32_t color2);


bool getMergeableColors(const uint8_t *data, uint8_t *color1, uint8_t *color2) {
    if (data && color1 && color2) {
#define PAL_SIZE 256
        // 1. try getting unused color index first
        bool *used finally(cleanBool) = malloc(sizeof(bool) * PAL_SIZE);
        memset(used, (int)false, sizeof(bool) * PAL_SIZE);
        size_t pixelOfs = PAL_SIZE * 4;
        for (size_t i = 0; i < 4096; ++i)
            used[data[pixelOfs + i]] |= true;
        for (size_t i = 0; i < PAL_SIZE; ++i) {
            if (!used[i]) {
                *color1 = *color2 = (uint8_t)i;
                return true;
            }
        }

        // 2. merge two colors to free a color slot
        int i1 = -1, i2 = -1, dist = INT_MAX;
        for (size_t i = 0; i < PAL_SIZE - 1; ++i) {
            uint32_t col1 = *(uint32_t*)(data + i*4);
            for (size_t j = i + 1; j < PAL_SIZE; ++j) {
                int d = colorDistance(col1, *(uint32_t*)(data + j*4));
                if (d < dist) { dist = d; i1 = i; i2 = j; }
                if (dist == 0) break;
            }
            if (dist == 0) break;
        }
        if (dist != INT_MAX) {
            *color1 = (uint8_t)i1;
            *color2 = (uint8_t)i2;
            return true;
        }
#undef PAL_SIZE
    }
    return false;
}


void adjustTileColors(uint8_t *data, uint8_t search, uint8_t replace) {
    if (data) {
        // "search" should be less or equal "replace"
        if (search > replace) {
            search ^= replace;
            replace ^= search;
            search ^= replace;
        }

        // adjusting pixels
        for (size_t p = 1024; p < 5120; ++p) {
            if (data[p] == search && search != replace)
                data[p] = replace;
            else if (data[p] < search)
                data[p]++;
        }

        // adjusting palette
        uint32_t *pal = (uint32_t*)data;
        for (int i = search; i > 0; --i) {
            pal[i] = pal[i - 1];
        }
        pal[0] = 0x0000ff00;
    }
}


int colorIndex(const void *data, size_t palSize, uint32_t color) {
    const uint32_t *pal = data;
    for (size_t i = 0; i < palSize; ++i)
        if (pal[i] == color)
            return i;
    return -1;
}


bool createRemappedTile(const uint32_t *srcTile, uint8_t *dstTile, bool useTransparent) {
    if (!srcTile || !dstTile) return false;
    quantize_t *quant finally(cleanQuantizer) = malloc(sizeof(quantize_t));
    if ((quant->attr = liq_attr_create()) == NULL) return false;
    liq_image *img = liq_image_create_rgba(quant->attr, srcTile, 64, 64, 0.0);
    if (!img) return false;
    if (useTransparent) {
        liq_color c; c.b = c.r = 0; c.a = c.g = 255;
        if (liq_image_add_fixed_color(img, c) != LIQ_OK) return false;
    }
    if (liq_image_quantize(img, quant->attr, &quant->result) != LIQ_OK) return false;
    if (liq_write_remapped_image(quant->result, img, dstTile + 1024, 4096) != LIQ_OK) return false;
    const liq_palette *pal = liq_get_palette(quant->result);
    for (unsigned i = 0; i < pal->count; ++i) {
        dstTile[i*4] = pal->entries[i].r;
        dstTile[i*4+1] = pal->entries[i].g;
        dstTile[i*4+2] = pal->entries[i].b;
        dstTile[i*4+3] = 0;
    }
    if (pal->count < 256)
        memset(dstTile + pal->count*4, 0, (256-pal->count)*4);
    return true;
}


int colorDistance(uint32_t color1, uint32_t color2) {
    int dr = (color1 >> 16) & 0xff;
    int dg = (color1 >> 8) & 0xff;
    int db = color1 & 0xff;
    dr -= (color2 >> 16) & 0xff;
    dg -= (color2 >> 8) & 0xff;
    db -= color2 & 0xff;
    dr *= 30;
    dg *= 59;
    db *= 11;
    return dr*dr + dg*dg + db*db;
}

void cleanQuantizer(quantize_t **pquant) {
    if (pquant && *pquant) {
        if ((*pquant)->result) liq_result_destroy((*pquant)->result);
        if ((*pquant)->attr) liq_attr_destroy((*pquant)->attr);
        free(*pquant);
        *pquant = NULL;
    }
}
