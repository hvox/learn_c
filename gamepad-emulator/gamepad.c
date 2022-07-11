#include <errno.h>
#include <fcntl.h>
#include <linux/uinput.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "controls.h"
#include "dynamic_array.h"

char *USAGE_MESSAGE =
  "usage: %s <keyboard event file>\n\n"
  "examples:\n  %s /dev/input/by-id/usb-Logitech_USB_Keyboard-event-kbd\n";

enum gamepad_controls {
	// 6 directional controls
	LEFT_STICK_X, LEFT_STICK_Y, LEFT_TRIGGER,
	RIGHT_STICK_X, RIGHT_STICK_Y, RIGHT_TRIGGER,
	DIRECTIONAL_PAD_X, DIRECTIONAL_PAD_Y,
	// 13 pushable controls
	BUTTON_A, BUTTON_B, BUTTON_X, BUTTON_Y,
	LEFT_BUMPER, RIGHT_BUMPER,
	BUTTON_START, BUTTON_BACK, BUTTON_X360,
	LEFT_STICK, RIGHT_STICK
};
short GAMEPAD_CONTROL_CODES[19] = {
	ABS_X, ABS_Y, ABS_Z, ABS_RX, ABS_RY, ABS_RZ, ABS_HAT0X, ABS_HAT0Y,
	BTN_A, BTN_B, BTN_X, BTN_Y, BTN_TL, BTN_TR,
	BTN_SELECT, BTN_START, BTN_MODE, BTN_THUMBL, BTN_THUMBR
};

struct action {
	char *name;
	int control;
	int direction;
};
struct action ACTIONS[26] = {
	{"do absolutely nothing"},
	{"joystick into keyboard"},
	{"left stick", LEFT_STICK, 1},
	{"left stick up", LEFT_STICK_Y, -128},
	{"left stick left", LEFT_STICK_X, -128},
	{"left stick down", LEFT_STICK_Y, 127},
	{"left stick right", LEFT_STICK_X, 127},
	{"left trigger", LEFT_TRIGGER, 127}, {"left bumper", LEFT_BUMPER, 1},
	{"right stick", RIGHT_STICK, 1},
	{"right stick up", RIGHT_STICK_Y, -128},
	{"right stick left", RIGHT_STICK_X, -128},
	{"right stick down", RIGHT_STICK_Y, 127},
	{"right stick right", RIGHT_STICK_X, 127},
	{"right trigger", RIGHT_TRIGGER, 127}, {"right bumper", RIGHT_BUMPER, 1},
	{"d-pad up", DIRECTIONAL_PAD_Y, -128},
	{"d-pad left", DIRECTIONAL_PAD_X, -128},
	{"d-pad down", DIRECTIONAL_PAD_Y, 127},
	{"d-pad right", DIRECTIONAL_PAD_X, 127},
	{"back button", BUTTON_BACK, 1}, {"start button", BUTTON_START, 1},
	{"x", BUTTON_X, 1}, {"y", BUTTON_Y, 1},
	{"a", BUTTON_A, 1}, {"b", BUTTON_B, 1},
};


char *CONJUNCTIONS[] = {
  "to", "press", "move", "punch", "rotate", "turn", "the", "button", NULL
};
char *KEY_PREPOSITIONS[] = {
  "key", "button", "the", NULL
};

int scan_word(FILE *source, char *word) {
	if (fscanf(source, "%64s", word) != 1) return 0;
	for (char *ch = word; *ch != 0; ch++)
		*ch += (*ch > 64 && *ch < 91) ? 32 : 0;
	if (word[0] != '#') return 1;
	while (fscanf(source, "%64[^\n]s", word) == 1);
	return scan_word(source, word);
}

int find(char **haystack, char *needle, int haystack_size) {
	int i = 0;
	while (i != haystack_size && strcmp(haystack[i], needle)) i++;
	return i != haystack_size ? i : -1;
}

int skip_words(FILE *source, char *word, char **skips, int skips_cnt) {
	if (skips_cnt == -1) {
		skips_cnt = 0;
		while (skips[skips_cnt] != NULL)
			skips_cnt++;
	}
	do {
		if (!scan_word(source, word)) return 0;
	} while (find(skips, word, skips_cnt) != -1);
	return 1;
}

