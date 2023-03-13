#include <unistd.h>

int main(int argc, char *argv[]) {
	char msg[] = "Hello there\n";
	write(1, msg, sizeof(msg));
}
