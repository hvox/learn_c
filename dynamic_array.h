#ifndef DYNAMIC_ARRAY_H
#define DYNAMIC_ARRAY_H
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#define new_array_of_type(type) ((type *) new_array(0, sizeof(type)))

struct array_info {
	uint16_t len;
	uint16_t element_size;
	uint16_t allocated;
};

void *new_array(uint16_t length, uint16_t element_size) {
	struct array_info info = {length, element_size, length};
	void *mem = malloc(length * element_size + sizeof(info));
	*((struct array_info *) mem) = info;
	return memset(mem + sizeof(info), 0, length * element_size);
}

void del_array(void *array) {
	free(array - sizeof(struct array_info));
}

void *push(void *array, void *element) {
	struct array_info *info = ((struct array_info *) array - 1);
	if (info->len == info->allocated) {
		uint16_t new_len = info->len * 3 / 2 + 1;
		array = sizeof(struct array_info) + realloc(
			array - sizeof(struct array_info),
			new_len * info->element_size + sizeof(struct array_info)
		);
		info = ((struct array_info *) array - 1);
		info->allocated = new_len;
	}
	size_t offset = info->element_size * (info->len)++;
	memcpy(array + offset, element, info->element_size);
	return array;
}

uint16_t len(void *array) {
	return ((struct array_info *) array - 1)->len;
}

#endif