int read_keys(char map[128], char *config_path) {
	FILE *config = fopen(config_path, "r");
	if (config == NULL) {
		perror("Failed to read keybindings from config file");
		return -1;
	}
	char word[65] = "";
	for (int done = !scan_word(config, word); !done;) {
		if (strcmp(word, "map")) {
			fprintf(stderr, "Expected \"map\" but got \"%s\"\n", word);
			fclose(config); return -2;
		}
		if (!skip_words(config, word, KEY_PREPOSITIONS, -1)) {
			fprintf(stderr, "You haven't specified what key to map\n");
			fclose(config); return -3;
		}
		int key = find(KEY_NAMES, word, KEY_NAMES_CNT);
		if (key == -1) {
			fprintf(stderr, "Invalid key name: %s\n", word);
			fclose(config); return -4;
		}
		if (!skip_words(config, word, CONJUNCTIONS, -1)) {
			fprintf(stderr, "You haven't specified action to be performed\n");
			fclose(config); return -5;
		}
		char action_name[128] = "";
		while (strlen(action_name) < 48 && strcmp(word, "map") && !done) {
			strcat(action_name, word);
			strcat(action_name, " ");
			done = scan_word(config, word) != 1;
		}
		action_name[strlen(action_name) - 1] = 0;
		int act = 25;
		while (act >= 0 && strcmp(ACTIONS[act].name, action_name)) act--;
		if (act == -1) {
			fprintf(stderr, "\"%s\" is not a valid action\n", action_name);
			fclose(config); return -6;
		}
		if (map[key]) {
			fprintf(stderr, "Key \"%s\" is already mapped to \"%s\"\n",
				KEY_NAMES[key], ACTIONS[map[key]].name);
			fclose(config); return -7;
		}
		map[key] = act;
	}
	fclose(config);
	return 0;
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

void sync_control(int fd, int control, int value) {
	int type, code = GAMEPAD_CONTROL_CODES[control];
	if (code > 128) {
		type = EV_KEY;
		value = value > 0 ? 1 : 0;
	} else  {
		type = EV_ABS;
		value = value > 127 ? 127 : value < -128 ? -128 : value;
	}
	struct input_event event = {.type=type, .code=code, .value=value};
	struct input_event flush = {.type=EV_SYN};
	if (write(fd, &event, sizeof(event)) < 0
		  || (write(fd, &flush, sizeof(flush)) < 0))
		printf("Failed to send event %d:%d\n", event.code, event.value);
}

int connect_to_keyboard(char *keyboard_path) {
	int keyboard = open(keyboard_path, O_RDONLY);
	if (keyboard == -1) return -1;
	if (ioctl(keyboard, EVIOCGRAB, 1) < 0)
		printf("Failed to block other programs from reading keyboard events");
	return keyboard;
}

struct control *get_supported_controls(int periphery) {
	struct control *controls = new_array_of_type(struct control);

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
			controls = push(controls, &cntrl);
		}
	unsigned char supported_buttons[(KEY_MAX + 7) / 8] = {};
	ioctl(periphery, EVIOCGBIT(EV_KEY, KEY_MAX), supported_buttons);
	for (int key = 0; key < 701; key++) {
		if ((supported_buttons[key / 8] >> (key % 8)) & 1) {
			struct control cntrl = {
				.code = 61 + key, .value = 0,
				.min_value = 0, .max_value = 1
			};
			controls = push(controls, &cntrl);
		}
	}
	return controls;
}

int main(int argc, char *argv[]) {
	struct control gamepad_controls[19] = {
		new_axis_control(ABS_X, 0, -128, 127), // left stick X
		new_axis_control(ABS_Y, 0, -128, 127), // left stick Y
		new_axis_control(ABS_Z, 0, 0, 127), // left trigger
		new_axis_control(ABS_RX, 0, -128, 127), // right stick X
		new_axis_control(ABS_RY, 0, -128, 127), // right stick Y
		new_axis_control(ABS_RZ, 0, 0, 127), // right trigger
		new_axis_control(ABS_HAT0X, 0, -128, 127), // directional pad X
		new_axis_control(ABS_HAT0Y, 0, -128, 127), // directional pad Y
		new_key_control(BTN_A), // button A
		new_key_control(BTN_B), // button B
		new_key_control(BTN_X), // button X
		new_key_control(BTN_Y), // button Y
		new_key_control(BTN_TL), // left bumper
		new_key_control(BTN_TR), // right bumper
		new_key_control(BTN_SELECT), // start button
		new_key_control(BTN_START), // back button
		new_key_control(BTN_MODE), // X360 button
		new_key_control(BTN_THUMBL), // left stick
		new_key_control(BTN_THUMBR), // right stick
	};

	if (argc != 2 || argv[1][0] != '/') {
		printf(USAGE_MESSAGE, argv[0], argv[0]);
		return 42;
	}
	char key_bindings[128] = "";
	if (read_keys(key_bindings, "./keys.keys") < 0) return 42;
	int keyboard_fd = connect_to_keyboard(argv[1]);
	struct control *controls = get_supported_controls(keyboard_fd);
	if (keyboard_fd < 0) {
		perror("Failed to connect to the keyboard");
		if (errno == 13)
			printf("Have you tried something like this:\n"
			       "  sudo %s %s\n", argv[0], argv[1]);
		return 42;
	}
	int gamepad_fd = create_gamepad();
	if (gamepad_fd < 0) {
		perror("Failed to create gamepad");
		return 42;
	}
	printf("The keyboard has been successfully turned into a joystick.\n"
	       "If you want to temporary turn it back, press DELETE on it.\n");
	struct input_event event;
	int balanse = 0, enabled = 1;
	while (read(keyboard_fd, &event, sizeof(event)) != -1) {
		if (event.type != EV_KEY) continue;
		if (event.code > 127 || event.value == 2) continue;
		if (event.code == KEY_DELETE && event.value == 0)
			ioctl(keyboard_fd, EVIOCGRAB, enabled = !enabled);
		balanse += event.value ? 1 : -1;
		printf("%d ", balanse);
		printf("pressure=%d key=%s\n", event.value, KEY_NAMES[event.code]);
		if (key_bindings[event.code] == 0) continue;
		if (key_bindings[event.code] == 1) break;
		struct action action = ACTIONS[key_bindings[event.code]];
		printf("performed action: %s\n", action.name);
		gamepad_controls[action.control].value
			+= action.direction * (event.value * 2 - 1);
		if (enabled)
			sync_control(gamepad_fd, action.control,
					gamepad_controls[action.control].value);
	}
	del_array(controls);
	close(keyboard_fd);
	close(gamepad_fd);
	return 0;
}
