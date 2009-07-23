#include <assert.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef unsigned char  uchar;
typedef unsigned short ushort;
typedef unsigned int   uint;

enum secttype { SECTION_CODE = 0, SECTION_DATA = 1, SECTION_VCODE = 2,
                SECTION_VDATA = 3, SECTION_BSS = 4, SECTION_INVALID = -1 };

struct section
{
    /* read from the object file: */
    enum secttype type;
    size_t size;
    char *data;

    /* assigned by the linker: */
    bool used;
    uint address;
};

struct export
{
    const char *name;
    uint section, offset;
};

struct xref
{
    uint patch_section;
    uint patch_offset;
    const char *name;  /* if NULL, section/offset are valid instead */
    uint section;
    uint offset;
};

/* Global data */
static uint nsection;
static struct section *sections;
static uint nexport;
static struct export *exports;
static uint nxref;
static struct xref *xrefs;

static uint main_section;
static uint main_offset;
static uint decodingtbl_section;
static uint decodingtbl_offset;
static uint nused;
static struct section **used;
static uint ramstart;
static uint extstart;
static uint endmem;
static uint stacksize;
static char *module;

/* For current file: */
static uint start_section;
static uint nimport;
static const char **imports;

#define resize(elem, new_size) \
    do_resize((void**)&elem##s, &n##elem, sizeof(*elem##s), new_size)

/* Returns the smallest power-of-two no smaller than `size'. */
static uint cap(uint size)
{
    uint res;
    if (size == 0) return 0;
    res = 1;
    while (res < size) res += res;
    return res;
}

static void do_resize( void **data, uint *size, size_t elem_size,
                       uint new_size )
{
    if (cap(*size) != cap(new_size))
    {
        *data = realloc(*data, cap(new_size)*elem_size);
        assert(new_size == 0 || *data != NULL);
    }
    *size = new_size;
}

static uint roundup(uint n, uint align)
{
    uint m = n%align;
    if (m != 0) n += align - m;
    return n;
}

static void usage()
{
    printf("usage: glulxld [-o module] object ...\n");
}

static void fatal(const char *fmt, ...)
{
    char buf[1024];
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(buf, sizeof(buf), fmt, ap);
    va_end(ap);
    fprintf(stderr, "ERROR: %s!\n", buf);
    exit(1);
}

static uint32_t get_int(const void *data)
{
    uint32_t res = 0;
    res |= (uint32_t)((unsigned char*)data)[0] << 24;
    res |= (uint32_t)((unsigned char*)data)[1] << 16;
    res |= (uint32_t)((unsigned char*)data)[2] <<  8;
    res |= (uint32_t)((unsigned char*)data)[3] <<  0;
    return res;
}

static void set_int(void *data, uint32_t value)
{
    ((unsigned char*)data)[0] = (value >> 24)&0xff;
    ((unsigned char*)data)[1] = (value >> 16)&0xff;
    ((unsigned char*)data)[2] = (value >>  8)&0xff;
    ((unsigned char*)data)[3] = (value >>  0)&0xff;
}

static uint32_t read_int(FILE *fp)
{
    unsigned char buf[4];
    if (fread(buf, 1, 4, fp) != 4) fatal("read failed");
    return get_int(buf);
}

static char *read_table(FILE *fp, size_t *size)
{
    char buf[4], *data;
    if (fread(buf, 1, 4, fp) != 4) return NULL;
    *size = (size_t)get_int(buf);
    if (*size < 4 || *size%4 != 0) return NULL;
    data = malloc(*size);
    if (data == NULL) return NULL;
    if (fread(data, 1, *size, fp) != *size)
    {
        free(data);
        return NULL;
    }
    return data;
}

/* FIXME: this function does not check for buffer overflows */
static bool parse_code_table(char *data, size_t size)
{
    uint n, nsect;
    nsect = get_int(data);
    data += 4;
    resize(section, nsection + nsect);
    for (n = start_section; n < nsection; ++n)
    {
        sections[n].type = get_int(data + 0);
        sections[n].size = get_int(data + 4);
        data += 8;
        if (sections[n].type == SECTION_BSS)
        {
            sections[n].data = NULL;
        }
        else
        {
            sections[n].data = data;
            data += roundup(sections[n].size, 4);
        }
    }
    return true;
}

/* FIXME: this function does not check for buffer overflows */
static bool parse_import_table(char *data, size_t size)
{
    uint n;
    assert(nimport == 0 && imports == NULL);
    nimport = get_int(data);
    imports = malloc(nimport*sizeof(*imports));
    for (n = 0; n < nimport; ++n)
        imports[n] = data + get_int(data + 4 + 4*n);
    return true;
}

/* FIXME: this function does not check for buffer overflows */
static bool parse_export_table(char *data, size_t size)
{
    uint nexp = get_int(data), n;
    resize(export, nexport + nexp);
    for (n = nexport - nexp; n < nexport; ++n)
    {
        char *entry = data + 4 + 12*n;
        exports[n].name    = get_int(entry + 0) + data;
        exports[n].section = get_int(entry + 4) + start_section;
        exports[n].offset  = get_int(entry + 8);
        assert(exports[n].section < nsection);
        assert(exports[n].offset <= sections[n].size);
    }
    return true;
}

static bool parse_cross_references(char *data, size_t size)
{
    uint nxr = get_int(data), n;

    resize(xref, nxref + nxr);
    for (n = nxref - nxr; n < nxref; ++n)
    {
        char *entry = data + 4 + 16*n;
        xrefs[n].patch_section = get_int(entry +  0) + start_section;
        xrefs[n].patch_offset  = get_int(entry +  4);
        xrefs[n].section       = get_int(entry +  8);
        xrefs[n].offset        = get_int(entry + 12);
        if ((int)xrefs[n].section != -1)
        {
            xrefs[n].name = NULL;
            xrefs[n].section += start_section;
            assert(xrefs[n].section < nsection);
        }
        else
        {
            assert(xrefs[n].offset < nimport);
            xrefs[n].name   = imports[xrefs[n].offset];
            xrefs[n].offset = 0;
        }
    }
    return true;
}

static void read_input(const char *path)
{
    FILE *fp;
    char magic[8];
    uint version, minstack;
    char *data;
    size_t size;

    start_section = nsection;

    /* Open file */
    fp = fopen(path, "rb");
    if (fp == NULL) fatal("could not open \"%s\" for reading", path);

    /* Read header, check version */
    if (fread(magic, 1, 8, fp) != 8 || memcmp(magic, "glulxobj", 8) != 0 ||
        (version = read_int(fp)) != 1)
        fatal("file \"%s\" is not a version 1 Glulx object file", path);

    /* Read required stack size */
    minstack = read_int(fp);
    if (minstack > stacksize) stacksize = roundup(minstack, 256);

    /* Read and parse code table */
    if ((data = read_table(fp, &size)) == NULL)
        fatal("could not read code table from file \"%s\"", path);
    if (!parse_code_table(data, size))
        fatal("could not parse code table from file \"%s\"", path);

    /* Read and parse import table */
    if ((data = read_table(fp, &size)) == NULL)
        fatal("could not read import table from file \"%s\"", path);
    if (!parse_import_table(data, size))
        fatal("could not parse import table from file \"%s\"", path);

    /* Read and parse export table */
    if ((data = read_table(fp, &size)) == NULL)
        fatal("could not read export table from file \"%s\"", path);
    if (!parse_export_table(data, size))
        fatal("could not parse export table from file \"%s\"", path);

    /* Read and parse cross-references */
    if ((data = read_table(fp, &size)) == NULL)
        fatal("could not read cross-references from file \"%s\"", path);
    if (!parse_cross_references(data, size))
        fatal("could not parse cross-references from file \"%s\"", path);

    /* Free import table (note: strings themselves are NOT freed here!) */
    free(imports);
    imports = NULL;
    nimport = 0;
}

static int cmp_exports_by_name(const void *a, const void *b)
{
    return strcmp(((struct export *)a)->name, ((struct export *)b)->name);
}

/* Sorts exports by name, and then verifies there are no duplicates. */
static void sort_exports()
{
    uint n;
    qsort(exports, nexport, sizeof(*exports), &cmp_exports_by_name);

    for (n = 1; n < nexport; ++n)
    {
        if (strcmp(exports[n - 1].name, exports[n].name) == 0)
            fatal("symbol \"%s\" defined more than once", exports[n].name);
    }
}

/* Binary-searches for an export with the given name. */
static const struct export *find_export(const char *name)
{
    uint lo = 0, hi = nexport;
    while (lo < hi)
    {
        uint mid = lo + (hi - lo)/2;
        int d = strcmp(exports[mid].name, name);
        if (d < 0)
            lo = mid + 1;
        else
        if (d > 0)
            hi = mid;
        else
            return &exports[mid];
    }
    return NULL;
}

static void resolve_imports()
{
    uint n;
    for (n = 0; n < nxref; ++n)
    {
        if (xrefs[n].name != NULL)
        {
            const struct export *export = find_export(xrefs[n].name);
            if (export == NULL)
                fatal("undefined reference to symbol \"%s\"", xrefs[n].name);
            xrefs[n].section = export->section;
            xrefs[n].offset  = export->offset;
        }
    }
}

static const struct section *find_section_by_type(enum secttype type)
{
    uint n;
    for (n = 0; n < nsection; ++n)
    {
        if (sections[n].type == type)
            return &sections[n];
    }
    return NULL;
}

static void resolve_main()
{
    const struct export *e;

    e = find_export("_start");
    if (e == NULL) e = find_export("main");

    if (e != NULL)
    {
        main_section = e->section;
        main_offset  = e->offset;
        if (sections[main_section].type != SECTION_CODE &&
            sections[main_section].type != SECTION_VCODE)
        {
            fprintf(stderr, "WARNING: application entry point lies"
                            "in a non-code section\n");
        }
    }
    else
    {
        const struct section *s;
        fprintf(stderr, "WARNING: application entry point not defined\n");
        s = find_section_by_type(SECTION_CODE);
        if (s == NULL)
        {
            s = find_section_by_type(SECTION_VCODE);
            if (s == NULL)
                fatal("no code sections in input");
        }
        main_section = s - sections;
        main_offset  = 0;
    }
}

static void resolve_decodingtbl()
{
    const struct export *e = find_export("decodingtbl");
    if (e == NULL)
    {
        decodingtbl_section = (uint)-1;
    }
    else
    {
        decodingtbl_section = e->section;
        decodingtbl_offset  = e->offset;
    }
}

static void mark_used_sections()
{
    uint n;
    bool changed;

    for (n = 0; n < nsection; ++n)
        sections[n].used = false;

    /* Mark main section used */
    sections[main_section].used = true;
    nused = 1;

    /* If we have a decoding table, and it is in a different section, mark it
       as used as well: */
    if (decodingtbl_section != (uint)-1 &&
        decodingtbl_section != main_section)
    {
        sections[decodingtbl_section].used = true;
        nused = 2;
    }

    /* Mark all indirectly reachable sections used: */
    do {
        changed = false;
        for (n = 0; n < nxref; ++n)
        {
            if (sections[xrefs[n].patch_section].used &&
                !sections[xrefs[n].section].used)
            {
                sections[xrefs[n].section].used = true;
                nused += 1;
                changed = true;
            }
        }
    } while(changed);
}

static int cmp_used_section(const void *a, const void *b)
{
    struct section *s = *(struct section **)a,
                   *t = *(struct section **)b;

    if (s->type != t->type) return s->type - t->type;
    if (s < t) return -1;
    if (s > t) return +1;
    return 0;
}

static void order_sections()
{
    uint n, m;

    used = malloc(sizeof(*used)*nused);
    assert(used != NULL);

    m = 0;
    for (n = 0; n < nsection; ++n)
        if (sections[n].used) used[m++] = &sections[n];
    assert(m == nused);

    qsort(used, nused, sizeof(*used), cmp_used_section);
}

static void assign_addresses()
{
    enum secttype type = SECTION_CODE;
    uint address, n;

    address = 36;  /* header size */
    for (n = 0; n < nused; ++n)
    {
        if (type < SECTION_VCODE && used[n]->type >= SECTION_VCODE)
        {
            type = SECTION_VCODE;
            ramstart = address = roundup(address, 256);
        }
        if (type < SECTION_BSS && used[n]->type >= SECTION_BSS)
        {
            type = SECTION_BSS;
            extstart = address = roundup(address, 256);
        }

        switch (used[n]->size)
        {
        case  0: break;
        case  1: break;
        case  2: address = roundup(address, 2); break;  /* short align */
        case  3: break;
        default: address = roundup(address, 4); break;  /* long align */
        }

        used[n]->address = address;
        address += used[n]->size;
    }

    if (type < SECTION_VCODE) ramstart = roundup(address, 256);
    if (type < SECTION_BSS)   extstart = roundup(address, 256);
    endmem = roundup(address, 256);
}

static void fix_references()
{
    unsigned n;
    for (n = 0; n < nxref; ++n)
    {
        struct section *patch = &sections[xrefs[n].patch_section];
        char *data;

        if (!patch->used) continue;
        if (patch->type == SECTION_BSS)
        {
            printf("WARNING: reference in BSS section ignored\n");
            continue;
        }
        assert(sections[xrefs[n].section].used);
        assert(xrefs[n].patch_offset + 4 <= patch->size);

        data = patch->data + xrefs[n].patch_offset;
        set_int(data, get_int(data) +
                      sections[xrefs[n].section].address + xrefs[n].offset);
    }
}

static void create_module()
{
    unsigned n;

    /* Allocate module memory (and initialize to zero) */
    module = malloc(extstart);
    if (module == NULL)
        fatal("could not allocate linked module (%d bytes)", extstart);
    memset(module, 0, extstart);

    /* Create header */
    set_int(module +  0, 0x476C756C);  /* magic: Glul */
    set_int(module +  4, 0x00030101);  /* version: 3.1.1 */
    set_int(module +  8, ramstart);
    set_int(module + 12, extstart);
    set_int(module + 16, endmem);
    set_int(module + 20, stacksize);
    set_int(module + 24, sections[main_section].address + main_offset);
    if (decodingtbl_section != (uint)-1)
    {
        set_int(module + 28,
            sections[decodingtbl_section].address + decodingtbl_offset);
    }

    /* Write in all used sections */
    for (n = 0; n < nused; ++n)
    {
        if (used[n]->type >= SECTION_BSS) continue;
        assert(used[n]->address + used[n]->size <= extstart);
        memcpy(module + used[n]->address, used[n]->data, used[n]->size);
    }
}

static void update_checksum()
{
    unsigned checksum = 0, n;
    for (n = 0; n < extstart; n += 4)
        checksum += get_int(module + n);
    set_int(module + 32, checksum);
}

static void process()
{
    if (nsection == 0) fatal("no sections to process");

    sort_exports();
    resolve_imports();
    resolve_main();
    resolve_decodingtbl();
    mark_used_sections();
    order_sections();
    assign_addresses();
    fix_references();
    create_module();
    update_checksum();
}

static void write_output(const char *path)
{
    FILE *fp = fopen(path, "wb");
    if (fp == NULL) fatal("could not open \"%s\" for writing", path);
    if (fwrite(module, 1, extstart, fp) != extstart)
        fatal("could not write module (%d bytes) to \"%s\"", extstart, path);
    fclose(fp);
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
