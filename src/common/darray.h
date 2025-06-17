#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

void* darray_alloc(uint32_t element_size, uint32_t capcity);
void darray_free(void** array);

bool darray_push(void** array_ptr, void* element);
bool darray_pop(void** array_ptr) ;

uint32_t darray_length(void* array);
uint32_t darray_capacity(void* array);
