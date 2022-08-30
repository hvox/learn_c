#include <dirent.h>
#include <fcntl.h>
#include <linux/uinput.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "parser.h"

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

int create_device(const struct device_configuration *device) {
	int fd = open("/dev/uinput", O_WRONLY | O_NONBLOCK);
	if (fd < 0) {
		perror("/dev/uinput");
		return -1;
	}
	ioctl(fd, UI_SET_EVBIT, EV_KEY);
	ioctl(fd, UI_SET_EVBIT, EV_ABS);
	ioctl(fd, UI_SET_EVBIT, EV_SYN);
	struct uinput_user_dev uidev = {.id={ .bustype=BUS_USB, .version=0x0110,
		.vendor=device->id.vendor_id, .product=device->id.product_id } };
	strcpy(uidev.name, device->id.name);
	for (int i = 0; i < len(device->controls); i++) {
		int key = control_as_key(device->controls[i]);
		if (key >= 0) {
			ioctl(fd, UI_SET_KEYBIT, key);
			continue;
		}
		int axis = control_as_axis(device->controls[i]);
		ioctl(fd, UI_SET_ABSBIT, axis);
		uidev.absmin[axis] = device->controls[i].min_value;
		uidev.absmax[axis] = device->controls[i].max_value;
		uidev.absfuzz[axis] = 0;
		uidev.absflat[axis] = 0;
	}
	if (write(fd, &uidev, sizeof(uidev)) < 0) return -2;
	if (ioctl(fd, UI_DEV_CREATE) < 0) return -3;
	return fd;
}

void set_axis_parameters(struct uinput_user_dev *uidev,
  int x_axis, int y_axis, int min, int max, int fuzz, int flat) {
	int axes[2] = {x_axis, y_axis};
	for (int i = 0; i < 2; i++) {
		uidev->absmin[axes[i]] = min;
		uidev->absmax[axes[i]] = max;
		uidev->absfuzz[axes[i]] = fuzz;
		uidev->absflat[axes[i]] = flat;
	}
}

