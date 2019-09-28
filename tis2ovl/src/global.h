#ifndef GLOBAL_H_INCLUDED
#define GLOBAL_H_INCLUDED

#include <stdbool.h>

/// Available tile conversion modes.
enum MODE { MODE_NONE = 0, MODE_TO_EE = 1, MODE_FROM_EE = 2, MODE_AUTO = 3};

/// Indicates whether log messages are printed.
extern bool param_quiet;

/// Indicates whether verbose log messages are printed.
extern bool param_verbose;

/// Specified conversion mode.
extern int param_mode;

#endif // GLOBAL_H_INCLUDED
