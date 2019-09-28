#include <stdlib.h>
#include <string.h>
#include "arrays.h"

#define DEFAULT_CAPACITY 16

bool arrayEnsureCapacity(array_t*);

size_t arrayInit(array_t *array, size_t capacity) {
    if (array) {
        if (!capacity) capacity = DEFAULT_CAPACITY;
        void **data = malloc(sizeof(void*) * capacity);
        if (data) {
            array->data = data;
            array->len = 0;
            array->cap = capacity;
            return array->cap;
        }
    }
    return 0;
}

void arrayFree(array_t *array) {
    if (array) {
        free(array->data);
        array->data = NULL;
        array->len = array->cap = 0;
    }
}

size_t arrayCompact(array_t *array){
    if (array) {
        if (array->cap > array->len + 1) {
            size_t cap = array->len + 1;
            void **data = malloc(sizeof(void*) * cap);
            if (data) {
                memcpy(data, array->data, sizeof(void*) * array->len);
                free(array->data);
                array->data = data;
                array->cap = cap;
            }
        }
        return array->cap;
    }
    return 0;
}

size_t arrayExpand(array_t *array, size_t capacity) {
    if (array) {
        if (capacity == 0) capacity = DEFAULT_CAPACITY;
        if (capacity > array->cap) {
            void **data = malloc(sizeof(void*) * capacity);
            if (data) {
                memcpy(data, array->data, sizeof(void*) * array->len);
                free(array->data);
                array->data = data;
                array->cap = capacity;
            }
        }
        return array->cap;
    }
    return 0;
}

bool arrayAddItem(array_t *array, void *item) {
    if (!arrayEnsureCapacity(array)) return false;
    array->data[array->len] = item;
    array->len++;
    return (array != NULL);
}

bool arrayInsertItem(array_t *array, size_t index, void *item) {
    if (!arrayEnsureCapacity(array)) return false;
    if (index == array->len)
        return arrayAddItem(array, item);
    else if (index < array->len) {
        for (size_t i = array->len; i > index; --i)
            array->data[i] = array->data[i-1];
        array->data[index] = item;
        array->len++;
        return true;
    }
    return false;
}

void* arrayRemoveItem(array_t *array, size_t index) {
    void *retVal = NULL;
    if (array && index < array->len) {
        retVal = array->data[index];
        for (size_t i = index + 1; i < array->len; ++i) {
            array->data[i-1] = array->data[i];
        }
        array->len--;
        array->data[array->len] = NULL;
    }
    return retVal;
}

void arrayClear(array_t *array, bool release) {
    if (array) {
        if (release) {
            for (size_t i = 0; i < array->len; ++i) {
                if (array->data[i]) {
                    free(array->data[i]);
                    array->data[i] = NULL;
                }
            }
        }
        array->len = 0;
    }
}


// Internally used. Ensures that array has at least one free slot available.
bool arrayEnsureCapacity(array_t *array) {
    if (array) {
        if (array->len >= array->cap) {
            size_t cap = ((array->cap < DEFAULT_CAPACITY) ? DEFAULT_CAPACITY : array->cap) * 2;
            void **data = malloc(sizeof(void*) * cap);
            if (!data) {
                cap = array->cap * 3 / 2;
                data = malloc(sizeof(void*) * cap);
            }
            if (!data) return false;
            if (data) {
                memcpy(data, array->data, sizeof(void*) * array->cap);
                free(array->data);
                array->data = data;
                array->cap = cap;
            }
        }
        return true;
    }
    return false;
}
