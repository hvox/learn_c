#include <stdio.h>
int main() {
	char s[256];
	while (printf(">>> "), scanf(" %255[^\n]", s) == 1) {
		printf("%s\n", s);
	}
}
