%{
#include "glulxas.h"
#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern int lineno;
extern char *yytext;
int yylex(void);

struct named_local {
    struct named_local *next;
    char *name;
    int index;
};

static enum opersize cur_size;
static struct literal cur_lit;
static struct instruction cur_instr;
static int cur_oper_index = 0;
static struct operand cur_oper;
static int cur_func_type;
static char *cur_func_name;
static int cur_nlocal;
static struct named_local *named_locals;

void yyerror(const char *str)
{
    fprintf(stderr, "Parse error on line %d: %s [%s]\n", lineno + 1, str, yytext);
    exit(1);
}

int yywrap()
{
    return 1;
}

static char *parse_labeldef(const char *s)
{
    char *p = strchr(s, ':');
    assert(p != NULL);
    return strdup(p + 1);
}

static void parse_labelref(const char *s)
{
    char *p = strchr(s, ':');
    assert(p != NULL);
    cur_lit.label = strdup(p + 1);
    if ((p = strchr(cur_lit.label, ':')) != NULL)
    {
        *p = 0;
        cur_lit.adjust = atoi(p + 1);
    }
    else
    {
        cur_lit.adjust = 0;
    }
}

static void begin_instr(const char *opname)
{
    cur_instr.o = operation_by_name(opname);
    if (cur_instr.o == NULL)
    {
        fprintf(stderr, "Invalid mnemonic '%s' on line %d!\n",
            opname, lineno + 1);
        exit(1);
    }
    cur_oper_index = 0;
}

static void end_instr()
{
    if (cur_oper_index < strlen(cur_instr.o->parameters))
    {
        fprintf(stderr, "Too few operands to %s instruction on line %d!\n",
            cur_instr.o->mnemonic, lineno + 1);
        exit(1);
    }
    emit_instr(&cur_instr);
}

static void add_operand()
{
    if (cur_oper_index == strlen(cur_instr.o->parameters))
    {
        fprintf(stderr, "Too many operands to %s instruction on line %d!\n",
            cur_instr.o->mnemonic, lineno + 1);
        exit(1);
    }
    assert(cur_oper_index < sizeof(cur_instr.opers)/sizeof(cur_instr.opers[0]));
    cur_instr.opers[cur_oper_index] = cur_oper;
    ++cur_oper_index;
}

static void emit_string(const char *str)
{
    size_t i, n = strlen(str);
    assert(str[0] == '"' && str[n - 1] == '"');
    cur_lit.label  = NULL;
    cur_lit.adjust = 0xe0;
    emit_data(&cur_lit, SIZE_BYTE);
    for (i = 1; i < n - 1; ++i)
    {
        if (str[i] == '\\')
        {
            switch (str[++i])
            {
            case '\\': cur_lit.adjust = '\\'; break;
            case 'n':  cur_lit.adjust = '\n'; break;
            default:   cur_lit.adjust = str[i]; break;
            }
        }
        else
        {
            cur_lit.adjust = str[i];
        }
        emit_data(&cur_lit, SIZE_BYTE);
    }
    cur_lit.adjust = 0;
    emit_data(&cur_lit, SIZE_BYTE);
}

static void func_header(const char *name, int type, int nlocal)
{
    split_section();
    def_label(name);
    cur_lit.label  = NULL;
    cur_lit.adjust = type;
    emit_data(&cur_lit, SIZE_BYTE);
    assert(nlocal >= 0);
    while (nlocal > 0)
    {
        int n = nlocal > 255 ? 255 : nlocal;
        cur_lit.adjust = 0x0400 | n;
        nlocal -= n;
        emit_data(&cur_lit, SIZE_SHORT);
    }
    cur_lit.adjust = 0;
    emit_data(&cur_lit, SIZE_SHORT);
}

static void free_named_locals(struct named_local *nl)
{
    if (nl == NULL) return;
    free_named_locals(nl->next);
    free(nl->name);
    free(nl);
}

static void free_locals()
{
    free_named_locals(named_locals);
    named_locals = NULL;
    cur_nlocal = 0;
}

static void set_unnamed_locals(int n)
{
    cur_nlocal = n;
}

static const struct named_local *find_named_local(const char *name)
{
    const struct named_local *res;
    for (res = named_locals; res != NULL; res = res->next)
        if (strcmp(res->name, name) == 0) break;
    return res;
}

static void add_named_local(const char *name, int index)
{
    if (find_named_local(name) != NULL)
    {
        fprintf(stderr, "Local variable `%s' redefined on line %d\n",
                        name, lineno);
        exit(1);
    }
    else
    {
        struct named_local *nl= malloc(sizeof(struct named_local));
        assert(nl != NULL);
        nl->name = strdup(name);
        assert(nl->name != NULL);
        nl->next = named_locals;
        nl->index = index;
        named_locals = nl;

        if (index >= cur_nlocal) cur_nlocal = index + 1;
    }
}

/* Set current operand to the offset of parameter named `name'. */
static void local_ref(const char *name)
{
    const struct named_local *nl;
    if ((nl = find_named_local(name)) != NULL)
    {
        cur_oper.value.label  = NULL;
        cur_oper.value.adjust = 4*nl->index;
        cur_oper.type = OPER_LOCAL;
        return;
    }
    fprintf(stderr, "Undeclared reference to `%s' on line %d\n", name, lineno);
    exit(1);
}

%}

