#include <stdio.h>
// this code is platform specific and does not work 64-bit architectures

int f1(int n, ...) {
	int sum = 0;
	int *parameters = &n + 1;
	for (int i = 0; i < n; i++) sum += parameters[i];
	for (int i = 0; i < n; i++) printf("%d -> %d\n", i, parameters[i]);
	return sum;
}

int f2(int n, int p, ...) {
	int sum = 0;
	int *parameters = &p;
	for (int i = 0; i < n; i++) sum += parameters[i];
	for (int i = 0; i < n; i++) printf("%d -> %d\n", i, parameters[i]);
	return sum;
}

int main() {
	int x = 42, y = 59, z = 64;
	printf("sum1 = %d\n", f1(3, x, y, z));
	printf("sum2 = %d\n", f2(3, x, y, z));
}
