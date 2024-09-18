#include <stdio.h>
#include "tools.h"
int main(int argc, char *argv[])
{
	int one = 1;
	if ((increment(3) + increment(4)) == (int)(increment(3) + increment(4)))
	{
		printf("%d\n", (int)(increment(3) + increment(4)));
	}
	else
	{
		printf("%.6f\n", (double)(increment(3) + increment(4)));
	}
	return 0;
	return 0;
}
