#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

int global_variable;
void print_pointer(char *title, void *ptr);

const int GIGABYTES_TO_ALLOCATE = 123;

int main() {
	void *ptrs[GIGABYTES_TO_ALLOCATE];
	for (int gb = 1; gb <= GIGABYTES_TO_ALLOCATE; gb++) {
		int i = gb - 1;
		ptrs[i] = malloc(1024 * 1024 * 1024);
		printf("%i GB allocated successfully\n", gb);
	}
	printf("Press ENTER to free allocated memory: ");
	getchar();
	for (int i = 0; i < GIGABYTES_TO_ALLOCATE; i++) {
		int *memory = ptrs[i];
		printf("%12p: %i, %i\n", memory, memory[0],
			   memory[(1024 * 1024 * 1024 - 1) / sizeof(int)]);
		free(ptrs[i]);
	}
}

void print_pointer(char *title, void *ptr) {
	printf("%s : %12p  Δ=%012lx\n", title, ptr, ptr - (void *)main);
}