int create_gamepad() {
	int btns[] = { BTN_A, BTN_B, BTN_X, BTN_Y, BTN_TL, BTN_TR,
		BTN_SELECT, BTN_START, BTN_MODE, BTN_THUMBL, BTN_THUMBR };
	int btns_sz = sizeof(btns) / sizeof(int);
	int axes[] = { ABS_X, ABS_Y, ABS_Z,
		ABS_RX, ABS_RY, ABS_RZ, ABS_HAT0X, ABS_HAT0Y };
	int axes_sz = sizeof(axes) / sizeof(int);
	struct uinput_user_dev uidev = { .name="Xbox 360 Controller", .id={
		.bustype=BUS_USB, .version=0x0110, .vendor=0x045e, .product=0x028e}};
	set_axis_parameters(&uidev, ABS_X, ABS_Y, -128, 127, 0, 16);
	set_axis_parameters(&uidev, ABS_RX, ABS_RY, -128, 127, 0, 16);
	set_axis_parameters(&uidev, ABS_HAT0X, ABS_HAT0Y, -128, 127, 0, 16);
	set_axis_parameters(&uidev, ABS_Z, ABS_RZ, 0, 127, 0, 16);

	int gamepad = open("/dev/uinput", O_WRONLY | O_NONBLOCK);
	if (gamepad < 0) return -1;
	ioctl(gamepad, UI_SET_EVBIT, EV_ABS);
	ioctl(gamepad, UI_SET_EVBIT, EV_KEY);
	ioctl(gamepad, UI_SET_EVBIT, EV_SYN);
	for (int i = 0; i < btns_sz; i++) ioctl(gamepad, UI_SET_KEYBIT, btns[i]);
	for (int i = 0; i < axes_sz; i++) ioctl(gamepad, UI_SET_ABSBIT, axes[i]);
	if (write(gamepad, &uidev, sizeof(uidev)) < 0) return -2;
	if (ioctl(gamepad, UI_DEV_CREATE) < 0) return -3;
	return gamepad;
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

void sync_control(int fd, const struct control *control) {
	int key = control_as_key(*control);
	int type, code;
	if (key >= 0) {
		type = EV_KEY;
		code = key;
	} else {
		int axis = control_as_axis(*control);
		type = EV_ABS;
		code = axis;
	}
	int value = control->value;
	value = value > control->max_value ? control->max_value : value;
	value = value < control->min_value ? control->min_value : value;
	struct input_event event = {.type=type, .code=code, .value=value};
	struct input_event flush = {.type=EV_SYN};
	printf("---- EVENT %d:%d:%d\n", event.type, event.code, event.value);
	if (write(fd, &event, sizeof(event)) < 0
		  || (write(fd, &flush, sizeof(flush)) < 0))
		printf("Failed to send event %d:%d\n", event.code, event.value);
}

int pipe_devices(struct device_configuration *src,
                 struct device_configuration *dest) {
	//int dest_fd = create_gamepad();
	int dest_fd = create_device(dest);
	if (dest_fd < 0) {
		fprintf(stderr, "Failed to create \"%s\"\n", dest->id.name);
		return -2;
	}
	else {
		printf("Successfully created %04x:%04x %s\n",
			dest->id.vendor_id, dest->id.product_id, dest->id.name);
	}
	int src_fd;
	while ((src_fd = connect_to_device(src->id.name)) < 0) {
		fprintf(stderr, "Failed to connect to \"%s\"\n", src->id.name);
		fprintf(stderr, "I will try again in a few seconds\n");
		sleep(2);
	}
	char name[UINPUT_MAX_NAME_SIZE] = "ANONYMOUS DEVICE";
	ioctl(src_fd, EVIOCGNAME(UINPUT_MAX_NAME_SIZE), name);
	printf("Successfully connected to \"%s\"\n", name);
	if (ioctl(src_fd, EVIOCGRAB, 1) < 0)
		perror(NULL);

	int enabled = 1;
	struct input_event event;
	while (read(src_fd, &event, sizeof(event)) != -1) {
		if (event.type != EV_KEY && event.type != EV_ABS) continue;
		if (event.type == EV_KEY && event.value == 2) {
			if (event.code == KEY_DELETE) break;
			else continue;
		}
		if (event.type == EV_KEY && event.code == KEY_DELETE && event.value == 0)
			ioctl(src_fd, EVIOCGRAB, enabled = !enabled);
		struct control *controls = src->controls;
		int i = (event.type == EV_KEY)
			? controls_find_key(controls, len(controls), event.code)
			: controls_find_axis(controls, len(controls), event.code);
		print_event(event);
		int old_value = controls[i].value;
		if (i < 0 || old_value == event.value) continue;
		controls[i].value = event.value;
		struct binding *bindings = src->bindings;
		int j = bindings_find_control(bindings, len(bindings), i);
		if (j < 0) continue;
		char **actions = src->actions;
		char *action = actions[bindings[j].action];
		printf("action: %s\n", action);
		struct control *cntrl = find_control_by_action(dest, action);
		if (cntrl == NULL) {
			printf("no control defined for this action\n");
			continue;
		}
		int direction = bindings[j].direction;
		int src_range = controls[i].max_value - controls[i].min_value + 1;
		int dst_range = cntrl->max_value - cntrl->min_value + 1;
		old_value = old_value * dst_range * direction / src_range;
		int new_value = event.value * dst_range * direction / src_range;
		printf("old_value = %d\n", old_value);
		printf("new_value = %d\n", new_value);
		if (new_value == old_value) continue;
		// use delta since there might be other controls triggering this action
		cntrl->value += new_value - old_value;
		sync_control(dest_fd, cntrl);
	}
	close(src_fd);
	close(dest_fd);
	return 0;
}

int main(int argc, char *argv[]) {
	struct device_configuration *src = read_device_configuration(argv[1]);
	if (src == NULL) {
		fprintf(stderr, "Failed to read source device configuration\n");
		return -1;
	}
	struct device_configuration *dst = read_device_configuration(argv[2]);
	if (dst == NULL) {
		fprintf(stderr, "Failed to read target device configuration\n");
		return -2;
	}
	return pipe_devices(src, dst);
}
