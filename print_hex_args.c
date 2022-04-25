#include <stdio.h>
#include <stdlib.h>

int main(int n, char **args) {
	for (int i = 0; i < n; i++) {
		char *numb_end;
		int numb = strtol(args[i], &numb_end, 16);
		if (numb_end != args[i])
			printf("arg[%d] : int = %d;\n", i, numb);
		else
			printf("arg[%d] : str = %s;\n", i, args[i]);
	}
}
