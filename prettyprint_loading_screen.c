#include <stdio.h>
#include <unistd.h>

void print(char *string) {
	printf("%s", string);
	fflush(stdout);
}

void println(char *string) {
	printf("%-72s\n", string);
	fflush(stdout);
}

int main() {
	int delta_time = 400 * 1000;
	for (int counter = 1; counter; counter++) {
		print("loading");
		usleep(delta_time);
		for (int i = 0; i < 3; i++) {
			print(".");
			usleep(delta_time);
		}
		char output[80];
		sprintf(output, "%c i = %d", 13, counter);
		println(output);
	}
}
