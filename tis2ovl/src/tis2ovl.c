#include <stdio.h>
#include <string.h>
#include "tis2ovl.h"
#include "version.h"
#include "compat.h"
#include "arrays.h"
#include "functions.h"
#include "colors.h"
//#include "quant.h"

#define TILE_SIZE 5120
#define TILE_DIM 64

#define TRANSPARENT 0x0000ff00

// Used by the convertXX functions
typedef struct {
    int pri, sec;
} tile_t;

// Cleanup function definitions
void cleanArrayRelease(array_t **pvar) {
    if (pvar) {
        arrayClear(*pvar, true);
        free(*pvar);
    }
}

// Detect conversion mode from pixel data.
int getMode(int, const uint8_t *);
// Convert a single tile from classic to EE mode.
bool tileToEE(const tile_t *, const uint8_t *, const uint8_t *, uint8_t *, uint8_t *);
// Convert a single tile from EE to classic mode.
bool tileFromEE(const tile_t *, const uint8_t *, const uint8_t *, uint8_t *, uint8_t *, const char *);
// Retrieve relevant information from WED file.
bool parseWED(const char *, char *, array_t *);
// Retrieve header information from TIS file.
FILE* parseTISFile(const char *, int *, int *);
// Store full path of TIS file based on given search path list and TIS filename.
bool findTISFile(array_t *, const char *, char *);

void printHelp() {
    printf("Usage: %s [OPTIONS]... WEDFILE...\n", TIS2OVL_NAME);
    printf("Retrieve information from WEDFILE(s) to convert tileset (TIS) overlays between classic BG2 and Enhanced Edition games.\n\n");
    printf("Options:\n");
    printf("  -c            Convert TIS overlays from classic to Enhanced Edition mode.\n");
    printf("  -e            Convert TIS overlays from Enhanced Edition to classic mode.\n");
    printf("                (Note: Omit -c and -e to autodetect TIS overlay conversion mode.)\n");
    printf("  -s path       Search path for TIS files. This option can be specified\n");
    printf("                multiple times. Default: current directory\n");
    printf("  -o out_path   Output directory for TIS files. Omit to update original files instead.\n");
    printf("  -q            Enable quiet mode. Do not print any log messages to standard output.\n");
    printf("  -h            Print this help and exit.\n");
    printf("  -v            Print version information and exit.\n");
}

void printVersion() {
    printf("%s v%s - Convert tileset overlays between classic and Enhanced Edition games.\n", TIS2OVL_NAME, TIS2OVL_VERSION);
    printf("Written by Argent77.\n\n");
}


