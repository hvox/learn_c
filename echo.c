#include <stdio.h>
int main() {
	char s[256];
	register char *reg = s;
	while (printf(">>> "), scanf(" %255[^\n]", reg) == 1) {
		for (char *ch = s; *ch; ch++)
			*ch -= (*ch > 96 && *ch < 123) ? 32 : 0;
		printf("%s\n", reg);
	}
}
