#include "stdio.h"
#include "stdlib.h"

void foo() { printf("foo\n"); }
void bar() { printf("bar\n"); }
void baz() { printf("baz\n"); }
void quux()
{
	static int cnt = 1;
	printf("quux %d\n", cnt++);
}

/* Prints digits correspondig with atexit() return values;
   last three digits should be 1, rest 0. Then, the atexit handlers
   should print in reverse order. */
int main()
{
	int i;
	printf("%d", atexit(&foo));
	printf("%d", atexit(&bar));
	printf("%d", atexit(&baz));
	for (i = 0; i < ATEXIT_MAX; ++i) printf("%d", atexit(&quux));
	printf("\n");
	/*exit(0);*/  /* should work both with exit() or return from main() */
	return 0;
}
