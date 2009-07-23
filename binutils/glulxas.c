#include "glulxas.h"
#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/* Current module settings: */
static uint stack_size;
static uint ext_size;

/* Imported/exported symbols */
static uint nimport;
static char **imports;
static uint nexport;
static char **exports;

/* Assembled sections */
static uint nsection;
static struct section *sections;
static uint nglobaldef;
static struct labeldef *globaldefs;
static uint nglobalref;
static struct labelref *globalrefs;

/* For assembling sections: */
static enum secttype cur_secttype = SECTION_INVALID;
static uint npiece;
static struct piece *pieces;
static uint ndef;
static struct localdef { char *label; uint piece; } *defs;

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

static void warning(const char *fmt, ...)
{
    char buf[1024];
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(buf, sizeof(buf), fmt, ap);
    va_end(ap);
    fprintf(stderr, "WARNING: %s.\n", buf);
}

void set_stack_size(uint size)
{
    if (stack_size != 0) warning("stack size redefined");
    stack_size = size;
}

void set_ext_size(uint size)
{
    if (ext_size != 0) warning("ext size redefined");
    if (ext_size == 0) warning("ext size explicitly defined");
    ext_size = size;
}

uint sint_size(int i)
{
    if (i >=   -128 && i <   128) return 1;
    if (i >= -32768 && i < 32767) return 2;
    return 4;
}

uint uint_size(uint u)
{
    if (u <   256) return 1;
    if (u < 65536) return 2;
    return 4;
}

uint opcode_size(uint opcode)
{
    if (opcode <       0x80) return 1;
    if (opcode <     0x4000) return 2;
    if (opcode < 0x10000000) return 4;
    assert(0);
    return 0;
}

void encode_opcode(uint opcode, void *buf)
{
    uchar *bytes = buf;
    if (opcode < 0x80)
    {
        bytes[0] = opcode;
    }
    else
    if (opcode < 0x4000)
    {
        bytes[0] = ((opcode>>8)&0xff) | 0x80;
        bytes[1] = ((opcode>>0)&0xff);
    }
    else
    if (opcode < 0x0FFFFFFF)
    {
        bytes[0] = ((opcode>>24)&0xff) | 0xC0;
        bytes[1] = ((opcode>>16)&0xff);
        bytes[2] = ((opcode>> 8)&0xff);
        bytes[3] = ((opcode>> 0)&0xff);
    }
    else
    {
        assert(0);
    }
}

/* Returns the minimum size required to encode operand o,
   assuming label references will take the full 32-bit size. */
uint operand_size(const struct operand *o)
{
    if (o->type == OPER_STACK) return 0;
    if (o->value.label != NULL) return 4;
    if (o->type == OPER_CONST)
    {
        return o->value.adjust == 0 ? 0 : sint_size(o->value.adjust);
    }
    else
    {
        /* o->type == OPER_LOCAL/OPER_RAMREF/OPER_ROMREF */
        return uint_size(o->value.adjust);
    }
}

void set_operand_sizes(struct instruction *i)
{
    uint nparam = strlen(i->o->parameters), n;
    for (n = 0; n < nparam; ++n)
    {
        i->opers[n].size = operand_size(&i->opers[n]);
    }
}

uint piece_size(const struct piece *p)
{
    if (p->i != NULL)
    {
        uint res = 0, nparam = strlen(p->i->o->parameters), n;
        res += opcode_size(p->i->o->opcode);
        res += (nparam + 1)/2;  /* operand types */
        for (n = 0; n < nparam; ++n) res += p->i->opers[n].size;
        return res;
    }

    if (p->l != NULL)
    {
        assert(p->size == 1 || p->size == 2 || p->size == 4);
        return p->size;
    }

    return p->size;
}

/* Returns the offset for a relative jump from instruction `p' to instruction
   `q' adding adjustment `adjust'. Assumes sizes and offsets for pieces are
   fixed. */
int branch_address(const struct piece *p, const struct piece *q, int adjust)
{
    return q->offset - (p->offset + p->size) + adjust + 2;
}

/* Resolves a label reference to a section piece, or returns NULL if
   none is found. */
const struct piece *label_to_piece(const char *label)
{
    uint n;
    for (n = 0; n < ndef; ++n)
    {
        if (strcmp(defs[n].label, label) == 0)
        {
            return &pieces[defs[n].piece];
        }
    }
    return NULL;
}

