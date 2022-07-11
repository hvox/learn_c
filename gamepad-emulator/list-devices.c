#include <dirent.h>
#include <fcntl.h>
#include <linux/uinput.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int is_prefix(const char *str, const char *prefix) {
	if (prefix == NULL || str == NULL) return 0;
	for (int i = 0; prefix[i]; i++)
		if (str[i] != prefix[i])
			return 0;
	return 1;
}

int scan_devices() {
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

	for (int i = 0; i < count; i++) {
		char path[20] = "/dev/input/";
		strcat(path, inputs[i]->d_name);
		int fd = open(path, O_RDONLY);
		if (fd < 0) {
			perror(path);
			continue;
		}
		char name[UINPUT_MAX_NAME_SIZE];
		if (ioctl(fd, EVIOCGNAME(UINPUT_MAX_NAME_SIZE), name) < 0)
			perror(path);
		close(fd);
		free(inputs[i]);
		printf("%s: %s\n", path, name);
	}
	return 0;
}

int main() {
	return scan_devices();
}
