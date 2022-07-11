#include <errno.h>
#include <stdint.h>
#include <stdlib.h>
#include <fcntl.h>
#include <linux/uinput.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "controls.h"
#include "../dynamic_array.h"

char *USAGE_MESSAGE =
  "usage: %s <periphery event file>\n\n"
  "examples:\n  %s /dev/input/by-id/usb-Logitech_USB_Keyboard-event-kbd\n";

void *object(void *content, uint16_t size) {
	void *object = malloc(size + 2);
	*((uint16_t *) object) = size;
	return memcpy(object + 2, content, size);
}

void delete(void *object) {
	free(object - 2);
}

uint16_t object_size(void *object) {
	return *((uint16_t *) (object - 2));
}

void print_control_description(struct control cntrl) {
	int code = cntrl.code;
	if (code > 61)
		printf("%s\n", KEY_NAMES[code - 61]);
	else
		printf("%s [%d, %d, %d]\n", AXES_NAMES[code],
			cntrl.value, cntrl.min_value, cntrl.max_value);
}

struct control *get_supported_controls(int periphery) {
	int controls_count = 0;
	struct control controls[256] = {};

	unsigned char supported_buttons[(KEY_MAX + 7) / 8] = {};
	ioctl(periphery, EVIOCGBIT(EV_KEY, KEY_MAX), supported_buttons);
	for (int key = 0; key < 701; key++) {
		if ((supported_buttons[key / 8] >> (key % 8)) & 1) {
			struct control cntrl = {
				.code = 61 + key, .value = 0,
				.min_value = 0, .max_value = 1
			};
			controls[controls_count++] = cntrl;
		}
	}
	unsigned char supported_axes[(ABS_MAX + 7) / 8] = {};
	ioctl(periphery, EVIOCGBIT(EV_ABS, ABS_MAX), supported_axes);
	for (int axis = 0; axis < 61; axis++)
		if ((supported_axes[axis / 8] >> (axis % 8)) & 1) {
			struct input_absinfo axis_info;
			ioctl(periphery, EVIOCGABS(axis), &axis_info);
			struct control cntrl = {
				.code = axis, .value = axis_info.value,
				.min_value = axis_info.minimum, .max_value = axis_info.maximum
			};
			controls[controls_count++] = cntrl;
		}
	return object(controls, controls_count * sizeof(struct control));
}

int print_periphery(char *periphery_path) {
	int periphery = open(periphery_path, O_RDONLY);
	if (periphery == -1) return -1;
	char name[UINPUT_MAX_NAME_SIZE] = "ANONIMIOUS DEVICE";
	if (ioctl(periphery, EVIOCGNAME(UINPUT_MAX_NAME_SIZE), name) < 0)
		perror(periphery_path);
	uint16_t id[4];
	if (ioctl(periphery, EVIOCGID, id) < 0)
		perror(periphery_path);
	printf(":%04X:%04X %s\n", id[1], id[2], name);
	struct control *controls = get_supported_controls(periphery);
	int controls_count = object_size(controls) / sizeof(struct control);
	for (int i = 0; i < controls_count; i++)
		print_control_description(controls[i]);
	//del_array(controls);
	close(periphery);
	return 0;
}

int main(int argc, char *argv[]) {
	if (argc != 2 || argv[1][0] != '/') {
		printf(USAGE_MESSAGE, argv[0], argv[0]);
		return 42;
	}
	return print_periphery(argv[1]);
	return 0;
}
