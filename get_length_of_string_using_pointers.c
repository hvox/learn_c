#include <stdio.h>

int main() {
	char s[256];
	printf("Input string: ");
	fgets(s, 255, stdin);
	char *ps = s;
	while (*ps) ps++;
	int length = ps - s;
	printf("length of the string in bytes: %d\n", length);
	printf("bytes:");
	for (int i = 0; i < length; i++)
		printf(" 0x%02x", (unsigned char)s[i]);
	puts("");
}
