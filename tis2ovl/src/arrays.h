#ifndef ARRAYS_H_INCLUDED
#define ARRAYS_H_INCLUDED

#include <stddef.h>
#include <stdbool.h>

// Array structure
typedef struct {
    void **data;    // array of pointers
    size_t len;     // number of items in the array
    size_t cap;     // maximum capacity
} array_t;

/// Create empty array with specified capacity. Returns initialized capacity. Existing array data will be discarded in the process.
size_t arrayInit(array_t *array, size_t capacity);

/// Release specified array from memory. Array can't be used until initialized again by arrayInit().
void arrayFree(array_t *array);

/// Return number of items in the array.
static inline size_t arrayGetSize(const array_t *array) { return array ? array->len : 0; }

/// Return maximum capacity of the array without having to perform an array expansion.
static inline size_t arrayGetCapacity(const array_t *array) { return array ? array->cap : 0; }

/// Attempt to reduce array capacity. Returns new array capacity.
size_t arrayCompact(array_t *array);

/// Attempts to increase array capacity to the specified value. Returns new array capacity.
size_t arrayExpand(array_t *array, size_t capacity);

/// Return specified array item. Returns NULL on error.
static inline void* arrayGetItem(const array_t *array, size_t index) { return (array && index < array->len) ? array->data[index] : NULL; }

/// Add given item to the end of the array. Capacity is expanded automatically if needed.
bool arrayAddItem(array_t *array, void *item);

/// Insert given item into the array at the specified position.
bool arrayInsertItem(array_t *array, size_t index, void *item);

/// Remove item from array at specified position. Returns removed item or NULL if unavailable.
void* arrayRemoveItem(array_t *array, size_t index);

/// Remove all items from the array without adjusting capacity. Specify "release" whether to explicitly release all pointers in the array.
void arrayClear(array_t *array, bool release);



#endif // ARRAYS_H_INCLUDED
