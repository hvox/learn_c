#include <stdio.h>
int var1, var2;
int main(void) {
	int *p = &var1, *q = &var2 + 1;
	printf("p    = %p\n", p);
	printf("q    = %p\n", q);
	printf("p==q = %d\n", p == q);
}
