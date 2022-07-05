#include <errno.h>
#include <fcntl.h>
#include <linux/uinput.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

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
	{"left stick", LEFT_STICK},
	{"left stick up", LEFT_STICK_Y, -128},
	{"left stick left", LEFT_STICK_X, -128},
	{"left stick down", LEFT_STICK_Y, 127},
	{"left stick right", LEFT_STICK_X, 127},
	{"left trigger", LEFT_TRIGGER, 127}, {"left bumper", LEFT_BUMPER},
	{"right stick", RIGHT_STICK},
	{"right stick up", RIGHT_STICK_Y, -128},
	{"right stick left", RIGHT_STICK_X, -128},
	{"right stick down", RIGHT_STICK_Y, 127},
	{"right stick right", RIGHT_STICK_X, 127},
	{"right trigger", RIGHT_TRIGGER, 127}, {"right bumper", RIGHT_BUMPER},
	{"d-pad up", DIRECTIONAL_PAD_Y, -128},
	{"d-pad left", DIRECTIONAL_PAD_X, -128},
	{"d-pad down", DIRECTIONAL_PAD_Y, 127},
	{"d-pad right", DIRECTIONAL_PAD_X, 127},
	{"back button", BUTTON_BACK}, {"start button", BUTTON_START},
	{"x", BUTTON_X}, {"y", BUTTON_Y}, {"a", BUTTON_A}, {"b", BUTTON_B},
};

char *KEY_NAMES[129] = {
  "this_button_does_not_have_a_name_yet", "esc", "1", "2", "3", "4", "5", "6",
  "7", "8", "9", "0", "minus", "equal", "backspace", "tab", "q", "w", "e", "r",
  "t", "y", "u", "i", "o", "p", "leftbrace", "rightbrace", "enter", "leftctrl",
  "a", "s", "d", "f", "g", "h", "j", "k", "l", "semicolon", "apostrophe",
  "grave", "leftshift", "backslash", "z", "x", "c", "v", "b", "n", "m", "comma",
  "dot", "slash", "rightshift", "kpasterisk", "leftalt", "space", "capslock",
  "f1", "f2", "f3", "f4", "f5", "f6", "f7", "f8", "f9", "f10", "numlock",
  "scrolllock", "kp7", "kp8", "kp9", "kpminus", "kp4", "kp5", "kp6", "kpplus",
  "kp1", "kp2", "kp3", "kp0", "kpdot", "qfuy", "zenkakuhankaku", "102nd", "f11",
  "f12", "ro", "katakana", "hiragana", "henkan", "katakanahiragana", "muhenkan",
  "kpjpcomma", "kpenter", "rightctrl", "kpslash", "sysrq", "rightalt",
  "linefeed", "home", "up", "pageup", "left", "right", "end", "down",
  "pagedown", "insert", "delete", "macro", "mute", "volumedown", "volumeup",
  "power", "kpequal", "kpplusminus", "pause", "scale", "kpcomma", "hanguel",
  "hanja", "yen", "leftmeta", "rightmeta", "compose", NULL
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

int find(char **haystack, char *needle) {
	int i = 0;
	while (haystack[i] != NULL && strcmp(haystack[i], needle)) i++;
	return haystack[i] != NULL ? i : -1;
}

int skip_words(FILE *source, char *word, char **skips) {
	do {
		if (!scan_word(source, word)) return 0;
	} while (find(skips, word) != -1);
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
		if (!skip_words(config, word, KEY_PREPOSITIONS)) {
			fprintf(stderr, "You haven't specified what key to map\n");
			fclose(config); return -3;
		}
		int key = find(KEY_NAMES, word);
		if (key == -1) {
			fprintf(stderr, "Invalid key name: %s\n", word);
			fclose(config); return -4;
		}
		if (!skip_words(config, word, CONJUNCTIONS)) {
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

int main(int argc, char *argv[]) {
	if (argc != 2 || argv[1][0] != '/') {
		printf(USAGE_MESSAGE, argv[0], argv[0]);
		return 42;
	}
	char key_bindings[128] = "";
	if (read_keys(key_bindings, "./keys.keys") < 0) return 42;
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
	int balanse = 0;
	while (read(keyboard_fd, &event, sizeof(event)) != -1) {
		if (event.type != EV_KEY) continue;
		if (event.code > 127 || event.value == 2) continue;
		if (event.code == KEY_DELETE && event.value == 1) break;
		balanse += event.value ? 1 : -1;
		printf("balance: %d ", balanse);
		printf("pressure=%d key=%s\n", event.value, KEY_NAMES[event.code]);
		if (key_bindings[event.code] == 0) continue;
		if (key_bindings[event.code] == 1) break;
		struct action action = ACTIONS[key_bindings[event.code]];
		printf("performed action: %s\n", action.name);
		int control = GAMEPAD_CONTROL_CODES[action.control];
		int direction = action.direction ? action.direction : 1;
		int type = control > 128 ? EV_KEY : EV_ABS;
		int value = event.value ? direction : 0;
		send_event(gamepad_fd, type, control, event.value ? direction : 0);
	}
	close(keyboard_fd);
	close(gamepad_fd);
	return 0;
}