void encode_int(uint value, uint size, void *buf)
{
    uchar *bytes = buf;
    switch (size)
    {
    case 1:
        bytes[0] = value&0xff;
        break;
    case 2:
        bytes[0] = (value>>8)&0xff;
        bytes[1] = (value>>0)&0xff;
        break;
    case 4:
        bytes[0] = (value>>24)&0xff;
        bytes[1] = (value>>16)&0xff;
        bytes[2] = (value>> 8)&0xff;
        bytes[3] = (value>> 0)&0xff;
        break;
    default: assert(0);
    }
}

/* Declare a cross-section label reference */
void ref_label(const char *label, uint offset)
{
    resize(globalref, nglobalref + 1);
    globalrefs[nglobalref - 1].label   = strdup(label);
    globalrefs[nglobalref - 1].section = nsection;
    globalrefs[nglobalref - 1].offset  = offset;
}

/* Declare an exported symbol (which must be declared as a label somewhere) */
void def_export(const char *label)
{
    resize(export, nexport + 1);
    exports[nexport - 1] = strdup(label);
}

/* Define a section-local label */
void def_label(const char *label)
{
    resize(def, ndef + 1);
    defs[ndef - 1].label = strdup(label);
    defs[ndef - 1].piece = npiece;
}

/* Allocates the binary data representation of a piece. */
void set_piece_data(struct piece *p)
{
    if (p->data != NULL) return;
    if (cur_secttype == SECTION_BSS) return;
    p->data = malloc(p->size);
    assert(p->data != NULL);
    if (p->i != NULL)
    {
        uint nparam = strlen(p->i->o->parameters), n;
        char *pos = p->data;

        encode_opcode(p->i->o->opcode, pos);
        pos += opcode_size(p->i->o->opcode);

        /* Encode operand types */
        memset(pos, 0, (nparam + 1)/2);
        for (n = 0; n < nparam; ++n)
        {
            const struct operand *oper = &p->i->opers[n];
            int type = -1;
            switch (oper->type)
            {
            case OPER_CONST:
                switch (oper->size)
                {
                case 0: type = 0x00; break;
                case 1: type = 0x01; break;
                case 2: type = 0x02; break;
                case 4: type = 0x03; break;
                }
                break;

                /* 0x04 unused */

            case OPER_ROMREF:
                switch (oper->size)
                {
                case 0: assert(0); break;
                case 1: type = 0x05; break;
                case 2: type = 0x06; break;
                case 4: type = 0x07; break;
                }
                break;

            case OPER_STACK:
                type = 0x08;
                break;

            case OPER_LOCAL:
                switch (oper->size)
                {
                case 0: assert(0); break;
                case 1: type = 0x09; break;
                case 2: type = 0x0A; break;
                case 4: type = 0x0B; break;
                }
                break;

                /* 0x0C unused */

            case OPER_RAMREF:
                 /* not supported right now, because we need an adjustment equal
                    to ramstart which is not fixed! */
                assert(0);
                break;
            }
            assert(type != -1);
            pos[n/2] |= type << (n%2 ? 4 : 0);
        }
        pos += (nparam + 1)/2;

        /* Encode operand values */
        for (n = 0; n < nparam; ++n)
        {
            const struct operand *oper = &p->i->opers[n];
            if (oper->ref != NULL)
            {
                /* This is an IP-relative branch offset */
                encode_int(branch_address(p, oper->ref, oper->value.adjust),
                           oper->size, pos);
            }
            else
            {
                switch (oper->type)
                {
                case OPER_STACK:
                    break;

                case OPER_CONST:
                    if (oper->size == 0) break;
                    /* falls through */
                case OPER_LOCAL:
                case OPER_ROMREF:
                    encode_int(oper->value.adjust, oper->size, pos);
                    if (oper->value.label != NULL)
                    {
                        assert(oper->size == 4);
                        ref_label(oper->value.label, p->offset + (pos - p->data));
                    }
                    break;

                case OPER_RAMREF:
                default:
                    assert(0);
                    break;
                }
            }
            pos += oper->size;
        }
        assert(pos == p->data + p->size);
        return;
    }

    if (p->l != NULL)
    {
        encode_int(p->l->adjust, p->size, p->data);
        if (p->l->label != NULL)
        {
            assert(p->size == 4);
            ref_label(p->l->label, p->offset);
        }
        return;
    }

    memset(p->data, 0, p->size);
    return;
}

