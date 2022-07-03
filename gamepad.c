#include <fcntl.h>
#include <linux/uinput.h>
#include <stdio.h>
#include <unistd.h>

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

int main(int argc, char *argv[]) {
	int gamepad_fd = create_gamepad();
	if (gamepad_fd < 0) {
		printf("Failed to create gamepad\n");
		return 42;
	}

	struct input_event event = {.type=EV_KEY, .code=BTN_A};
	struct input_event sync_event = {.type=EV_SYN};
	printf("press ENTER on a keyboard to press/release A on the gamepad\n");
	while (1) {
		char buffer[81] = "";

		if (!fgets(buffer, 80, stdin)) break;
		event.value = 1;
		write(gamepad_fd, &event, sizeof(event));
		write(gamepad_fd, &sync_event, sizeof(sync_event));

		if (!fgets(buffer, 80, stdin)) break;
		event.value = 0;
		write(gamepad_fd, &event, sizeof(event));
		write(gamepad_fd, &sync_event, sizeof(sync_event));
	}
	close(gamepad_fd);
	return 0;
}
