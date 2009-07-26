extern int printf(const char *fmt, ...);

struct X { int x; int y; short z; };

struct X foo(struct X x)
{
	struct X res;
	res = x;
	res.x += 1;
	res.z -= 123;
	return res;
}

void init(struct X *x)
{
	x->x = 1234566;
	x->y = 'q';
	x->z = 890;
}

int main()
{
	struct X x;
	init(&x);
	printf("%d %c %d\n", x.x, x.y, x.z);
	x = foo(x);
	printf("%d %c %d\n", x.x, x.y, x.z);
	return 0;
}
