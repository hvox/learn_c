#include <errno.h>
#include <fcntl.h>
#include <linux/uinput.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

char *USAGE_MESSAGE =
  "usage: %s <periphery event file>\n\n"
  "examples:\n  %s /dev/input/by-id/usb-Logitech_USB_Keyboard-event-kbd\n";

char *KEY_NAMES[701] = {
  "this_button_does_not_have_a_name_yet", "esc", "1", "2", "3", "4", "5", "6",
  "7", "8", "9", "0", "minus", "equal", "backspace", "tab", "q", "w", "e", "r",
  "t", "y", "u", "i", "o", "p", "leftbrace", "rightbrace", "enter", "leftctrl",
  "a", "s", "d", "f", "g", "h", "j", "k", "l", "semicolon", "apostrophe",
  "grave", "leftshift", "backslash", "z", "x", "c", "v", "b", "n", "m", "comma",
  "dot", "slash", "rightshift", "kpasterisk", "leftalt", "space", "capslock",
  "f1", "f2", "f3", "f4", "f5", "f6", "f7", "f8", "f9", "f10", "numlock",
  "scrolllock", "kp7", "kp8", "kp9", "kpminus", "kp4", "kp5", "kp6", "kpplus",
  "kp1", "kp2", "kp3", "kp0", "kpdot", NULL, "zenkakuhankaku", "102nd", "f11",
  "f12", "ro", "katakana", "hiragana", "henkan", "katakanahiragana", "muhenkan",
  "kpjpcomma", "kpenter", "rightctrl", "kpslash", "sysrq", "rightalt",
  "linefeed", "home", "up", "pageup", "left", "right", "end", "down",
  "pagedown", "insert", "delete", "macro", "mute", "volumedown", "volumeup",
  "power", "kpequal", "kpplusminus", "pause", "scale", "kpcomma", "hanguel",
  "hanja", "yen", "leftmeta", "rightmeta", "compose", "stop", "again", "props",
  "undo", "front", "copy", "open", "paste", "find", "cut", "help", "menu",
  "calc", "setup", "sleep", "wakeup", "file", "sendfile", "deletefile", "xfer",
  "prog1", "prog2", "www", "msdos", "screenlock", "rotate_display",
  "cyclewindows", "mail", "bookmarks", "computer", "back", "forward", "closecd",
  "ejectcd", "ejectclosecd", "nextsong", "playpause", "previoussong", "stopcd",
  "record", "rewind", "phone", "iso", "config", "homepage", "refresh", "exit",
  "move", "edit", "scrollup", "scrolldown", "kpleftparen", "kprightparen",
  "new", "redo", "f13", "f14", "f15", "f16", "f17", "f18", "f19", "f20", "f21",
  "f22", "f23", "f24", NULL, NULL, NULL, NULL, NULL, "playcd", "pausecd",
  "prog3", "prog4", "dashboard", "suspend", "close", "play", "fastforward",
  "bassboost", "print", "hp", "camera", "sound", "question", "email", "chat",
  "search", "connect", "finance", "sport", "shop", "alterase", "cancel",
  "brightnessdown", "brightnessup", "media", "switchvideomode",
  "kbdillumtoggle", "kbdillumdown", "kbdillumup", "send", "reply",
  "forwardmail", "save", "documents", "battery", "bluetooth", "wlan", "uwb",
  "unknown", "video_next", "video_prev", "brightness_cycle", "brightness_auto",
  "display_off", "wireless_wan", "radio_kill", "micmute", NULL, NULL, NULL,
  NULL, NULL, NULL, "reserved_by_at", "btn_0", "btn_1", "btn_2", "btn_3",
  "btn_4", "btn_5", "btn_6", "btn_7", "btn_8", "btn_9", NULL, NULL, NULL, NULL,
  NULL, NULL, "btn_left", "btn_right", "btn_middle", "btn_side", "btn_extra",
  "btn_forward", "btn_back", "btn_task", NULL, NULL, NULL, NULL, NULL, NULL,
  NULL, NULL, "btn_trigger", "btn_thumb", "btn_thumb2", "btn_top", "btn_top2",
  "btn_pinkie", "btn_base", "btn_base2", "btn_base3", "btn_base4", "btn_base5",
  "btn_base6", NULL, NULL, NULL, "btn_dead", "btn_a", "btn_b", "btn_c", "btn_x",
  "btn_y", "btn_z", "btn_tl", "btn_tr", "btn_tl2", "btn_tr2", "btn_select",
  "btn_start", "btn_mode", "btn_thumbl", "btn_thumbr", NULL, "btn_tool_pen",
  "btn_tool_rubber", "btn_tool_brush", "btn_tool_pencil", "btn_tool_airbrush",
  "btn_tool_finger", "btn_tool_mouse", "btn_tool_lens", "btn_tool_quinttap",
  "btn_stylus3", "btn_touch", "btn_stylus", "btn_stylus2", "btn_tool_doubletap",
  "btn_tool_tripletap", "btn_tool_quadtap", "btn_gear_down", "btn_gear_up",
  NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
  NULL, "ok", "select", "goto", "clear", "power2", "option", "info", "time",
  "vendor", "archive", "program", "channel", "favorites", "epg", "pvr", "mhp",
  "language", "title", "subtitle", "angle", "zoom", "mode", "keyboard",
  "screen", "pc", "tv", "tv2", "vcr", "vcr2", "sat", "sat2", "cd", "tape",
  "radio", "tuner", "player", "text", "dvd", "aux", "mp3", "audio", "video",
  "directory", "list", "memo", "calendar", "red", "green", "yellow", "blue",
  "channelup", "channeldown", "first", "last", "ab", "next", "restart", "slow",
  "shuffle", "break", "previous", "digits", "teen", "twen", "videophone",
  "games", "zoomin", "zoomout", "zoomreset", "wordprocessor", "editor",
  "spreadsheet", "graphicseditor", "presentation", "database", "news",
  "voicemail", "addressbook", "messenger", "displaytoggle", "spellcheck",
  "logoff", "dollar", "euro", "frameback", "frameforward", "context_menu",
  "media_repeat", "10channelsup", "10channelsdown", "images", NULL,
  "notification_center", "pickup_phone", "hangup_phone", NULL, "del_eol",
  "del_eos", "ins_line", "del_line", NULL, NULL, NULL, NULL, NULL, NULL, NULL,
  NULL, NULL, NULL, NULL, NULL, "fn", "fn_esc", "fn_f1", "fn_f2", "fn_f3",
  "fn_f4", "fn_f5", "fn_f6", "fn_f7", "fn_f8", "fn_f9", "fn_f10", "fn_f11",
  "fn_f12", "fn_1", "fn_2", "fn_d", "fn_e", "fn_f", "fn_s", "fn_b",
  "fn_right_shift", NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
  NULL, "brl_dot1", "brl_dot2", "brl_dot3", "brl_dot4", "brl_dot5", "brl_dot6",
  "brl_dot7", "brl_dot8", "brl_dot9", "brl_dot10", NULL, NULL, NULL, NULL, NULL,
  "numeric_0", "numeric_1", "numeric_2", "numeric_3", "numeric_4", "numeric_5",
  "numeric_6", "numeric_7", "numeric_8", "numeric_9", "numeric_star",
  "numeric_pound", "numeric_a", "numeric_b", "numeric_c", "numeric_d",
  "camera_focus", "wps_button", "touchpad_toggle", "touchpad_on",
  "touchpad_off", "camera_zoomin", "camera_zoomout", "camera_up", "camera_down",
  "camera_left", "camera_right", "attendant_on", "attendant_off",
  "attendant_toggle", "lights_toggle", NULL, "btn_dpad_up", "btn_dpad_down",
  "btn_dpad_left", "btn_dpad_right", NULL, NULL, NULL, NULL, NULL, NULL, NULL,
  NULL, NULL, NULL, NULL, NULL, "als_toggle", "rotate_lock_toggle", NULL, NULL,
  NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
  "buttonconfig", "taskmanager", "journal", "controlpanel", "appselect",
  "screensaver", "voicecommand", "assistant", "kbd_layout_next", "emoji_picker",
  "dictate", NULL, NULL, NULL, NULL, NULL, "brightness_min", "brightness_max",
  NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
  NULL, "kbdinputassist_prev", "kbdinputassist_next",
  "kbdinputassist_prevgroup", "kbdinputassist_nextgroup",
  "kbdinputassist_accept", "kbdinputassist_cancel", "right_up", "right_down",
  "left_up", "left_down", "root_menu", "media_top_menu", "numeric_11",
  "numeric_12", "audio_desc", "3d_mode", "next_favorite", "stop_record",
  "pause_record", "vod", "unmute", "fastreverse", "slowreverse", "data",
  "onscreen_keyboard", "privacy_screen_toggle", "selective_screenshot",
  "next_element", "previous_element", "autopilot_engage_toggle",
  "mark_waypoint", "sos", "nav_chart", "fishing_chart", "single_range_radar",
  "dual_range_radar", "radar_overlay", "traditional_sonar", "clearvu_sonar",
  "sidevu_sonar", "nav_info", "brightness_menu", NULL, NULL, NULL, NULL, NULL,
  NULL, "macro1", "macro2", "macro3", "macro4", "macro5", "macro6", "macro7",
  "macro8", "macro9", "macro10", "macro11", "macro12", "macro13", "macro14",
  "macro15", "macro16", "macro17", "macro18", "macro19", "macro20", "macro21",
  "macro22", "macro23", "macro24", "macro25", "macro26", "macro27", "macro28",
  "macro29", "macro30", NULL, NULL, "macro_record_start", "macro_record_stop",
  "macro_preset_cycle", "macro_preset1", "macro_preset2", "macro_preset3", NULL,
  NULL, "kbd_lcd_menu1", "kbd_lcd_menu2", "kbd_lcd_menu3", "kbd_lcd_menu4",
  "kbd_lcd_menu5",
};

char *AXES_NAMES[62] = {
  "x", "y", "z", "rx", "ry", "rz", "throttle", "rudder", "wheel", "gas",
  "brake", NULL, NULL, NULL, NULL, NULL, "hat0x", "hat0y", "hat1x", "hat1y",
  "hat2x", "hat2y", "hat3x", "hat3y", "pressure", "distance", "tilt_x",
  "tilt_y", "tool_width", NULL, NULL, NULL, "volume", NULL, NULL, NULL, NULL,
  NULL, NULL, NULL, "misc", NULL, NULL, NULL, NULL, NULL, "reserved", "mt_slot",
  "mt_touch_major", "mt_touch_minor", "mt_width_major", "mt_width_minor",
  "mt_orientation", "mt_position_x", "mt_position_y", "mt_tool_type",
  "mt_blob_id", "mt_tracking_id", "mt_pressure", "mt_distance", "mt_tool_x",
  "mt_tool_y",
};

int connect_to_periphery(char *periphery_path) {
	int periphery = open(periphery_path, O_RDONLY);
	if (periphery == -1) return -1;
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