int convert(const char *wedFile, array_t *searchPath, const char *outputDir) {
    if (!wedFile || !searchPath) {
        printMsg(OUTPUT_ERR, "Error: Internal error.\n");
        return -1;
    }
    switch (param_mode) {
    case MODE_AUTO:
    case MODE_TO_EE:
    case MODE_FROM_EE:
      break;
    default:
      printMsg(OUTPUT_ERR, "Error: Invalid conversion mode: %d.\n", param_mode);
      return -1;
    }

    array_t *tileList finally(cleanArrayRelease) = malloc(sizeof(array_t));
    arrayInit(tileList, 1);
    char tisName[15] = {0}, tisFile[FILENAME_MAX] = {0};

    // Parsing WED
    printMsg(OUTPUT_MSG, "Parsing WED file \"%s\"...\n", wedFile);
    if (!parseWED(wedFile, tisName, tileList)) return -1;

    // preparing TIS file
    if (!evalOp(findTISFile(searchPath, tisName, tisFile), "Error: Could not find TIS file: %s\n", tisName)) return false;
    if (outputDir) {
        char tisFileOut[FILENAME_MAX] = {0};
        sprintf(tisFileOut, "%s/%s", outputDir, tisName);
        if (!isFileIdentical(tisFile, tisFileOut)) {
            if (!evalOp(copyFile(tisFile, tisFileOut, true), "Error: Could not create output TIS file: %s\n", tisFileOut)) return false;
        }
        strcpy(tisFile, tisFileOut);
    }

    // Processing TIS
    printMsg(OUTPUT_MSG, "Processing TIS file \"%s\"...\n", tisFile);
    int num_processed = 0;
    int tileCount, ofsTiles;
    FILE *fp finally(cleanFile) = parseTISFile(tisFile, &ofsTiles, &tileCount);
    if (!fp) return -1;
    uint8_t *pixels_pri finally(cleanMem8) = malloc(TILE_SIZE);
    uint8_t *pixels_sec finally(cleanMem8) = malloc(TILE_SIZE);
    uint8_t *pixels_pri_out finally(cleanMem8) = malloc(TILE_SIZE);
    uint8_t *pixels_sec_out finally(cleanMem8) = malloc(TILE_SIZE);
    for (int i = 0, imax = arrayGetSize(tileList); i < imax; ++i) {
        tile_t *tileInfo = (tile_t*)arrayGetItem(tileList, i);
        if (tileInfo->sec >= 0) {
            if (tileInfo->pri >= tileCount) {
                printMsg(OUTPUT_ERR, "Error: Invalid tile reference %d. Only %d tiles available in TIS file: %s\n", tileInfo->pri, tileCount, tisFile);
                return -1;
            }
            if (tileInfo->sec >= tileCount) {
                printMsg(OUTPUT_ERR, "Error: Invalid tile reference %d. Only %d tiles available in TIS file: %s\n", tileInfo->sec, tileCount, tisFile);
                return -1;
            }

            // reading primary tile
            fseek(fp, ofsTiles + tileInfo->pri * TILE_SIZE, SEEK_SET);
            if (fread(pixels_pri, 1, (size_t)TILE_SIZE, fp) != (size_t)TILE_SIZE) {
                printMsg(OUTPUT_ERR, "Error: Error reading tile %d from TIS file: %s\n", tileInfo->pri, tisFile);
                return -1;
            }
            // reading secondary tile
            fseek(fp, ofsTiles + tileInfo->sec * TILE_SIZE, SEEK_SET);
            if (fread(pixels_sec, 1, (size_t)TILE_SIZE, fp) != (size_t)TILE_SIZE) {
                printMsg(OUTPUT_ERR, "Error: Error reading tile %d from TIS file: %s\n", tileInfo->sec, tisFile);
                return -1;
            }

            // performing tile conversion
            switch (getMode(param_mode, pixels_pri)) {
            case MODE_TO_EE:
                if (!tileToEE(tileInfo, pixels_pri, pixels_sec, pixels_pri_out, pixels_sec_out)) return -1;
                break;
            case MODE_FROM_EE:
                if (!tileFromEE(tileInfo, pixels_pri, pixels_sec, pixels_pri_out, pixels_sec_out, tisFile)) return -1;
                break;
            default:
                return -1;
            }

            // writing primary output tile
            fseek(fp, ofsTiles + tileInfo->pri * TILE_SIZE, SEEK_SET);
            if (fwrite(pixels_pri_out, 1, (size_t)TILE_SIZE, fp) != (size_t)TILE_SIZE) {
                printMsg(OUTPUT_ERR, "Error: Error writing tile %d to TIS file: %s\n", tileInfo->pri, tisFile);
                return -1;
            }

            // writing secondary output tile
            fseek(fp, ofsTiles + tileInfo->sec * TILE_SIZE, SEEK_SET);
            if (fwrite(pixels_sec_out, 1, (size_t)TILE_SIZE, fp) != (size_t)TILE_SIZE) {
                printMsg(OUTPUT_ERR, "Error: Error writing tile %d to TIS file: %s\n", tileInfo->sec, tisFile);
                return -1;
            }

            num_processed++;
        }
    }

    return num_processed;
}


int getMode(int mode, const uint8_t *pixels_pri) {
    switch (mode) {
    case MODE_FROM_EE:
    case MODE_TO_EE:
        return mode;
    case MODE_AUTO:
    {
        int mode2 = MODE_TO_EE;
        if (pixels_pri) {
            if (((uint32_t*)pixels_pri)[0] == TRANSPARENT) {
                for (int i = 1024; i < TILE_SIZE; ++i) {
                    if (pixels_pri[i] == 0) {
                        mode2 = MODE_FROM_EE;
                        break;
                    }
                }
            }
        }
        return mode2;
    }
    default:
        return mode;
    }
}


