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

int connect_to_periphery(char *periphery_path) {
	int periphery = open(periphery_path, O_RDONLY);
	if (periphery == -1) return -1;
	struct control controls[256] = {};
	if (ioctl(periphery, EVIOCGRAB, 1) < 0)
		printf("Failed to block other programs from reading periphery events");
	return periphery;
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
		return 42;
	}
	printf("Press DELETE to temporary stop.\n");
	int enabled = 1;
	struct input_event event;
	while (read(periphery_fd, &event, sizeof(event)) != -1) {
		if (event.type == EV_SYN) continue;
		if (event.code == KEY_DELETE && event.value == 0)
			ioctl(periphery_fd, EVIOCGRAB, enabled = !enabled);
		print_event(event);
	}
	close(periphery_fd);
	return 0;
}
