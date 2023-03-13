#include <unistd.h>
#include <string.h>
#include <stdio.h>

int main(int argc, char *argv[]) {
	char msg[20];
	for (int fd = 0; fd < 16; fd++) {
		sprintf(msg, "Hello, file#%i!\n", fd);
		int code = write(fd, msg, sizeof(msg));
		if (code == -1) {
			fprintf(stderr, "Failed to write to file#%i\n", fd);
		}
	}
}
