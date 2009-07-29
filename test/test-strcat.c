#include "string.h"
#include "stdio.h"

void test(const char *init, const char *src, int n)
{
	char dest[20], *res;
	int i;
	for (i = 0; init[i]; ++i) dest[i] = init[i];
	dest[i++] = '\0';
	for ( ; i < 20; ++i) dest[i] = 'A' + i;
	res = (n < 0) ? strcat(dest, src) : strncat(dest, src, (size_t)n);
	for (i = 0; i < 20; ++i) putchar(dest[i] ? dest[i] : '-');
	printf(" %d\n", (int)(res - dest));
}

int main()
{
	test("", "foobar", -1);
	test("foo", "bar", -1);
	test("", "foobar", 10);
	test("foo", "bar",  5);
	test("hello", "world", 2);
	test("foo", "bar", 0);
	test("", "", 0);
	return 0;
}
