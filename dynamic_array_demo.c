#include <stdio.h>
#include "dynamic_array.h"

void print_array(int array[]) {
	for (int i = 0; i < len(array); i++)
		printf("%d -> %d\n", i, array[i]);
}

int main() {
	int *array = new_array_of_type(int);
	int x;
	while (printf(">>> "), scanf(" %d", &x) == 1) {
		array = push(array, &x);
		printf("array(%p) = [%d", array, array[0]);
		for (int i = 1; i < len(array); i++)
			printf(", %d", array[i]);
		printf("]\n");
	}
	del_array(array);
}
