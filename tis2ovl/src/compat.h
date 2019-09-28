#ifndef COMPAT_H_INCLUDED
#define COMPAT_H_INCLUDED

// Cleanup functionality requires gcc 4.0 or later.
#ifndef __GNUC__
# error "GNU C compiler required"
#elif __GNUC__ < 4
# error "GNU C compiler version 4 or higher required"
#endif

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

// Shortcut for assigning cleanup functions to variable declaration when they go out of scope
#define finally(func) __attribute((cleanup (func)))

// Auto-generate cleanup function of given name for given type
#define def_cleanFunc(name, type) void name(type * pvar) { if (pvar) free(*pvar); }

void cleanMem(void**);
void cleanMem8(uint8_t**);
void cleanMem32(uint32_t**);
void cleanBool(bool**);
void cleanFile(FILE**);

#endif // COMPAT_H_INCLUDED
