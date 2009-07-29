#include <string.h>

void test(const char *s, int c)
{
	const char *x, *y;
	x = strchr(s, c);
	y = strrchr(s, c);
	printf("%d %d\n", (x ? (int)(x - s) : 999) , (y ? (int)(y - s) : 999));
}

int main()
{
	test("abcde\0abc", 'a');
	test("abcde\0abc", 'b');
	test("abcde\0abc", 'c');
	test("abcde\0abc", 'd');
	test("abcde\0abc", 'e');
	test("abcde\0abc", 'f');
	test("abcde\0abc", '\0');
	test("abcdefdecbabcdefedbca\0abc", 'a');
	test("abcdefdecbabcdefedbca\0abc", 'b');
	test("abcdefdecbabcdefedbca\0abc", 'c');
	test("abcdefdecbabcdefedbca\0abc", 'd');
	test("abcdefdecbabcdefedbca\0abc", 'e');
	test("abcdefdecbabcdefedbca\0abc", 'g');
	return 0;
}