bool tileToEE(const tile_t *tileInfo, const uint8_t *pixels_pri, const uint8_t *pixels_sec, uint8_t *pixels_pri_out, uint8_t *pixels_sec_out) {
    if (!tileInfo || !pixels_pri || !pixels_sec || !pixels_pri_out || !pixels_sec_out) {
        printMsg(OUTPUT_ERR, "Error: Internal error.\n");
        return false;
    }

    if (param_mode == MODE_AUTO)
        printMsg(OUTPUT_LOG, "Conversion mode for tiles (%d, %d): classic->EE\n", tileInfo->pri, tileInfo->sec);

    // preparing palette
    memcpy(pixels_pri_out, pixels_pri, TILE_SIZE);
    int col_idx = colorIndex(pixels_pri_out, 256, TRANSPARENT);
    if (col_idx < 0) {
        // adjusting palette
        uint8_t ci1, ci2;
        getMergeableColors(pixels_pri_out, &ci1, &ci2);
        adjustTileColors(pixels_pri_out, ci1, ci2);
        col_idx = 0;
    }
    memcpy(pixels_sec_out, pixels_pri_out, TILE_SIZE);

    // finalizing primary tile
    for (int p = 1024; p < TILE_SIZE; ++p)
        if (!pixels_sec[p])
            pixels_pri_out[p] = col_idx;

    // finalizing secondary tile
    for (int p = 1024; p < TILE_SIZE; ++p)
        if (pixels_sec[p])
            pixels_sec_out[p] = col_idx;

    return true;
}


bool tileFromEE(const tile_t *tileInfo, const uint8_t *pixels_pri, const uint8_t *pixels_sec, uint8_t *pixels_pri_out, uint8_t *pixels_sec_out, const char *tisFile) {
    if (!tileInfo || !pixels_pri || !pixels_sec || !pixels_pri_out || !pixels_sec_out) {
        printMsg(OUTPUT_ERR, "Error: Internal error.\n");
        return false;
    }

    if (param_mode == MODE_AUTO)
        printMsg(OUTPUT_LOG, "Conversion mode for tiles (%d, %d): EE->classic\n", tileInfo->pri, tileInfo->sec);

    // preparing primary output tile
    // assembling primary output tile from both input tiles
    uint32_t *pixels_rgba finally(cleanMem32) = malloc(TILE_DIM * TILE_DIM * sizeof(uint32_t));
    uint32_t *pal_pri = (uint32_t*)pixels_pri;
    uint32_t *pal_sec = (uint32_t*)pixels_sec;
    bool useTransparent = false;
#define OPAQUE 0xff000000
    for (int p = 1024; p < TILE_SIZE; ++p) {
        if (pixels_pri[p]) {
            pixels_rgba[p - 1024] = pal_pri[pixels_pri[p]] | OPAQUE;
        } else if (pixels_sec[p]) {
            pixels_rgba[p - 1024] = pal_sec[pixels_sec[p]] | OPAQUE;
        } else {
            pixels_rgba[p - 1024] = TRANSPARENT | OPAQUE;
            useTransparent = true;
        }
    }
#undef OPAQUE
    if (!evalOp(createRemappedTile(pixels_rgba, pixels_pri_out, useTransparent),
                "Error: Could not generate palette for tile %d in TIS file: %s\n", tileInfo->pri, tisFile)) return false;

    // fixing palette order
    int colIdx = colorIndex(pixels_pri_out, 256, TRANSPARENT);
    if (colIdx > 0) {
        // swapping palette entry
        uint32_t *pal = (uint32_t*)pixels_pri_out;
        pal[0] ^= pal[colIdx];
        pal[colIdx] ^= pal[0];
        pal[0] ^= pal[colIdx];
        // swapping color indices
        for (int p = 1024; p < TILE_SIZE; ++p) {
            if (pixels_pri_out[p] == 0)
                pixels_pri_out[p] = colIdx;
            else if (pixels_pri_out[p] == colIdx)
                pixels_pri_out[p] = 0;
        }
    }

    // preparing secondary output tile
    memcpy(pixels_sec_out, pixels_pri, TILE_SIZE);

    return true;
}


