#include <stdio.h>

int A[99][99];
int main() {
	int n;
	printf("Size of square array: ");
	scanf("%d", &n);
	int *A_end = *(A + n);
	for (int i = 0; i < n; i++)
		for (int j = 0; j < n; j++)
			scanf("%d", &A[i][j]);

	for (int *row = *A, *rend = row + n; row < A_end; row += 99, rend += 99)
		for (int *p = row; p < rend; p++)
			printf("%p -> %d\n", p, *p);
}
