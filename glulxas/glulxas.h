#ifndef GLULXAS_H_INCLUDED
#define GLULXAS_H_INCLUDED

/* This file contains definitions used by glulxas.c, and in particular
   those that are shared with parser.y */

#include "operations.h"

enum secttype { SECTION_CODE = 0, SECTION_DATA = 1, SECTION_VCODE = 2,
                SECTION_VDATA = 3, SECTION_BSS = 4, SECTION_INVALID = -1 };

enum opersize { SIZE_BYTE = 1, SIZE_SHORT = 2, SIZE_LONG = 4 };
enum opertype { OPER_CONST, OPER_ROMREF, OPER_RAMREF, OPER_LOCAL, OPER_STACK };

struct labeldef
{
    char *label;
    unsigned section, offset;
};

struct labelref
{
    char *label;
    unsigned section, offset;
};

struct section
{
    enum secttype type;
    unsigned size;
    char *data;
};

struct literal
{
    char *label;  /* if NULL, adjust is a literal value */
    int adjust;
};

struct operand
{
    enum opertype type;
    enum opersize size;
    const struct piece *ref;
    struct literal value;
};

struct instruction
{
    const struct operation *o;
    struct operand opers[8];
};

struct piece
{
    struct instruction *i;
    struct literal *l;

    unsigned offset, size;
    char *data;
};

extern int yyparse();  /* generated from parser.y in parser.tab.c */

/* These functions are called by the parser: */
extern void set_stack_size(unsigned size);
extern void set_ext_size(unsigned size);
extern void begin_section(enum secttype secttype);
extern void def_export(const char *label);
extern void def_label(const char *label);
extern void emit_blank(unsigned count);
extern void emit_data(const struct literal *lit, enum opersize size);
extern void emit_instr(const struct instruction *instr);

#endif /* ndef GLULXAS_H_INCLUDED */
