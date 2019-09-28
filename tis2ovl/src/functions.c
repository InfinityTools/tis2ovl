#include <dirent.h>
#include <ctype.h>
#include <stdarg.h>
#include <string.h>
#include "functions.h"
#include "global.h"
#include "compat.h"

#ifdef _WIN32
#   include <windows.h>
#else
#   include <sys/stat.h>
#endif

int printMsg(int outputType, const char *format, ...) {
    bool show = false;
    FILE *ch = NULL;
    switch (outputType) {
    case OUTPUT_LOG:
        show = !param_quiet && param_verbose;
        ch = stdout;
        break;
    case OUTPUT_ERR:
        show = !param_quiet;
        ch = stdout;
        break;
    default:
        show = true;
        ch = stderr;
    }
    if (ch && show && format) {
        va_list args;
        va_start(args, format);
        return vfprintf(ch, format, args);
    }
    return 0;
}

bool sort(void *data, size_t size, size_t count, fnGT cmp) {
    if (data) {
        uint8_t *tmp finally(cleanMem8) = malloc(size);
        uint8_t *ptr = data;
        size_t n = count;
        do {
            size_t newn = 0;
            for (size_t i = 1; i < n; i++) {
                void *item1 = ptr + (i-1)*size;
                void *item2 = ptr + i*size;
                if ((cmp && cmp(item1, item2)) || memcmp(item1, item2, size) > 0) {
                    memcpy(tmp, item1, size);
                    memcpy(item1, item2, size);
                    memcpy(item2, tmp, size);
                    newn = i;
                }
            }
            n = newn;
        } while (n > 1);
        return true;
    }
    return false;
}

size_t unique(void *data, size_t size, size_t count, fnEq eq, fnDiscard discard) {
    size_t retVal = count;
    uint8_t *ptr = data;
    for (size_t i = 0; i < count; ++i) {
        void *item1 = ptr + i*size;
        void *item2 = ptr + (i-1)*size;
        if ((eq && eq(item1, item2)) || memcmp(item1, item2, size) == 0) {
            if (!discard || discard(item1, item2)) {
                item2 = ptr + (i+1)*size;
                memmove(item1, item2, (count - i - 1)*size);
                retVal--;
            }
        }
    }
    return retVal;
}

char* lowerString(char *str) {
    if (str)
        for (char *p = str; *p; ++p)
            *p = tolower(*p);
    return str;
}

char* upperString(char *str) {
    if (str)
        for (char *p = str; *p; ++p)
            *p = toupper(*p);
    return str;
}

bool fileExists(const char *fileName) {
    bool retVal = false;
    if (fileName && *fileName) {
        FILE *fp = fopen(fileName, "r");
        if (fp) {
            retVal = true;
            fclose(fp);
        }
    }
    return retVal;
}

bool directoryExists(const char *pathName) {
    bool retVal = false;
    if (pathName && *pathName) {
        DIR *dp = opendir(pathName);
        if (dp) {
                retVal = true;
            closedir(dp);
        }
    }
    return retVal;
}

bool isFileIdentical(const char *fileName1, const char *fileName2) {
    if (!fileName1 || !fileName2) return false;
    if (strcmp(fileName1, fileName2) == 0) return true;

#ifdef _WIN32
    // Windows
    BY_HANDLE_FILE_INFORMATION fi1, fi2;
    HANDLE hf1 = CreateFile(fileName1, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hf1 == INVALID_HANDLE_VALUE)
        return false;

    HANDLE hf2 = CreateFile(fileName2, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hf2 == INVALID_HANDLE_VALUE) {
        CloseHandle(hf1);
        return false;
    }

    if (!GetFileInformationByHandle(hf1, &fi1) || !GetFileInformationByHandle(hf2, &fi2)) {
        CloseHandle(hf2);
        CloseHandle(hf1);
        return false;
    }
    CloseHandle(hf2);
    CloseHandle(hf1);

    return (fi1.dwVolumeSerialNumber == fi2.dwVolumeSerialNumber &&
            fi1.nFileIndexHigh == fi2.nFileIndexHigh &&
            fi1.nFileIndexLow == fi2.nFileIndexLow);
#else   // _WIN32
    // Unix
    struct stat stat1;
    if (stat(fileName1, &stat1) < 0) return false;
    struct stat stat2;
    if (stat(fileName2, &stat2) < 0) return false;

    return (stat1.st_dev == stat2.st_dev &&
            stat1.st_ino == stat2.st_ino);
#endif  // _WIN32
}

