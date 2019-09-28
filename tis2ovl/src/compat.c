#include "compat.h"

void cleanMem(void **pvar) {
    if (pvar) free(*pvar);
}

void cleanMem8(uint8_t **pvar) {
    if (pvar) free(*pvar);
}

void cleanMem32(uint32_t **pvar) {
    if (pvar) free(*pvar);
}

void cleanBool(bool **pvar) {
    if (pvar) free(*pvar);
}

void cleanFile(FILE **pvar) {
    if (pvar && *pvar) {
        fclose(*pvar);
        *pvar = NULL;
    }
}
