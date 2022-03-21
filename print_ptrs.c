#include <stdio.h>

int global_variable;

void recursion(int i, int n) {
	int local_variable;
	if (i > n) return;
	printf("enter recursion %d : %p\n", i, &local_variable);
	recursion(i + 1, n);
	printf("leave recursion %d : %p\n", i, &local_variable);
}

int main() {
	int x, y;
	printf("main : %p\n", main);
	printf("recr : %p\n", recursion);
	printf("glbl : %p\n", &global_variable);
	printf("x    : %p\n", &x);
	printf("y    : %p\n", &y);
	recursion(1, 9);
}
