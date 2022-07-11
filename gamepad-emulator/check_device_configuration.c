#include "parser.h"

char *USAGE_MESSAGE =
  "usage: %s <periphery configuration file>\n";

int main(int argc, char *argv[]) {
	if (argc != 2) {
		printf(USAGE_MESSAGE, argv[0], argv[0]);
		return 42;
	}
	struct device_configuration *device = read_device_configuration(argv[1]);
	if (device == NULL) {
		fprintf(stderr, "Failed to read device configuration\n");
		return 42;
	}
	printf("device configuration is ok\n");
	printf("device name = \"%s\"\n", device->id.name);
	return 0;
}
