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

static enum opersize cur_size;
static struct literal cur_lit;
static struct instruction cur_instr;
static int cur_oper_index = 0;
static struct operand cur_oper;
static char *cur_name;

void yyerror(const char *str)
{
    fprintf(stderr, "Parse error on line %d: %s [%s]\n", lineno + 1, str, yytext);
    exit(1);
}

int yywrap()
{
    return 1;
}

static void parse_name()
{
    const char *s = yytext;
    while (*s != '\0' && !isspace(*s)) ++s;
    while (*s != '\0' &&  isspace(*s)) ++s;
    assert(*s != '\0');
    assert(cur_name == NULL);
    cur_name = strdup(s);
}

static void free_name()
{
    free(cur_name);
    cur_name = NULL;
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
}

static void func_header(const char *name, int type, int narg)
{
    split_section();
    def_label(name);
    cur_lit.label  = NULL;
    cur_lit.adjust = type;
    emit_data(&cur_lit, SIZE_BYTE);
    assert(narg >= 0);
    while (narg > 0)
    {
        int n = narg > 255 ? 255 : narg;
        cur_lit.adjust = 0x0400 | n;
        narg -= n;
        emit_data(&cur_lit, SIZE_SHORT);
    }
    cur_lit.adjust = 0;
    emit_data(&cur_lit, SIZE_SHORT);
}

%}

%token ERROR
%token SECTION CODE DATA VCODE VDATA BSS
%token STACK EXT EXPORT IMPORT
%token FUNC_STACK FUNC_LOCAL
%token LABELDEF LABELREF
%token OPCODE
%token DC DS SZB SZS SZL
%token SP DISCARD
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

export_stmt : EXPORT { parse_name(yytext); def_export(cur_name); free_name(); }
            ;

import_stmt : IMPORT { fprintf(stderr, "WARNING: import statement ignored "
                                       "on line %d\n", lineno); }
            ;

func_stmt   : FUNC_STACK { parse_name(); }
              INT { func_header(cur_name, 0xc0, atoi(yytext)); free_name(); }
            | FUNC_LOCAL { parse_name(); }
              INT { func_header(cur_name, 0xc1, atoi(yytext)); free_name(); }
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
        | DISCARD { cur_oper.type = OPER_CONST;
                    cur_oper.value.label = NULL;
                    cur_oper.value.adjust = 0; }
        | SP { cur_oper.type = OPER_STACK; }
        | LPAREN literal { cur_oper.value = cur_lit;
                           cur_oper.type  = OPER_ROMREF; } RPAREN optszpf
        | LBRACK literal { cur_oper.value = cur_lit;
                           cur_oper.type  = OPER_RAMREF; } RBRACK optszpf
        | LBRACE int_literal { cur_oper.value = cur_lit;
                               cur_oper.type  = OPER_LOCAL; } RBRACE optszpf
        ;

optszpf : SZB | SZS | SZL | ;

literal : int_literal
        | LABELREF { parse_labelref(yytext); } optszpf
        ;
int_literal
        : INT { cur_lit.label = NULL; sscanf(yytext, "%i", &cur_lit.adjust); } optszpf
        ;

section_stmt : SECTION CODE  { set_section(SECTION_CODE); }
             | SECTION DATA  { set_section(SECTION_DATA); }
             | SECTION VCODE { set_section(SECTION_VCODE); }
             | SECTION VDATA { set_section(SECTION_VDATA); }
             | SECTION BSS   { set_section(SECTION_BSS); }
             ;