void end_section()
{
    struct section *section;
    uint n, offset;

    if (cur_secttype == SECTION_INVALID) fatal("no section declared");

    /* Pass 1: find (worst-case) sizes and initial offsets of instructions  */
    offset = 0;
    for (n = 0; n < npiece; ++n)
    {
        if (pieces[n].i != NULL) set_operand_sizes(pieces[n].i);
        pieces[n].offset = offset;
        pieces[n].size   = piece_size(&pieces[n]);
        offset += pieces[n].size;
    }

    /* Pass 2: resolve relative addresses (used in branches) and update
               the size of applicable operands */
    for (n = 0; n < npiece; ++n)
    {
        if (pieces[n].i != NULL)
        {
            const char *params = pieces[n].i->o->parameters;
            uint p, nparam = strlen(params);
            for (p = 0; p < nparam; ++p)
            {
                struct operand *oper = &pieces[n].i->opers[p];
                oper->ref = NULL;
                if (params[p] != 'b') continue;
                if (oper->type != OPER_CONST)
                {
                    warning("non-constant branch operand");
                    continue;
                }
                if (oper->value.label == NULL)
                {
                    warning("fixed branch operand specified");
                    continue;
                }
                oper->ref = label_to_piece(oper->value.label);
                if (oper->ref == NULL) fatal("couldn't resolve branch operand");

                oper->size = sint_size(branch_address(
                    &pieces[n], oper->ref, oper->value.adjust ));
            }
        }
    }

    /* Pass 3: update to final offsets and sizes */
    offset = 0;
    for (n = 0; n < npiece; ++n)
    {
        pieces[n].offset = offset;
        pieces[n].size   = piece_size(&pieces[n]);
        offset += pieces[n].size;
    }

    /* Pass 4: sizes, offsets and operand sizes are known; encode data */
    for (n = 0; n < npiece; ++n)
        set_piece_data(&pieces[n]);

    /* Move local label definitions over to global list */
    resize(globaldef, nglobaldef + ndef);
    for (n = 0; n < ndef; ++n)
    {
        struct labeldef *def = &globaldefs[nglobaldef - ndef + n];
        def->label   = defs[n].label;
        def->section = nsection;
        def->offset  = pieces[defs[n].piece].offset;
    }
    ndef = 0;
    defs = NULL;

    /* Create new section */
    resize(section, nsection + 1);
    section = &sections[nsection - 1];
    section->type = cur_secttype;
    section->size = offset;
    if (section->type == SECTION_BSS)
    {
        section->data = NULL;
    }
    else
    {
        section->data = malloc(offset);
        memset(section->data, 0, section->size);
        assert(section->data != 0);
        for (n = 0; n < npiece; ++n)
        {
            memcpy(section->data + pieces[n].offset,
                   pieces[n].data, pieces[n].size);
        }
    }

    /* Clear for next section */
    cur_secttype = SECTION_INVALID;
    resize(piece, 0);
}

void begin_section(enum secttype secttype)
{
    if (npiece != 0) end_section();
    cur_secttype = secttype;
}

static void *dup_data(const void *data, size_t size)
{
    void *res;

    if (data == NULL) return NULL;

    res = malloc(size);
    assert(res != NULL);
    memcpy(res, data, size);
    return res;
}

struct instruction *dup_instruction(const struct instruction *i)
{
    return dup_data(i, sizeof(*i));
}

struct literal *dup_literal(const struct literal *l)
{
    return dup_data(l, sizeof(*l));
}

static void add_piece( const struct instruction *i,
                       const struct literal *l,
                       uint size, char *data )
{
    resize(piece, npiece + 1);
    pieces[npiece - 1].i      = dup_instruction(i);
    pieces[npiece - 1].l      = dup_literal(l);
    pieces[npiece - 1].offset = (uint)-1;
    pieces[npiece - 1].size   = size;
    pieces[npiece - 1].data   = dup_data(data, size);
}

void emit_blank(uint count)
{
    add_piece(NULL, NULL, count, NULL);
}

void emit_data(const struct literal *lit, enum opersize size)
{
    add_piece(NULL, lit, (uint)size, NULL);
}

