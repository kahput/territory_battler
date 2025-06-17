#include "darray.h"
#include <raylib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct _dynamic_array {
	uint32_t element_size;
	uint32_t count, capacity;
} DArray;

void* darray_alloc(uint32_t element_size, uint32_t capcity) {
	DArray* darr = malloc(sizeof(DArray) + (element_size * capcity));
	darr->element_size = element_size;
	darr->capacity = capcity;
	darr->count = 0;

	memset(darr + 1, 0, capcity * element_size);
	return darr + 1;
}

void darray_free(void** array) {
	DArray* darr = ((DArray*)(*array)) - 1;
	// printf("Count = %i, Capacity = %i\n", arr->capacity, arr->count);
	free(darr);
}

bool darray_push(void** array_ptr, void* element) {
	if (!array_ptr || !*array_ptr || !element) {
		return false;
	}

	void* array = *array_ptr;
	DArray* darr = (DArray*)array - 1;

	if (darr->count >= darr->capacity) {
		uint32_t new_capacity = (darr->capacity == 0) ? 1 : darr->capacity * 2;

		void* new_darr = realloc(darr, sizeof(DArray) + new_capacity * darr->element_size);
		if (!new_darr) {
			return false;
		}

		/* Update the pointer and capacity */
		darr = new_darr;
		darr->capacity = new_capacity;

		/* Update the user's pointer */
		*array_ptr = darr + 1;
		array = *array_ptr;
	}

	/* Add the element */
	void* dst = (uint8_t*)array + (darr->count * darr->element_size);
	memcpy(dst, element, darr->element_size);
	darr->count++;

	return true;
}

bool darray_pop(void **array_ptr) {
	if (!array_ptr || !*array_ptr) {
		return false;
	}

	void *array = *array_ptr;
	DArray*darr = (DArray*)array - 1;

	if (darr->count <= 0) {
		return false;
	}

	darr->count--;
	return true;
}

uint32_t darray_length(void* array) {
	DArray* darr = ((DArray*)array) - 1;
	return darr->count;
}
uint32_t darray_capacity(void* array) {
	DArray* darr = ((DArray*)array) - 1;
	return darr->capacity;
}