%token ERROR
%token SECTION CODE DATA VCODE VDATA BSS
%token STACK EXT EXPORT IMPORT
%token FUNC_STACK FUNC_LOCAL
%token LABELDEF LABELREF
%token OPCODE NAME
%token DC DS SZB SZS SZL
%token SP TILDE
%token INT STRING
%token LBRACE RBRACE
%token LPAREN RPAREN
%token LBRACK RBRACK

%%

start : statements;

statements : statements statement
           | ;

statement : section_stmt
          | export_stmt
          | import_stmt
          | func_stmt
          | optlabeldefs data_stmt
          | optlabeldefs code_stmt
          | STACK INT { set_stack_size(atoi(yytext)); }
          | EXT INT   { set_ext_size(atoi(yytext)); }
          ;

export_stmt : EXPORT NAME { def_export(yytext); }
            ;

import_stmt : IMPORT NAME { fprintf(stderr, "WARNING: import statement ignored "
                                            "on line %d\n", lineno); }
            ;

func_stmt   : func_type { free_locals(); }
              NAME { cur_func_name = strdup(yytext); }
              func_params
              { func_header(cur_func_name, cur_func_type, cur_nlocal);
                free(cur_func_name); }
            ;

func_params : unnamed_locals | named_locals;

unnamed_locals : INT { set_unnamed_locals(atoi(yytext)); }
               ;

named_locals : named_locals NAME { add_named_local(yytext, cur_nlocal); }
             | named_locals TILDE NAME
               { add_named_local(yytext, cur_nlocal - 1); }
             |
             ;

func_type   : FUNC_STACK { cur_func_type = 0xc0; }
            | FUNC_LOCAL { cur_func_type = 0xc1; }
            ;

optlabeldefs : optlabeldefs LABELDEF
               { char *l = parse_labeldef(yytext); def_label(l); free(l); }
             | ;

data_stmt : DC data_size data_literals
          | DS data_size INT { emit_blank((uint)cur_size*atoi(yytext)); }
          ;

data_literals : data_literals data_literal
              | ;

data_literal : literal { emit_data(&cur_lit, cur_size); }
             | STRING { emit_string(yytext); };

data_size : SZB { cur_size = SIZE_BYTE;  }
          | SZS { cur_size = SIZE_SHORT; }
          | SZL { cur_size = SIZE_LONG;  }
          ;

code_stmt : OPCODE { begin_instr(yytext); } operands { end_instr(); }
          ;

operands : operands operand { add_operand(); }
         | ;

operand : literal { cur_oper.type = OPER_CONST;
                    cur_oper.value = cur_lit;  }
        | TILDE { cur_oper.type = OPER_CONST;
                  cur_oper.value.label = NULL;
                  cur_oper.value.adjust = 0; }
        | SP { cur_oper.type = OPER_STACK; }
        | LPAREN literal { cur_oper.value = cur_lit;
                           cur_oper.type  = OPER_ROMREF; } RPAREN optszpf
        | LBRACK literal { cur_oper.value = cur_lit;
                           cur_oper.type  = OPER_RAMREF; } RBRACK optszpf
        | LBRACE int_literal { cur_oper.value = cur_lit;
                               cur_oper.type  = OPER_LOCAL; } RBRACE optszpf
        | NAME { local_ref(yytext); }
        ;

optszpf : SZB | SZS | SZL | ;

literal : int_literal
        | LABELREF { parse_labelref(yytext); } optszpf
        ;
int_literal
        : INT { cur_lit.label = NULL; sscanf(yytext, "%i", &cur_lit.adjust); }
          optszpf
        ;

section_stmt : SECTION CODE  { set_section(SECTION_CODE); }
             | SECTION DATA  { set_section(SECTION_DATA); }
             | SECTION VCODE { set_section(SECTION_VCODE); }
             | SECTION VDATA { set_section(SECTION_VDATA); }
             | SECTION BSS   { set_section(SECTION_BSS); }
             ;