void emit_instr(const struct instruction *instr)
{
    add_piece(instr, NULL, (uint)-1, NULL);
}

static void write_int(FILE *fp, uint value)
{
    fputc((value>>24)&0xff, fp);
    fputc((value>>16)&0xff, fp);
    fputc((value>> 8)&0xff, fp);
    fputc((value>> 0)&0xff, fp);
}

static uint roundup(uint n, uint align)
{
    uint m = n%align;
    if (m != 0) n += align - m;
    return n;
}

static void write_padding(FILE *fp, uint n, uint align)
{
    uint m = roundup(n, align);
    for (; n < m; ++n) fputc(0, fp);
}

static void write_code_table(FILE *fp)
{
    uint total_size = 4, n;
    for (n = 0; n < nsection; ++n)
    {
        total_size += 12;
        if (sections[n].type != SECTION_BSS)
            total_size += roundup(sections[n].size, 4);
    }
    write_int(fp, total_size);
    write_int(fp, nsection);
    for (n = 0; n < nsection; ++n)
    {
        write_int(fp, sections[n].type);
        write_int(fp, sections[n].size);
        write_int(fp, 0);
        if (sections[n].type != SECTION_BSS)
        {
            fwrite(sections[n].data, 1, sections[n].size, fp);
            write_padding(fp, sections[n].size, 4);
        }
    }
}

/* Create the import table, assuming globalrefs and globaldefs are sorted by
   label name. Every globalref that doesn't have a globaldef with a
   corresponding label is turned into an import (except that every name is
   imported only once). */
static void create_imports()
{
    uint r, d;  /* current globalref & globaldef*/
    int diff;

    assert(nimport == 0);
    d = 0;
    for (r = 0; r < nglobalref; ++r)
    {
        /* Skip duplicate names */
        if (r > 0 && strcmp(globalrefs[r].label, globalrefs[r - 1].label) == 0)
            continue;

        /* Find a matching definition */
        diff = +1;
        for ( ; d < nglobaldef; ++d)
        {
            diff = strcmp(globaldefs[d].label, globalrefs[r].label);
            if (diff >= 0) break;
        }
        if (diff == 0) continue;

        /* Add import: */
        resize(import, nimport + 1);
        imports[nimport - 1] = globalrefs[r].label;
    }

}

static void write_import_table(FILE *fp)
{
    uint total_size = 4, n, str_offset;
    for (n = 0; n < nimport; ++n)
        total_size += 4 + strlen(imports[n]) + 1;
    write_int(fp, roundup(total_size, 4));
    write_int(fp, nimport);
    str_offset = 4 + 4*nimport;
    for (n = 0; n < nimport; ++n)
    {
        write_int(fp, str_offset);
        str_offset += strlen(imports[n]) + 1;
    }
    for (n = 0; n < nimport; ++n)
    {
        fwrite(imports[n], 1, strlen(imports[n]), fp);
        fputc(0, fp);
    }
    write_padding(fp, total_size, 4);
}

static void write_export_table(FILE *fp)
{
    uint total_size = 4, n, str_offset, d = 0;
    for (n = 0; n < nexport; ++n)
        total_size += 12 + strlen(exports[n]) + 1;
    write_int(fp, roundup(total_size, 4));
    write_int(fp, nexport);
    str_offset = 4 + 12*nexport;
    for (n = 0; n < nexport; ++n)
    {
        /* Find matching section */
        for ( ; d < nglobaldef; ++d)
        {
            if (strcmp(exports[n], globaldefs[d].label) == 0) break;
        }
        if (d == nglobaldef)
            fatal("no label corresponding to exported symbol \"%s\"", exports[n]);

        write_int(fp, str_offset);
        str_offset += strlen(exports[n]) + 1;
        write_int(fp, globaldefs[d].section);
        write_int(fp, globaldefs[d].offset);
    }
    for (n = 0; n < nexport; ++n)
    {
        fwrite(exports[n], 1, strlen(exports[n]), fp);
        fputc(0, fp);
    }
    write_padding(fp, total_size, 4);
}

