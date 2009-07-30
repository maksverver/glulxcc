#include "stdio.h"
#include "ctype.h"

int main()
{
	int ch;
	while ((ch = getchar()) != -1)
	{
		if (isgraph(ch))
			printf("(%c)", ch);
		else
			printf("(%#x)", (int)ch);
		if (ch == '\n') putchar('\n');
	}
	return 0;
}
