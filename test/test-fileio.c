#include "assert.h"
#include "stdio.h"
#include "string.h"

int main()
{
    FILE *f, *g, *h;
    fpos_t pos;
    char buf[50], *line;
    int res, offset, length;

    printf("A:\n");
    f = fopen("foobar.tmp", "wt");
    assert(f != NULL);
    fputs("Lore ipsum", f);
    fputs(" dolor sit ", f);
    fputs("amet.\n", f);
    fputs("Line the second.\n", f);

    printf("B:\n");
    g = tmpfile();
    assert(g != NULL);
    res = fwrite("Hello", 5, 1, g);
    assert(res == 1);
    putc(' ', g);
    res = fwrite("world!", 1, 6, g);
    assert(res == 6);
    fputc('\n', g);
    offset = ftell(g);
    assert(offset == 13);
    res = fgetpos(g, &pos);
    assert(res == 0);
    rewind(g);
    line = fgets(buf, sizeof(buf), g);
    assert(line == buf);
    res = strcmp(line, "Hello world!\n");
    assert(res == 0);
    fclose(g);
    fclose(f);

    printf("C:\n");
    h = fopen("foobar.tmp", "rt");
    assert(h != NULL);
    line = fgets(buf, sizeof(buf), h);
    assert(line != NULL);
    res = strcmp(buf, "Lore ipsum dolor sit amet.\n");
    assert(res == 0);
    offset = ftell(h);
    fseek(h, 0, SEEK_END);
    length = ftell(h);
    fseek(h, offset - length, SEEK_END);
    line = fgets(buf, sizeof(buf), h);
    assert(line != NULL);
    res = strcmp(buf, "Line the second.\n");
    assert(res == 0);
    rewind(h);
    res = (int)fread(buf, 1, sizeof(buf), h);
    assert(res == 44);
    res = strcmp(buf, "Lore ipsum dolor sit amet.\n" "Line the second.\n");
    assert(res == 0);
    fseek(h, 0, SEEK_SET);
    memset(buf, 0, sizeof(buf));
    res = (int)fread(buf, 10, 1, h);
    assert(res == 1);
    res = strcmp(buf, "Lore ipsum");
    assert(res == 0);
    line = fgets(buf + 10, 11, h);
    assert(line != NULL);
    res = strcmp(buf, "Lore ipsum dolor sit");
    assert(res == 0);
    res = (int)fread(buf, 10, 2, h);
    assert(res == 2);
    res = strcmp(buf, " amet.\nLine the seco");
    assert(res == 0);
    fclose(h);

    remove("foobar.tmp");
    h = fopen("foobar.tmp", "rt");
    assert(h == NULL);

    return 0;
}
