#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>
#include <ctype.h>
#include <unistd.h>
#include "global.h"
#include "version.h"
#include "functions.h"
#include "arrays.h"
#include "tis2ovl.h"

int main(int argc, char *argv[])
{
    setlocale(LC_ALL, "");
    isFileIdentical("a", "b");

    if (argc < 2) {
        printHelp(TIS2OVL_NAME);
        return EXIT_SUCCESS;
    }

    int errors = 0;
    char *outputDir = NULL;
    array_t wedList, searchList;
    arrayInit(&searchList, 0);
    arrayInit(&wedList, 0);

    // parsing cmd options
    opterr = 0; // no automatic error messages
    int c;
    while ((c = getopt(argc, argv, "ceqxhvs:o:")) != -1) {
        switch (c) {
        case 'c':
            param_mode |= MODE_TO_EE;
            break;
        case 'e':
            param_mode |= MODE_FROM_EE;
            break;
        case 'q':
            param_quiet = true;
            break;
        case 'x':
            param_verbose = true;
            break;
        case 'h':
            printHelp(TIS2OVL_NAME);
            return EXIT_SUCCESS;
        case 'v':
            printVersion();
            return EXIT_SUCCESS;
        case 's':
            if (directoryExists(optarg)) {
                optarg = normalizeDir(optarg);
                arrayAddItem(&searchList, optarg);
            } else {
                printMsg(OUTPUT_ERR, "Error: Search path does not exist: %s. Skipping.\n", optarg);
            }
            break;
        case 'o':
            if (directoryExists(optarg)) {
                optarg = normalizeDir(optarg);
                outputDir = optarg;
            } else {
                printMsg(OUTPUT_ERR, "Error: Output directory does not exist: %s\n", optarg);
                return EXIT_FAILURE;
            }
            break;
        case '?':
            if (optopt == 's' || optopt == 'o') {
                printMsg(OUTPUT_ERR, "Error: Option -%c requires an argument.\n", optopt);
            } else if (isprint(optopt)) {
                printMsg(OUTPUT_ERR, "Error: Unknown option: -%c\n", optopt);
            } else {
                printMsg(OUTPUT_ERR, "Error: Unknown option character: \\x%x\n", optopt);
            }
            return EXIT_FAILURE;
        }
    }
    if (arrayGetSize(&searchList) == 0)
        arrayAddItem(&searchList, "");
    if (param_mode == MODE_NONE)
        param_mode = MODE_AUTO;

    // fetching remaining arguments
    for (int i = optind; i < argc; ++i) {
        if (fileExists(argv[i])) {
                arrayAddItem(&wedList, argv[i]);
        } else {
            printMsg(OUTPUT_ERR, "Error: WED file does not exist: %s. Skipping.\n", argv[i]);
            errors++;
        }
    }

    printMsg(OUTPUT_MSG, "Using configuration:\n");
    switch (param_mode) {
    case MODE_AUTO:
        printMsg(OUTPUT_MSG, "  Conversion mode: Autodetect\n");
        break;
    case MODE_TO_EE:
        printMsg(OUTPUT_MSG, "  Conversion mode: to EE\n");
        break;    case MODE_FROM_EE:
        printMsg(OUTPUT_MSG, "  Conversion mode: from EE\n");
        break;
    }
    printMsg(OUTPUT_MSG, "  Quiet mode: %s\n", param_quiet ? "enabled" : "disabled");
    size_t num = arrayGetSize(&searchList);
    if (num > 1) {
        for (size_t i = 0, imax = arrayGetSize(&searchList); i < imax; ++i) {
            char *path = arrayGetItem(&searchList, i);
            printMsg(OUTPUT_MSG, "  TIS search path %d: %s\n", i+1, (path && *path) ? path : "(current directory)");
        }
    } else {
        char *path = arrayGetItem(&searchList, 0);
        printMsg(OUTPUT_MSG, "  TIS search path: %s\n", (path && *path) ? path : "(current directory)");
    }
    if (outputDir) printMsg(OUTPUT_MSG, "  Output directory: %s\n", *outputDir ?  outputDir : "(current directory)");
        else printMsg(OUTPUT_MSG, "  Output directory: (update input files)\n");
    printMsg(OUTPUT_MSG, "  Found %d input WED file(s)\n", arrayGetSize(&wedList));
    printMsg(OUTPUT_MSG, "\n");

    // performing conversion
    for (size_t idx = 0; idx < arrayGetSize(&wedList); ++idx) {
        int num = 0;
        num = convert((char*)arrayGetItem(&wedList, idx), &searchList, outputDir);
        if (num >= 0) {
            printMsg(OUTPUT_MSG, "Tileset converted successfully. %d tiles updated.\n\n", num);
        } else {
            errors++;
            putchar('\n');
        }
    }

    if (errors) {
        if (arrayGetSize(&wedList) > 1)
            printMsg(OUTPUT_MSG, "Conversion finished with %d error(s).\n", errors);
        return EXIT_FAILURE;
    } else {
        return EXIT_SUCCESS;
    }
}
