#include "stdio.h"
#include "string.h"

void test1(const char *a, const char *b, size_t n, int expected)
{
	int res = memcmp(a, b, n);
	printf("memcmp(\"%s\", \"%s\", %d); --> %d\n", a, b, (int)n, res);
	if (res != expected) printf("FAILED: got: %d, expected: %d\n", res, expected);
}

void test2(const char *a, const char *b, int expected)
{
	int res = strcmp(a, b);
	printf("strcmp(\"%s\", \"%s\"); --> %d\n", a, b, res);
	if (res != expected) printf("FAILED: got: %d, expected: %d\n", res, expected);
}

void test3(const char *a, const char *b, size_t n, int expected)
{
	int res = strncmp(a, b, n);
	printf("strncmp(\"%s\", \"%s\", %d); --> %d\n", a, b, (int)n, res);
	if (res != expected) printf("FAILED: got: %d, expected: %d\n", res, expected);
}

int main()
{
	test1("foo", "bar", 3, +1);
	test1("bar", "barfood", 999, -1);
	test1("foobar", "football", 3, 0);
	test1("w00t", "blaat", 0, 0);
	test1("abcd", "abcd", 4, 0);
	test2("foo", "bar", +1);
	test2("foobarbazquux", "foobarbazquux", 0);
	test2("foobarba", "foobarbazquux", -1);
	test2("foobarbazquux", "foobarbazqu", +1);
	test2("abcdeffdsjklhfdhfldsa", "abcdefugfodsgf", -1);
	test2("z", "y", 1);
	test2("a", "b", -1);
	test2("", "adsf", -1);
	test2("xyzzy", "", +1);
	test2("", "", 0);
	test2("equal", "equal", 0);
	test3("foobarba", "foobarbazquux", 10, -1);
	test3("foobarbazquux", "foobarbazqu", 11, 0);
	test3("foobarbazquux", "foobarbazqu", 12, 1);
	test3("foobarbazquux", "foobarbazqu", 5, 0);
	test3("asdf", "qwert", 0, 0);
	test3("asdf", "qwert", 999, -1);
	test3("equal", "equal", 999, 0);
	return 0;
}
