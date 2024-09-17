#include <stdio.h>

int main(int argc, char *argv[]) {
	int x = 8;
	double y = 3.555000;
	if ((x * y) == (int)(x * y)) {
		printf("%d\n", (int)(x * y));
	} else {
		printf("%.6f\n", (double)(x * y));
	}
	return 0;
}
