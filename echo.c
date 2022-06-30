#include <stdio.h>
int main() {
	char s[256];
	register char *reg = s;
	while (printf(">>> "), scanf(" %255[^\n]", reg) == 1) {
		printf("%s\n", reg);
	}
}
