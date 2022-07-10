#ifndef PARSER_H
#define PARSER_H
#include <fcntl.h>
#include <sys/stat.h>
#include <stdio.h>
#include <unistd.h>
#include "controls.h"
#include "dynamic_array.h"

struct device_id {
	char name[80];
	uint16_t vendor_id;
	uint16_t product_id;
};

struct binding {
	uint16_t control;
	uint16_t action;
	int8_t direction;
};

struct device_configuration {
	struct device_id id;
	char **actions;
	struct control *controls;
	struct binding *bindings;
};

uint64_t scan_uint(char *src, int size) {
	uint64_t number = 0;
	for (int i = 0; i < size; i++) {
		uint64_t digit = src[i] - 48;
		if (digit > 9) digit -= 7;
		number = number * 16 + digit;
	}
	return number;
}

struct device_id scan_device_id(char *src) {
	struct device_id id;
	id.vendor_id = scan_uint(src + 1, 4);
	id.product_id = scan_uint(src + 6, 4);
	size_t name_len = strlen(src + 11) > 79 ? 79 : strlen(src + 11);
	strncpy(id.name, src + 11, name_len);
	return id;
}

size_t scan_direction(char *src, int8_t *direction) {
	if (is_prefix(src, "positive ")) {
		*direction = 1;
		return sizeof("positive ");
	}
	if (is_prefix(src, "negative ")) {
		*direction = -1;
		return sizeof("negative ");
	}
	if (is_prefix(src, "inversed ")) {
		*direction = -2;
		return sizeof("inversed ");
	}
	*direction = 2;
	return 0;
}

struct device_configuration *read_device_configuration(char *path) {
	FILE *src = fopen(path, "r");
	if (src == NULL) {
		perror(NULL);
		return NULL;
	}
	char buffer[256] = "";
	// first line of configuration file must contain the name of the device
	if (!fgets(buffer, sizeof(buffer), src)) return NULL;
	buffer[strlen(buffer) - 1] = 0;
	struct device_id id = scan_device_id(buffer);
	char **actions = new_array_of_type(char *);
	struct control *controls = new_array_of_type(struct control);
	struct binding *bindings = new_array_of_type(struct binding);
	while (fgets(buffer, sizeof(buffer), src)) {
		buffer[strlen(buffer) - 1] = 0;
		char *line = buffer;
		while (*line == ' ' || *line == '\t') line++;
		if (!line[0] || line[0] == '#') continue;
		struct control cntrl;
		line += scan_control(line, &cntrl);
		while (*line == ' ' || *line == '\t') line++;
		if (*line != '-') return NULL;
		while (*line == ' ' || *line == '\t') line++;
		int8_t direction;
		line += scan_direction(line, &direction);
		char *action = malloc(strlen(line) + 1);
		strcpy(action, line);
		struct binding binding = {len(controls), len(actions), direction};
		controls = push(controls, &cntrl);
		actions = push(actions, &action);
		bindings = push(bindings, &binding);
	}
	fclose(src);
	struct device_configuration *device_ptr =
		malloc(sizeof(struct device_configuration));
	struct device_configuration device = {id, actions, controls, bindings};
	*device_ptr = device;
	return device_ptr;
}

#endif
