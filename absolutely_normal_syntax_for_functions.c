#include <stdio.h>

f(); // absolutely ok prototype

int main() {
	for (int i = 0; i < 5; i++)
		printf("%d : f() -> %d\n", i, f());
}

f() {
	static int counter = 0;
	return ++counter;
}
