%{
#include "parser.tab.h"
int lineno = 0;
static int t = 0;     /* number of tokens parsed on this line,
                         not counting label definitions or comments*/
%}
%%
    /* special directives (appear only as the first token on a line) */
section             return ++t == 1 ? SECTION    : NAME;
code                return ++t == 2 ? CODE       : NAME;
data                return ++t == 2 ? DATA       : NAME;
vcode               return ++t == 2 ? VCODE      : NAME;
vdata               return ++t == 2 ? VDATA      : NAME;
bss                 return ++t == 2 ? BSS        : NAME;
stack               return ++t == 1 ? STACK      : NAME;
ext                 return ++t == 1 ? EXT        : NAME;
func_stack          return ++t == 1 ? FUNC_STACK : NAME;
func_local          return ++t == 1 ? FUNC_LOCAL : NAME;
export              return ++t == 1 ? EXPORT     : NAME;
import              return ++t == 1 ? IMPORT     : NAME;
dc                  return ++t == 1 ? DC         : NAME;
ds                  return ++t == 1 ? DS         : NAME;

    /* Label definitions/references.
       Labels can defined only at the start of a line; in that case, they are
       not counted. */
:[^ \n\t\r:;.,()\[\]{}]+  if (t == 0) return LABELDEF; ++t; return LABELREF;
:[^ \n\t\r:;.,()\[\]{}]+(:[+-]?[0-9]+)?  ++t; return LABELREF;

    /* A few unambiguous tokens */
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

    /* Variable tokens */
["][^"]*["]                             ++t; return STRING;
[+-]?(0|[1-9][0-9]*|0x[0-9a-fA-F]+)     return INT;
[a-zA-Z_][a-zA-Z0-9_]*                  return ++t == 1 ? OPCODE : NAME;

    /* Comments/whitespace/line ends */
[ \t\r]+            /* eat whitespace */
;[^\n]*             /* eat comments */
\n                  ++lineno; t = 0; /* start new line */

.                   return ERROR;
%%
