%{
#include "parser.tab.h"
int lineno = 0;
%}
%%
section             return SECTION;
code                return CODE;
data                return DATA;
vcode               return VCODE;
vdata               return VDATA;
bss                 return BSS;
stack               return STACK;
ext                 return EXT;

func_stack[ \t\r]+[^ \n\t\r:;.,()\[\]{}]+   return FUNC_STACK;
func_local[ \t\r]+[^ \n\t\r:;.,()\[\]{}]+   return FUNC_LOCAL;
export[ \t\r]+[^ \n\t\r:;.,()\[\]{}]+       return EXPORT;
import[ \t\r]+[^ \n\t\r:;.,()\[\]{}]+       return IMPORT;
^[ \t\r]*:[^ \n\t\r:;.,()\[\]{}]+           return LABELDEF;
:[^ \n\t\r:;.,()\[\]{}]+(:[+-]?[0-9]+)?     return LABELREF;

dc                  return DC;
ds                  return DS;
[.]b                return SZB;
[.]s                return SZS;
[.]l                return SZL;

[(]sp[)]            return SP;
[~]                 return DISCARD;

[{]                 return LBRACE;
[}]                 return RBRACE;
[(]                 return LPAREN;
[)]                 return RPAREN;
[[]                 return LBRACK;
[]]                 return RBRACK;

[+-]?(0|[1-9][0-9]*|0x[0-9a-fA-F]+) return INT;
[a-z]+              return OPCODE;

["][^"]*["]         return STRING;

[ \t\r]+            /* eat whitespace */
;[^\n]*             /* eat comments */
\n                  ++lineno;

.                   return ERROR;
%%
