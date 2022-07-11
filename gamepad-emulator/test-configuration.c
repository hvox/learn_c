#include <dirent.h>
#include <fcntl.h>
#include <linux/uinput.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "parser.h"

char *USAGE_MESSAGE =
  "usage: %s <periphery configuration file>\n";

int is_infix(const char *str, const char *infix) {
	if (str == NULL || infix == NULL) return 0;
	for (int i = 0; str[i]; i++)
		if (!strncmp(str + i, infix, strlen(infix)))
			return strlen(infix) - strlen(str) + 0x7fffffff;
	return 0;
}

int connect_to_device(const char *target_device_name) {
	int is_event(const struct dirent *input) {
		return is_prefix(input->d_name, "event");
	}
	int comparator(const struct dirent **inp1, const struct dirent **inp2) {
		int index1, index2;
		sscanf((*inp1)->d_name, "event%d", &index1);
		sscanf((*inp2)->d_name, "event%d", &index2);
		return index1 - index2;
	}

	struct dirent **inputs;
	int count = scandir("/dev/input", &inputs, is_event, comparator);
	if (count <= 0) {
		perror("Failed to scan /dev/input");
		return -1;
	}

	int max_likeness = 0;
	int best_candidate = -1;
	for (int i = 0; i < count; i++) {
		char path[20] = "/dev/input/";
		strcat(path, inputs[i]->d_name);
		free(inputs[i]);
		int fd = open(path, O_RDONLY);
		if (fd < 0) {
			perror(path);
			continue;
		}
		char name[UINPUT_MAX_NAME_SIZE] = "ANONYMOUS DEVICE";
		if (ioctl(fd, EVIOCGNAME(UINPUT_MAX_NAME_SIZE), name) < 0)
			perror(path);
		close(fd);
		int likeness = is_infix(name, target_device_name);
		if (likeness > max_likeness) {
			max_likeness = likeness;
			sscanf(inputs[i]->d_name, "event%d", &best_candidate);
		}
	}
	if (best_candidate == -1) {
		fprintf(stderr, "Failed to find \"%s\"\n", target_device_name);
		return -1;
	}
	char path[20];
	sprintf(path, "/dev/input/event%d", best_candidate);
	int device_fd = open(path, O_RDONLY);
	if (device_fd < 0) {
		perror(path);
		return -2;
	}
	return device_fd;
}

void print_event(struct input_event event) {
	if (event.type == EV_KEY) {
		if (event.code > 700 || event.value == 2) return;
		printf("key %s was %s\n", KEY_NAMES[event.code],
			event.value ? "pressed" : "released");
	} else if (event.type == EV_ABS) {
		if (event.code > 61) return;
		printf("axis %s has value %d\n", AXES_NAMES[event.code], event.value);
	} else if (event.type == EV_MSC && event.code == MSC_SCAN) {
		printf("physical object #%d changed its state\n", event.value);
	} else {
		printf("event %d:%d:%d occurred\n", event.type, event.code, event.value);
	}
}


int main(int argc, char *argv[]) {
	if (argc != 2) {
		printf(USAGE_MESSAGE, argv[0], argv[0]);
		return 41;
	}
	struct device_configuration *device = read_device_configuration(argv[1]);
	if (device == NULL) {
		fprintf(stderr, "Failed to read device configuration\n");
		return 42;
	}
	int device_fd = connect_to_device(device->id.name);
	if (device_fd < 0) {
		fprintf(stderr, "Failed to connect to \"%s\"\n", device->id.name);
		return 43;
	}
	char name[UINPUT_MAX_NAME_SIZE] = "ANONYMOUS DEVICE";
	ioctl(device_fd, EVIOCGNAME(UINPUT_MAX_NAME_SIZE), name);
	printf("Successfully connected to \"%s\"\n", name);
	struct input_event event;
	while (read(device_fd, &event, sizeof(event)) != -1) {
		if (event.type != EV_KEY && event.type != EV_ABS) continue;
		if (event.type == EV_KEY && event.value == 2) continue;
		int i = 0;
		struct control *controls = device->controls;
		if (event.type == EV_KEY)
			i = controls_find_key(controls, len(controls), event.code);
		else
			i = controls_find_axis(controls, len(controls), event.code);
		print_event(event);
		if (i < 0) continue;
		controls[i].value = event.value;
		struct binding *bindings = device->bindings;
		i = bindings_find_control(bindings, len(bindings), i);
		if (i < 0) continue;
		printf("action: %s\n", device->actions[i]);
	}
	close(device_fd);
	return 0;
}
