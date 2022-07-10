#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>
#include "dynamic_array.h"

unsigned char *read_bytes(int fd) {
	struct stat info;
	if (fstat(fd, &info) < 0 || info.st_size > 65535)
		return NULL;
	unsigned char *bytes = new_array(info.st_size, 1);
	read(fd, bytes, info.st_size);
	return bytes;
}

int main(int argc, char *argv[]) {
	if (argc < 2) fprintf(stderr, "Invalid usage\n");

	int file = open(argv[1], O_RDONLY);
	if (file < 0) {
		perror("Failed to open file");
		return 42;
	}
	unsigned char *bytes = read_bytes(file);
	if (bytes == NULL) {
		if (errno)
			perror("Failed to read file");
		else
			fprintf(stderr, "It's too big to be displayed\n");
		return 42;
	}
	close(file);

	int size = len(bytes);
	if (size)
		printf(" ROW  %-47s  ASCII\n", "BYTES");
	for (int row = 0; row < size; row += 16) {
		printf(" %03x:", row / 16);
		for (int i = row; i < row + 16; i++)
			if (i < size)
				printf(" %02x", bytes[i]);
			else
				printf(" --");
		printf("  ");
		for (int i = row; i < row + 16 && i < size; i++) {
			if (32 <= bytes[i] && bytes[i] <= 126)
				printf("%c", bytes[i]);
			else if (bytes[i] == 9)
				printf("·");
			else if (bytes[i] == 10)
				printf("¶");
			else
				printf("᰽");
		}
		printf("\n");
	}
	printf("file size = %d bytes\n", size);
}
