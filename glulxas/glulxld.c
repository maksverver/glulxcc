#include <stdio.h>

void usage()
{
	printf("usage: glulxld [-o module] object ...\n");
}

static void read_input(const char *path)
{
	printf("TODO: read input from %s\n", path);
}

static void process()
{
	/* TODO */
}

static void write_output(const char *path)
{
	printf("TODO: write output to %s\n", path);
}

int main(int argc, char *argv[])
{
	const char *out = "a.ulx";
	int i = 1;
	if (i < argc && argv[i][0] == '-')
	{
		if (argv[i][1] == 'o')
		{
			if (argv[i][2] != '\0')
			{
				out = argv[i] + 2;
				i++;
			}
			else
			if (i + 1 < argc)
			{
				out = argv[i + 1];
				i += 2;
			}
		}
		else
		{
			usage();
			return 1;
		}
	}
	if (i == argc)  /* no input files */
	{
		usage();
		return 1;
	}
	for ( ; i < argc; ++i) read_input(argv[i]);
	process();
	write_output(out);
	return 0;
}