char* normalizeDir(char *str) {
    if (str) {
        for (int i = strlen(str) - 1; i >= 0; --i)
            if (str[i] == '/' || str[i] == '\\')
                str[i] = '\0';
        if (!*str) {
            str[0] = '.';
            str[1] = '\0';
        }
    }
    return str;
}

bool copyFile(const char *srcFile, const char *dstFile, bool overwrite) {
    if (srcFile && dstFile) {
        if (!overwrite && fileExists(dstFile)) return false;
        FILE *fin = fopen(srcFile, "rb");
        if (!fin) return false;
        FILE *fout = fopen(dstFile, "wb");
        if (!fout) { fclose(fin); return false; }
#define BUF_SIZE 65536
        void *buf finally(cleanMem) = malloc(BUF_SIZE);
        size_t len, fsize = 0;
        do {
            len = fread(buf, 1, BUF_SIZE, fin);
            if (len < BUF_SIZE && ferror(fin)) {
                fclose(fout);
                fclose(fin);
                remove(dstFile);
                return false;
            }
            if (fwrite(buf, 1, len, fout) < len) {
                fclose(fout);
                fclose(fin);
                remove(dstFile);
                return false;
            }
            fsize += len;
        } while (!feof(fin));
#undef BUF_SIZE
        fclose(fout);
        fclose(fin);
        return true;
    }
    return false;
}

bool getString(void *ptr, int ofs, int len, char *str) {
    if (ptr && ofs >= 0 && len >= 0 && str) {
        memcpy(str, (int8_t*)ptr + ofs, len);
        str[len] = '\0';
        return true;
    }
    return false;
}

bool getByte(void *ptr, int ofs, int8_t *value) {
    if (ptr && ofs >= 0 && value) {
        memcpy(value, (int8_t*)ptr + ofs, 1);
        return true;
    }
    return false;
}

bool getShort(void *ptr, int ofs, int16_t *value) {
    if (ptr && ofs >= 0 && value) {
        memcpy(value, (int8_t*)ptr + ofs, 2);
        return true;
    }
    return false;
}

bool getLong(void *ptr, int ofs, int32_t *value) {
    if (ptr && ofs >= 0 && value) {
        memcpy(value, (int8_t*)ptr + ofs, 4);
        return true;
    }
    return false;
}

bool readString(FILE *fp, int ofs, int len, char *str) {
    if (fp && ofs >= 0 && len >= 0 && str) {
        if (fseek(fp, ofs, SEEK_SET) == 0) {
            if (fread(str, sizeof(char), (size_t)len, fp) == (size_t)len) {
                str[len] = '\0';
                return true;
            } else {
                printMsg(OUTPUT_ERR, "Error: Could not read string of length of %d at offset %d\n", len, ofs);
            }
        }
    }
    return false;
}

bool readByte(FILE *fp, int ofs, int8_t *value) {
    if (fp && ofs >= 0 && value) {
        if (fseek(fp, ofs, SEEK_SET) == 0) {
            if (fread(value, 1, 1, fp) == 1) {
                return true;
            } else {
                printMsg(OUTPUT_ERR, "Error: Could not read byte value at offset %d\n", ofs);
            }
        }
    }
    return false;
}

bool readShort(FILE *fp, int ofs, int16_t *value) {
    if (fp && ofs >= 0 && value) {
        if (fseek(fp, ofs, SEEK_SET) == 0) {
            if (fread(value, 2, 1, fp) == 1) {
                return true;
            } else {
                printMsg(OUTPUT_ERR, "Error: Could not read short value at offset %d\n", ofs);
            }
        }
    }
    return false;
}

bool readLong(FILE *fp, int ofs, int32_t *value) {
    if (fp && ofs >= 0 && value) {
        if (fseek(fp, ofs, SEEK_SET) == 0) {
            if (fread(value, 4, 1, fp) == 1) {
                return true;
            } else {
                printMsg(OUTPUT_ERR, "Error: Could not read long value at offset %d\n", ofs);
            }
        }
    }
    return false;
}

bool evalOp(bool condition, const char *fmt, ...) {
    if (!condition && fmt) {
        va_list args;
        va_start(args, fmt);
        vfprintf(stderr, fmt, args);
    }
    return condition;
}
