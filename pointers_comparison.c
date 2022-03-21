#include <stdio.h>
int a, b;
int main(void) {
	int *pa = &a, *pb = &b + 1;
	printf("&a       = %p\n", pa);
	printf("&b+1     = %p\n", pb);
	printf("&a==&b+1 = %d\n", pa == pb);
}
