# tis2ovl
*Command line tool for converting tileset overlays between classic BG2 and Enhanced Edition games.*

## Overview

Tilesets with (e.g. water or lava) overlays are not directly interchangeable between the original BG2 engine and the Enhanced Editions, as both engines expect different overlay mask definitions. This tool allows you to convert tilesets between the two engine variants.

## Usage

```
Usage: tis2ovl [OPTIONS]... WEDFILE...

Retrieve information from WEDFILE(s) to convert tileset (TIS) overlays between classic BG2 and
Enhanced Edition games.

Options:
  -c            Convert TIS overlays from classic to Enhanced Edition mode.
  -e            Convert TIS overlays from Enhanced Edition to classic mode.
                (Note: Omit -c and -e to autodetect TIS overlay conversion mode.)
  -s path       Search path for TIS files. This option can be specified
                multiple times. Default: current directory
  -o out_path   Output directory for TIS files. Omit to update original files instead.
  -q            Enable quiet mode. Do not print any log messages to standard output.
  -h            Print this help and exit.
  -v            Print version information and exit.
```

TIS file names are retrieved from the specified WED files. They are searched for in the specified search paths, or the current directory if no search path has been specified. Only palette-based tilesets are supported by the tool.

**Note:** On systems with case-sensitive filesystems TIS filenames are assumed to be lower-cased.

## Examples

This call converts the tileset referenced in AR1000.WED, which also has to be present in the current directory. Conversion mode will be autodetected by the tool. Original TIS file will be overwritten.
```
tis2ovl AR1000.WED
```

This call converts the tilesets referenced by AR1000.WED and AR1001.WED, which are searched for in the "tis_input" subfolder. The resulting TIS files will be saved in the "tis_output" subfolder. Only tilesets with overlay tile definitions compatible with classic BG2 will be considered for the conversion.
```
tis2ovl -c -s tis_input -o tis_output AR1000.WED AR1001.WED
```

## Building from source

Requirements:
- A toolchain that provides a GNU C compatible compiler at version 4.0 or later (e.g. MinGW on Windows)
- CMake at version 3.1 or later (https://cmake.org/)
- libimagequant (https://pngquant.org/lib/)

By default the static library for libimagequant is expected in the `lib/libimagequant` subfolder.

Building:
1. mkdir build && cd build
2. Create Makefile:
    - Unix/macOS: `cmake .. -DCMAKE_BUILD_TYPE=Release`
    - Windows: `cmake -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release`
3. Build executable:
    - Unix/macOS: `make`
    - Windows: `mingw32-make`

## License

"tis2ovl" is distributed under the terms and conditions of the MIT license. See LICENSE file for more information.
