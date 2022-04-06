#include <stdio.h>
#include <string.h>

int countOnes(unsigned char byte) {
	int ones = 0;
	for (int i = 0; i < 8; i++)
		ones += (byte >> i) & 1;
	return ones;
}

int main() {
	char s[80];
	printf("Input string: ");
	fgets(s, 79, stdin);
	int n = strlen(s);
	
	FILE *output = fopen("output.txt", "w");
	if (output == NULL) {
		printf("ERROR! CAN'T OPEN \"output.txt\"");
		return 42;
	}
	
	fprintf(output, "%s\n", s);
	for (int i = 0; i < n; i++)
		fprintf(output, "%02X ", (unsigned char)s[i]);
	fprintf(output, "\n");
	
	int j = 0;
	int buffur_size = 0;
	unsigned short buffer = 0;
	for (int i = 0; i < n; i++) {
		unsigned char byte = s[i];
		int ones = countOnes(byte);
		int zeros = 8 - ones;
		if (ones > zeros) {
			buffer = buffer << 7 | (byte & 127);
			buffur_size += 7;
		}
		else {
			buffer = buffer << 8 | byte;
			buffur_size += 8;
		}
		if (buffur_size >= 8) {
			s[j++] = (buffer >> (buffur_size - 8)) & 255;
			buffur_size -= 8;
		}
	}
	if (buffur_size)
		s[j++] = buffer << (8 - buffur_size);
	s[j] = 0;
	
	for (int i = 0; i < n; i++)
		fprintf(output, "%02X ", (unsigned char)s[i]);
	fprintf(output, "\n");
	fprintf(output, "%s\n", s);
	fclose(output);
	printf("Output was saved to \"output.txt\"!\n");
	return 0;
}
	
