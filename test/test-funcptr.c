int foo(char *(*bar)(void))
{
	printf("hello %s!\n", (*bar)());
	return 0;
}

char *baz() { return "world"; }

int main()
{
	foo(&baz);
	return 0;
}
