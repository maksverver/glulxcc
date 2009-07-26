extern int printf(const char *, ...);

void init(int *a)
{
	a[0] = 123;
	a[2] = 7890;
	a[1] = 456;
}

int main()
{
	int foo[3];
	init(foo);
	printf("%d %d %d\n", foo[0], foo[1], foo[2]);
	return 0;
}
