#ifndef FUNCTIONS_H_INCLUDED
#define FUNCTIONS_H_INCLUDED

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

/// Message output types, used by printMsg(), in ascending priority.
enum OUTPUT_TYPE { OUTPUT_LOG, OUTPUT_MSG, OUTPUT_ERR };

// Function prototype: Determine whether first argument is "greater" than second argument.
typedef bool (*fnGT)(const void*, const void*);
// Function prototype: Determine whether first argument is "equal to" second argument.
typedef fnGT fnEq;
// Function prototype: Allows final cleanup of discarded element. Returns whether discarded element is allowed to be discarded.
typedef bool (*fnDiscard)(void *discard, void *remain);

/**
 * Print a message of the specified output type.
 * \param outputType    One of the constants LOG, MSG and ERR (see OUTPUT_TYPE enum).
 * \param format        Formatted string with variable arguments to be displayed.
 */
int printMsg(int outputType, const char *format, ...);

/// Sort "data" with "count" elements of "size" bytes each by using function "cmp". Omit "cmp" to compare raw memory.
bool sort(void *data, size_t size, size_t count, fnGT cmp);

/// Remove duplicate entries from the sorted array "data" with "count" elements of "size" bytes each.
/// "discard" is called to allow final cleanup of discarded element.
size_t unique(void *data, size_t size, size_t count, fnEq eq, fnDiscard discard);

/// To-lower given string.
char* lowerString(char *str);

/// To-upper given string.
char* upperString(char *str);

/// Return whether fileName refers to an existing file.
bool fileExists(const char *fileName);

/// Return whether pathName refers to an existing path.
bool directoryExists(const char *pathName);

/**
 * Check if specified filenames reference the same file.
 * \param fileName1 Path to first file.
 * \param fileName2 Path to second file.
 * \return true if both files exist and are identical. Returns false otherwise.
 */
bool isFileIdentical(const char *fileName1, const char *fileName2);

/// Remove excess path separators from specified path string.
char* normalizeDir(char *str);

/// Copy source file to destination. Existing destination will be overwritten if "overwrite" is true. Otherwise function will return false.
bool copyFile(const char *srcFile, const char *dstFile, bool overwrite);

/// Extract string of given length to str.
bool getString(void *ptr, int ofs, int len, char *str);

/// Retrieve byte value from buffer and store it in value.
bool getByte(void *ptr, int ofs, int8_t *value);

/// Retrieve short value from buffer and store it in value.
bool getShort(void *ptr, int ofs, int16_t *value);

/// Retrieve long value from buffer and store it in value.
bool getLong(void *ptr, int ofs, int32_t *value);

/// Read string of given length from file and store it in value.
bool readString(FILE *fp, int ofs, int len, char *str);

/// Read byte value from file and store it in value.
bool readByte(FILE *fp, int ofs, int8_t *value);

/// Read short value from file and store it in value.
bool readShort(FILE *fp, int ofs, int16_t *value);

/// Read long value from file and store it in value.
bool readLong(FILE *fp, int ofs, int32_t *value);

/// Helper function: Print message if condition fails. Return specified condition.
bool evalOp(bool condition, const char *fmt, ...);



#endif // FUNCTIONS_H_INCLUDED
