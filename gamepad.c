#include <errno.h>
#include <fcntl.h>
#include <linux/uinput.h>
#include <stdio.h>
#include <unistd.h>

char *USAGE_MESSAGE = "usage: %s <keyboard event file>\n\n"
 "examples:\n  %s /dev/input/by-id/usb-Logitech_USB_Keyboard-event-kbd\n";

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
	set_axis_parameters(&uidev, ABS_X, ABS_Y, -32768, 32767, 16, 128);
	set_axis_parameters(&uidev, ABS_RX, ABS_RY, -32768, 32767, 16, 128);
	set_axis_parameters(&uidev, ABS_HAT0X, ABS_HAT0Y, -1, 1, 0, 0);
	set_axis_parameters(&uidev, ABS_Z, ABS_RZ, 0, 255, 0, 16);

	int gamepad = open("/dev/uinput", O_WRONLY | O_NONBLOCK);
	if (gamepad < 0) return -1;
	ioctl(gamepad, UI_SET_EVBIT, EV_KEY);
	ioctl(gamepad, UI_SET_EVBIT, EV_SYN);
	ioctl(gamepad, UI_SET_EVBIT, EV_ABS);
	for (int i = 0; i < btns_sz; i++) ioctl(gamepad, UI_SET_KEYBIT, btns[i]);
	for (int i = 0; i < axes_sz; i++) ioctl(gamepad, UI_SET_ABSBIT, axes[i]);
	if (write(gamepad, &uidev, sizeof(uidev)) < 0) return -2;
	if (ioctl(gamepad, UI_DEV_CREATE) < 0) return -3;
	return gamepad;
}

void send_event(int fd, int TYPE, int CODE, int VALUE) {
	struct input_event event = {.type=TYPE, .code=CODE, .value=VALUE};
	if (write(fd, &event, sizeof(struct input_event)) < 0)
		printf("Failed to send event %d:%d\n", event.code, event.value);
}

void flush_events(int fd) {
	struct input_event event = {.type=EV_SYN};
	if (write(fd, &event, sizeof(struct input_event)) < 0)
		printf("Failed to flush events\n");
}

int connect_to_keyboard(char *keyboard_path) {
	int keyboard = open(keyboard_path, O_RDONLY);
	if (keyboard == -1) return -1;
	if (ioctl(keyboard, EVIOCGRAB, 1) < 0)
		printf("Failed to block other programs from reading keyboard events");
	return keyboard;
}

int main(int argc, char *argv[]) {
	if (argc != 2 || argv[1][0] != '/') {
		printf(USAGE_MESSAGE, argv[0], argv[0]);
		return 42;
	}
	int keyboard_fd = connect_to_keyboard(argv[1]);
	if (keyboard_fd < 0) {
		perror("Failed to connect to the keyboard");
		if (errno == 13)
			printf("Have you tried something like this:\n"
			       "  sudo %s %s\n", argv[0], argv[1]);
	}
	int gamepad_fd = create_gamepad();
	if (gamepad_fd < 0) printf("Failed to create gamepad\n");
	if (keyboard_fd < 0 || gamepad_fd < 0) return 42;
	printf("The keyboard has been successfully turned into a joystick.\n"
	       "If you want to turn it back, press DELETE on it.\n");

	struct input_event event;
	while (read(keyboard_fd, &event, sizeof(event)) != -1) {
		//   press the DELETE key to turn off xbox 360 controller emulation   //
		if (event.type != EV_KEY) continue;
		if (event.code == KEY_DELETE && event.value == 1) break;
		// left joystick
		if (event.code == KEY_E) send_event(gamepad_fd, EV_KEY, BTN_THUMBL, event.value);
		if (event.code == KEY_W) send_event(gamepad_fd, EV_ABS, ABS_Y, event.value ? -32768 : 0);
		if (event.code == KEY_A) send_event(gamepad_fd, EV_ABS, ABS_X, event.value ? -32768 : 0);
		if (event.code == KEY_S) send_event(gamepad_fd, EV_ABS, ABS_Y, event.value ? 32767 : 0);
		if (event.code == KEY_D) send_event(gamepad_fd, EV_ABS, ABS_X, event.value ? 32767 : 0);
		// right joystick
		if (event.code == KEY_U) send_event(gamepad_fd, EV_KEY, BTN_THUMBR, event.value);
		if (event.code == KEY_I) send_event(gamepad_fd, EV_ABS, ABS_RY, event.value ? -32768 : 0);
		if (event.code == KEY_J) send_event(gamepad_fd, EV_ABS, ABS_RX, event.value ? -32768 : 0);
		if (event.code == KEY_K) send_event(gamepad_fd, EV_ABS, ABS_RY, event.value ? 32767 : 0);
		if (event.code == KEY_L) send_event(gamepad_fd, EV_ABS, ABS_RX, event.value ? 32767 : 0);
		// XYAB
		if (event.code == KEY_ENTER && event.value != 2) send_event(gamepad_fd, EV_KEY, BTN_A, event.value);
		if (event.code == KEY_SPACE && event.value != 2) send_event(gamepad_fd, EV_KEY, BTN_A, event.value);
		if (event.code == KEY_LEFTSHIFT && event.value != 2) send_event(gamepad_fd, EV_KEY, BTN_B, event.value);
		if (event.code == KEY_SEMICOLON && event.value != 2) send_event(gamepad_fd, EV_KEY, BTN_X, event.value);
		if (event.code == KEY_APOSTROPHE && event.value != 2) send_event(gamepad_fd, EV_KEY, BTN_Y, event.value);
		// Bumpers and triggers
		if (event.code == KEY_Q && event.value != 2) send_event(gamepad_fd, EV_KEY, BTN_TL, event.value);
		if (event.code == KEY_CAPSLOCK && event.value != 2) send_event(gamepad_fd, EV_ABS, ABS_Z, event.value*255);
		if (event.code == KEY_O && event.value != 2) send_event(gamepad_fd, EV_KEY, BTN_TR, event.value);
		if (event.code == KEY_P && event.value != 2) send_event(gamepad_fd, EV_ABS, ABS_RZ, event.value*255);
		// Buttons with terrible names
		if (event.code == KEY_TAB && event.value != 2) send_event(gamepad_fd, EV_KEY, BTN_SELECT, event.value);
		if (event.code == KEY_ESC && event.value != 2) send_event(gamepad_fd, EV_KEY, BTN_START, event.value);
		// D-pad
		if (event.code == KEY_UP && event.value != 2) send_event(gamepad_fd, EV_ABS, ABS_HAT0Y, -event.value);
		if (event.code == KEY_LEFT && event.value != 2) send_event(gamepad_fd, EV_ABS, ABS_HAT0X, -event.value);
		if (event.code == KEY_DOWN && event.value != 2) send_event(gamepad_fd, EV_ABS, ABS_HAT0Y, event.value);
		if (event.code == KEY_RIGHT && event.value != 2) send_event(gamepad_fd, EV_ABS, ABS_HAT0X, event.value);
		flush_events(gamepad_fd);
	}
	close(keyboard_fd);
	close(gamepad_fd);
	return 0;
}
