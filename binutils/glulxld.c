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

struct searchpath
{
    struct searchpath *next;
    const char *directory;
};

struct section
{
    /* read from the object file: */
    enum secttype type;
    size_t size;
    uint align;
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
static uint stacksize = 256;
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

#define fourcc(a,b,c,d) (((a)<<24)|((b)<<16)|((c)<<8)|(d))

/* Reads an IFF chunk and returns its type. */
static uint read_chunk(FILE *fp, char **data_out, size_t *size_out)
{
    char buf[8], *data;
    uint type, size, readsize;

    *data_out = NULL;
    *size_out = 0;
    if (fread(buf, 8, 1, fp) != 1) return 0;
    type = get_int(buf + 0);
    size = get_int(buf + 4);
    if (type == 0) return 0;
    readsize = roundup(size, 2);
    if ((data = malloc(readsize)) == NULL) return 0;
    if (fread(data, 1, readsize, fp) != readsize)
    {
        free(data);
        return 0;
    }
    *data_out = data;
    *size_out = size;
    return type;
}

/* Parses an IFF chunk from a memory buffer and returns its type. */
static uint parse_chunk(char *data_in, size_t size_in,
                        char **data_out, size_t *size_out)
{
    uint type, size;
    if (size_in < 8) return 0;
    type = get_int(data_in + 0);
    size = get_int(data_in + 4);
    if (size_in - 8 < size) return 0;
    *data_out = data_in + 8;
    *size_out = size;
    return type;
}

/* Updates data and size to point past the current chunk */
static void skip_chunk(char **data, size_t *size)
{
    uint chunk_size;
    assert(*size >= 8);
    chunk_size = get_int(*data + 4);
    if (chunk_size%2 == 1) chunk_size += 1;
    assert(chunk_size <= *size - 8);
    *data += 8 + chunk_size;
    *size -= 8 + chunk_size;
}

static bool parse_header(char *data, size_t size)
{
    uint version, minstack;
    if (size < 8) return false;
    version  = get_int(data + 0);
    minstack = get_int(data + 4);
    if ((version&0xffff0000) != 0x00010000)
        return false;
    if (minstack > stacksize)
        stacksize = roundup(minstack, 256);
    return true;
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
        sections[n].type  = get_int(data + 0);
        sections[n].size  = get_int(data + 4);
        sections[n].align = get_int(data + 8)&0xffff;
        data += 12;
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
    char *entry;
    uint nexp = get_int(data), n;
    resize(export, nexport + nexp);
    entry = data + 4;
    for (n = nexport - nexp; n < nexport; ++n)
    {
        exports[n].name    = get_int(entry + 0) + data;
        exports[n].section = get_int(entry + 4) + start_section;
        exports[n].offset  = get_int(entry + 8);
        assert(exports[n].section < nsection);
        assert(exports[n].offset <= sections[exports[n].section].size);
        entry += 12;
    }
    return true;
}

static bool parse_xref_table(char *data, size_t size)
{
    char *entry;
    uint nxr = get_int(data), n;

    resize(xref, nxref + nxr);
    entry = data + 4;
    for (n = nxref - nxr; n < nxref; ++n)
    {
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
        entry += 16;
    }
    return true;
}

static void read_input(const char *path)
{
    FILE *fp;
    uint num_objects, type;
    char *data, *chunk_data;
    size_t size, chunk_size;

    /* Open file */
    fp = fopen(path, "rb");
    if (fp == NULL) fatal("could not open \"%s\" for reading", path);

    num_objects = 0;
    while ((type = read_chunk(fp, &data, &size)) != 0)
    {
        /* Recognize Glulx object chunks */
        if (type != fourcc('F','O','R','M') || size < 4 ||
            get_int(data) != fourcc('G','L','U','O'))
        {
            fprintf(stderr, "WARNING: skipping unexpected chunk "
                            "in \"%s\"\n", path);
            free(data);
            continue;
        }

        /* Initialize */
        start_section = nsection;
        data += 4;
        size -= 4;

        /* Parse chunks in this FORM chunk */
        while ((type = parse_chunk(data, size, &chunk_data, &chunk_size)) != 0)
        {
            if (type == fourcc('H','E','A','D'))
            {
                if (!parse_header(chunk_data, chunk_size))
                {
                    fprintf(stderr, "WARNING: invalid Glulx header chunk in "
                                    "\"%s\"\n", path);
                    break;
                }
            }
            else
            {
                bool parsed = false;
                switch (type)
                {
                case fourcc('C','O','D','E'):
                    parsed = parse_code_table(chunk_data, chunk_size);
                    break;
                case fourcc('I','M','P','O'):
                    parsed = parse_import_table(chunk_data, chunk_size);
                    break;
                case fourcc('E','X','P','O'):
                    parsed = parse_export_table(chunk_data, chunk_size);
                    break;
                case fourcc('X','R','E','F'):
                    parsed = parse_xref_table(chunk_data, chunk_size);
                    break;
                }
                if (!parsed)
                {
                    fprintf(stderr,
                            "WARNING: couldn't parse chunk in \"%s\"\n", path);
                }
            }
            skip_chunk(&data, &size);
        }

        if (size > 0)
        {
            fprintf(stderr,
                    "WARNING: could not completely parse \"%s\"\n", path);
        }

        /* Free import table (note: strings themselves are NOT freed here!) */
        free(imports);
        imports = NULL;
        nimport = 0;
        num_objects++;
    }

    if (num_objects == 0)
        fprintf(stderr, "WARNING: no objects read from \"%s\"\n", path);
}


/* Returns a library filename of the form "${dir}/lib{$name}.ulo". */
static char *libfilename(const char *dir, const char *name)
{
    char *buf = malloc(strlen(dir) + 4 + strlen(name) + 4 + 1);
    assert(buf != NULL);
    sprintf(buf, "%s/lib%s.ulo", dir, name);
    return buf;
}

/* Adds an entry at the end of the search path list, WITHOUT copying the path
   argument (so it should remain valid). */
static void add_search_path(struct searchpath **paths, const char *path)
{
    struct searchpath *entry = malloc(sizeof(*entry));
    assert(entry != NULL);
    entry->next      = NULL;
    entry->directory = path;
    while (*paths != NULL) paths = &(*paths)->next;
    *paths = entry;
}

static bool can_open(const char *path)
{
    FILE *fp = fopen(path, "rb");
    if (fp == NULL) return false;
    fclose(fp);
    return true;
}

static void read_library(const struct searchpath *paths, const char *name)
{
    for ( ; paths != NULL; paths = paths->next)
    {
        char *path = libfilename(paths->directory, name);
        if (can_open(path))
        {
            read_input(path);
            return;
        }
        free(path);
    }
    fatal("could not find library named \"%s\"", name);
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
    if (e == NULL)
    {
        e = find_export("main");
        if (e != NULL)
        {
            fprintf(stderr, "WARNING: symbol \"_start\" not defined; using "
                            "\"main\" instead.\n");
        }
    }

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

        if (used[n]->align > 1)
            address = roundup(address, used[n]->align);

        used[n]->address = address;
        address += used[n]->size;
    }

