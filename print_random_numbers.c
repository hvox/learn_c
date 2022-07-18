#include <stdio.h>
#include <stdlib.h>

int main() {
	printf(">>> ");
	unsigned int seed;
	scanf(" %d", &seed);
	srand(seed);
	for (int i = 0; i < 32; i++)
		printf("[%d] = %d\n", i, rand());
	return 0;
}
