#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

int global_variable;
void print_pointer(char *title, void *ptr);

int main() {
	int local_variable;
	print_pointer("code", main);
	print_pointer("data", &global_variable);
	print_pointer("heap", &local_variable);
	print_pointer("stck", malloc(1));
}

void print_pointer(char *title, void *ptr) {
	printf("%s : %12p  Δ=%012lx\n", title, ptr, ptr - (void *)main);
}