    if (type < SECTION_VCODE) ramstart = roundup(address, 256);
    if (type < SECTION_BSS)   extstart = roundup(address, 256);
    endmem = roundup(address, 256);
}

static void fix_references()
{
    uint n;
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
    uint n;

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
    uint checksum = 0, n;
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
    int i;
    bool parseopts = true;
    const char *out = "a.ulx";
    struct searchpath *searchpaths = NULL;

    if (argc <= 1)
    {
        printf("usage: glulxld [-o module] object ...\n");
        return 0;
    }

    for (i = 1; i < argc; ++i)
    {
        if (parseopts && strcmp(argv[i], "--") == 0)
        {
            /* End of options */
            parseopts = false;
            continue;
        }

        if (parseopts && argv[i][0] == '-')
        {
            /* All supported options take an argument: */
            const char *opt = argv[i], *arg = &argv[i][2];
            if (*arg == '\0' && i < argc) arg = argv[++i];

            switch (opt[1])
            {
            case 'o':
                out = arg;
                break;

            case 'L':
                add_search_path(&searchpaths, arg);
                break;

            case 'l':
                read_library(searchpaths, arg);
                break;

            default:
                fatal("unrecognized option: %s\n", opt);
                break;
            }
        }
        else
        {
            /* Non-option argument: */
            read_input(argv[i]);
        }
    }

    process();
    write_output(out);
    return 0;
}