static void write_xref_table(FILE *fp)
{
    uint r, d, i;  /* current globalref, globaldef and import */
    write_int(fp, 4 + 16*nglobalref);
    write_int(fp, nglobalref);

    d = i = 0;
    for (r = 0; r < nglobalref; ++r)
    {
        const struct labelref *ref = &globalrefs[r];

        write_int(fp, ref->section);
        write_int(fp, ref->offset);

        /* Find a matching definition */
        for ( ; d < nglobaldef; ++d)
        {
            int diff = strcmp(globaldefs[d].label, ref->label);
            if (diff < 0) continue;
            if (diff > 0) break;
            write_int(fp, globaldefs[d].section);
            write_int(fp, globaldefs[d].offset);
            goto next;
        }

        /* No matching definition; find a matching import instead */
        for ( ; i < nimport; ++i)
        {
            int diff = strcmp(imports[i], ref->label);
            if (diff < 0) continue;
            if (diff > 0) break;
            write_int(fp, -1);
            write_int(fp, i);
            goto next;
        }

        /* No matching import found either! This can't happen. */
        assert(0);

    next: continue;
    }
}

static int cmp_exports(const void *a, const void *b)
{
    return strcmp(*(const char**)a, *(const char **)b);
}

static void sort_exports()
{
    uint i, j;

    /* Sort by label name: */
    qsort(exports, nexport, sizeof(char*), cmp_exports);

    /* Remove duplicates. */
    for (i = j = 0; j < nexport; ++j)
    {
        if (i == 0 || strcmp(exports[j], exports[i - 1]) != 0)
            exports[i++] = exports[j];
        else
            free(exports[j]);
    }
    resize(export, j);
}

static int cmp_globaldef(const void *a, const void *b)
{
    return strcmp(((struct labeldef*)a)->label, ((struct labeldef*)b)->label);
}

static void sort_globaldefs()
{
    uint n;

    /* Sort by label name: */
    qsort(globaldefs, nglobaldef, sizeof(struct labeldef), cmp_globaldef);

    /* Check for duplicates */
    for (n = 1; n < nglobaldef; ++n)
    {
        if (strcmp(globaldefs[n].label, globaldefs[n - 1].label) == 0)
            fatal("label \"%s\" defined more than once", globaldefs[n].label);
    }
}

static int cmp_globalref(const void *a, const void *b)
{
    return strcmp(((struct labelref*)a)->label, ((struct labelref*)b)->label);
}

static void sort_globalrefs()
{
    /* Sort by label name: */
    qsort(globalrefs, nglobalref, sizeof(struct labelref), cmp_globalref);
}

static void write_object_file(FILE *fp)
{
    fwrite("glulxobj", 1, 8, fp);
    write_int(fp, 1);  /* version */
    write_int(fp, stack_size);
    write_code_table(fp);
    write_import_table(fp);
    write_export_table(fp);
    write_xref_table(fp);
}

static void write_object_path(const char *path)
{
    FILE *fp = fopen(path, "wb");
    if (fp == NULL) fatal("couldn't open \"%s\" for writing", path);
    write_object_file(fp);
    fclose(fp);
}

int main(int argc, char *argv[])
{
    if (argc > 3)
    {
        printf("usage: glulxas [<input> [<output>]]\n");
        return 1;
    }

    if (argc >= 2)
    {
        extern FILE *yyin;
        yyin = fopen(argv[1], "rt");
        if (yyin == NULL) fatal("couldn't open \"%s\" for reading", argv[1]);
    }

    /* Parse input */
    (void)yyparse();
    if (npiece != 0) end_section();

    /* Process parsed data */
    sort_exports();
    sort_globaldefs();
    sort_globalrefs();
    create_imports();

    /* Write output */
    if (argc < 2)
    {
        write_object_file(stdout);
    }
    else
    if (argc == 2)
    {
        char *in = argv[1];
        char *out, *p;

        /* set `in' to file component of input path: */
        p = strrchr(in, '/');
        in = (p == NULL) ? in : p + 1;

        /* set `p' to last period in filename (or end of string) */
        p = strrchr(in, '.');
        if (p == NULL) p = in + strlen(in);

        /* create output path: */
        out = malloc(p - in + 5);
        assert(out != NULL);
        memcpy(out, in, p - in);
        memcpy(out + (p - in), ".ulo", 5);
        write_object_path(out);
    }
    else /* argc >= 3 */
    {
        write_object_path(argv[2]);
    }

    return 0;
}
