#include "stdlib.h"
int main()
{
	int i = 0;
	for (i = 0; i < 10; ++i) printf("%10d\n", rand());
	printf("--srand 1\n");
	srand(1);
	for (i = 0; i < 10; ++i) printf("%10d\n", rand());
	printf("--srand 2\n");
	srand(2);
	for (i = 0; i < 10; ++i) printf("%10d\n", rand());
	printf("--srand 0\n");
	srand(0);
	for (i = 0; i < 10; ++i) printf("%10d\n", rand());
	printf("--srand 1\n");
	srand(1);
	for (i = 0; i < 10; ++i) printf("%10d\n", rand());
	return 0;
}
