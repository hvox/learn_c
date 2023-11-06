#include <errno.h>
#include <fcntl.h>
#include <linux/uinput.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "controls.h"

char *USAGE_MESSAGE =
  "usage: %s <periphery event file>\n\n"
  "examples:\n  %s /dev/input/by-id/usb-Logitech_USB_Keyboard-event-kbd\n";


const int FUNCTION_KEYS[] = {
	KEY_F1, KEY_F2, KEY_F3, KEY_F4, KEY_F5, KEY_F6, KEY_F7, KEY_F8, KEY_F9, KEY_F10, KEY_F11, KEY_F12, KEY_F13,
	KEY_F14, KEY_F15, KEY_F16, KEY_F17, KEY_F18, KEY_F19, KEY_F20, KEY_F21, KEY_F22, KEY_F23, KEY_F24
};



int connect_to_periphery(char *periphery_path) {
	int periphery = open(periphery_path, O_RDONLY);
	if (periphery == -1) return -1;
	struct control controls[256] = {};
	if (ioctl(periphery, EVIOCGRAB, 1) < 0)
		printf("Failed to block other programs from reading periphery events");
	return periphery;
}

int key_name_to_key_code(const char *key_name) {
	for (int key = 0; key < KEY_NAMES_CNT; key++)
		if (KEY_NAMES[key] != NULL && strcmp(key_name, KEY_NAMES[key]) == 0)
			return key;
	fprintf(stderr, "ERROR: Key code for \"%s\" is not found\n", key_name);
	return -1;
}


int create_device() {
	int fd = open("/dev/uinput", O_WRONLY | O_NONBLOCK);
	if (fd < 0) {
		perror("/dev/uinput");
		return -1;
	}
	ioctl(fd, UI_SET_EVBIT, EV_KEY);
	for (int i = 0; i <= sizeof(FUNCTION_KEYS) / sizeof(FUNCTION_KEYS[0]); i++)
		ioctl(fd, UI_SET_KEYBIT, FUNCTION_KEYS[i]);
	struct uinput_setup usetup = {.id={.bustype=BUS_USB, .vendor=0x258A, .product=0x0027}, .name="Fake"};
	if (ioctl(fd, UI_DEV_SETUP, &usetup) < 0) return -1;
	if (ioctl(fd, UI_DEV_CREATE) < 0) return -3;
	return fd;
}

void process_event(int fd, struct input_event event) {
	if (event.type == EV_KEY) {
		if (event.code > 700 || event.value == 2) return;
		int code = FUNCTION_KEYS[7 + event.code - KEY_1];
		struct input_event ev = {.type=EV_KEY, .code=code, .value=event.value};
		struct input_event flush = {.type=EV_SYN, .code=SYN_REPORT, .value=0};
		if (write(fd, &ev, sizeof(ev)) < 0 || (write(fd, &flush, sizeof(flush)) < 0))
			perror("?");
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
	if (argc != 2 || argv[1][0] != '/') {
		printf(USAGE_MESSAGE, argv[0], argv[0]);
		return 42;
	}
	int periphery_fd = connect_to_periphery(argv[1]);
	if (periphery_fd < 0) {
		perror("Failed to connect to the periphery");
		if (errno == 13)
			printf("Have you tried something like this:\n"
			       "  sudo %s %s\n", argv[0], argv[1]);
		return 1;
	}
	int fake_mouse_device = create_device();
	if (fake_mouse_device < 0) {
		perror("Failed to create fake mouse.\n");
		close(periphery_fd);
		return 1;
	}
	printf("Press DELETE to temporary stop.\n");
	int enabled = 1;
	struct input_event event;
	while (read(periphery_fd, &event, sizeof(event)) != -1) {
		if (event.type == EV_SYN) continue;
		if (event.code == KEY_DELETE && event.value == 0)
			ioctl(periphery_fd, EVIOCGRAB, enabled = !enabled);
		process_event(fake_mouse_device, event);
	}
	close(periphery_fd);
	return 0;
}