bool parseWED(const char *wedFile, char *tisName, array_t *tileList) {
    if (!wedFile || !tisName || !tileList) {
        printMsg(OUTPUT_ERR, "Internal error.\n");
        return false;
    }

    if (!fileExists(wedFile)) {
        printMsg(OUTPUT_ERR, "WED file not found: %s\n", wedFile);
        return false;
    }

    FILE *fp finally(cleanFile) = fopen(wedFile, "rb");
    if (!evalOp(fp != NULL, "Error: Unable to open WED file: %s\n", wedFile)) return false;

    // speed up parsing by loading whole file into memory
    fseek(fp, 0, SEEK_END);
    long file_size = ftell(fp);
    if (!evalOp(file_size >= 0, "Error: Could not read from WED file: %s\n", wedFile)) return false;
    void *data finally(cleanMem) = malloc(file_size);
    fseek(fp, 0, SEEK_SET);
    if (!evalOp(fread(data, 1, (size_t)file_size, fp) == (size_t)file_size, "Error: Unexpected end of file: %s\n", wedFile)) return false;

    // parsing overlay data
    char sig[9] = {0};
    int32_t ofs_ovl;
    if (!getString(data, 0, 8, sig)) return false;
    if (!evalOp(strcmp(sig, "WED V1.3") == 0, "Error: Not a valid WED file: %s\n", wedFile)) return false;
    if (!getLong(data, 0x10, &ofs_ovl)) return false;

    // getting TIS filename
    char tisResref[9] = {0};
    if (!getString(data, ofs_ovl + 4, 8, tisResref)) return false;
    lowerString(tisResref);
    if (!evalOp(strlen(tisResref), "Error: No TIS file referenced in WED file: %s\n", wedFile)) return false;
    sprintf(tisName, "%s.tis", tisResref);

    // getting tile information
    int32_t ofs_tilemap, ofs_lookup;
    int16_t num_width, num_height;
    if (!getShort(data, ofs_ovl, &num_width)) return false;
    if (!getShort(data, ofs_ovl + 2, &num_height)) return false;
    if (!getLong(data, ofs_ovl + 0x10, &ofs_tilemap)) return false;
    if (!getLong(data, ofs_ovl + 0x14, &ofs_lookup)) return false;
    size_t num_tiles = num_width * num_height;
    if (arrayExpand(tileList, num_tiles) != num_tiles) {
        printMsg(OUTPUT_ERR, "Error: Not enough memory to process tileset.\n");
        return false;
    }
    for (size_t i = 0, ofs = ofs_tilemap; i < num_tiles; i++, ofs += 10) {
        int16_t tile_sec, tile_pri_idx, tile_pri;
        int8_t flags;
        int pri = -1, sec = -1;
        if (!getShort(data, ofs + 4, &tile_sec)) return false;
        if (!getByte(data, ofs + 6, &flags)) return false;
        if (flags) {
            if (!getShort(data, ofs, &tile_pri_idx)) return false;
            if (!getShort(data, ofs_lookup + tile_pri_idx * 2, &tile_pri)) return false;
            pri = tile_pri;
            sec = tile_sec;
        }
        if (pri != -1 && sec != -1) {
            tile_t *t = malloc(sizeof(tile_t));
            t->pri = pri;
            t->sec = sec;
            arrayAddItem(tileList, t);
        }
    }

    return true;
}


FILE* parseTISFile(const char *tisFile, int *ofsTiles, int *tileCount) {
    if (!tisFile) return NULL;

    char sig[9] = {0};
    int32_t count, size, ofs, dim;
    FILE *fp = fopen(tisFile, "r+b");
    if (!evalOp(fp != NULL, "Error: Unable to open TIS file: %s\n", tisFile)) return NULL;
    if (!readString(fp, 0, 8, sig)) { fclose(fp); return NULL; }
    if (!evalOp(strcmp(sig, "TIS V1  ") == 0, "Error: Not a valid TIS file: %s\n", tisFile)) { fclose(fp); return NULL; }
    if (!readLong(fp, 0x08, &count)) { fclose(fp); return NULL; }
    if (!readLong(fp, 0x0c, &size)) { fclose(fp); return NULL; }
    if (!evalOp(size == TILE_SIZE, "Error: Not a palette-based TIS file: %s\n", tisFile)) { fclose(fp); return NULL; }
    if (!readLong(fp, 0x10, &ofs)) { fclose(fp); return NULL; }
    if (!readLong(fp, 0x14, &dim)) { fclose(fp); return NULL; }
    if (!evalOp(dim == TILE_DIM, "Error: Unexpected tile size: %d\n", dim)) { fclose(fp); return NULL; }

    if (tileCount) *tileCount = count;
    if (ofsTiles) *ofsTiles = ofs;
    return fp;
}


bool findTISFile(array_t *searchPath, const char *tisName, char *tisFile) {
    if (tisName && tisFile) {
        if (searchPath && arrayGetSize(searchPath)) {
            char path[FILENAME_MAX];
            for (size_t i = 0; i < arrayGetSize(searchPath); ++i) {
                sprintf(path, "%s/%s", (char*)arrayGetItem(searchPath, i), tisName);
                if (fileExists(path)) {
                    strcpy(tisFile, path);
                    return true;
                }
            }
        } else {
            if (fileExists(tisName)) {
                strcpy(tisFile, tisName);
                return true;
            }
        }
    }
    return false;
}
