#ifndef TIS2OVL_H_INCLUDED
#define TIS2OVL_H_INCLUDED

#include <stdbool.h>
#include "global.h"
#include "arrays.h"

/// Print usage information.
void printHelp(const char *name);

/// Print version information.
void printVersion();

/// Performs tileset conversion based on "mode".
int convert(const char *wedFile, array_t *searchPath, const char *outputDir);


/// Performs tileset conversion from classic into EE format.
int convertToEE(const char *wedFile, array_t *searchPath);

/// Performs tileset conversion from EE into classic format.
int convertFromEE(const char *wedFile, array_t *searchPath);


#endif // TIS2OVL_H_INCLUDED
