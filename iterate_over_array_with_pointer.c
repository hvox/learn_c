#include <stdio.h>
#define max_array_size 4

int A[max_array_size][max_array_size];
int main() {
	int n;
	printf("Size of an matrix: ");
	scanf("%d", &n);

	for (int i = 0; i < n; i++) {
		printf("row #%d: ", i + 1);
		for (int *p = *(A + i); p < *(A + i) + n; p++)
			scanf("%d", p);
	}

	int *Alast = *A + n * max_array_size - 1;
	for (int *p = *A + n - 1; p < Alast; p += max_array_size - 1)
		printf("%p -> %d\n", p, *p);
}
