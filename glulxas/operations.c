#include "operations.h"
#include <stdlib.h>
#include <string.h>

const int noperation = 87;
const struct operation operations[87] = {
    /* num    mnemonic          parameters */
    {  0x00, "nop",             ""},
    {  0x10, "add",             "lls"},
    {  0x11, "sub",             "lls"},
    {  0x12, "mul",             "lls"},
    {  0x13, "div",             "lls"},
    {  0x14, "mod",             "lls"},
    {  0x15, "neg",             "ls"},
    {  0x18, "bitand",          "lls"},
    {  0x19, "bitor",           "lls"},
    {  0x1A, "bitxor",          "lls"},
    {  0x1B, "bitnot",          "ls"},
    {  0x1C, "shiftl",          "lls"},
    {  0x1D, "sshiftr",         "lls"},
    {  0x1E, "ushiftr",         "lls"},
    {  0x20, "jump",            "b"},
    {  0x22, "jz",              "lb"},
    {  0x23, "jnz",             "lb"},
    {  0x24, "jeq",             "llb"},
    {  0x25, "jne",             "llb"},
    {  0x26, "jlt",             "llb"},
    {  0x27, "jge",             "llb"},
    {  0x28, "jgt",             "llb"},
    {  0x29, "jle",             "llb"},
    {  0x2A, "jltu",            "llb"},
    {  0x2B, "jgeu",            "llb"},
    {  0x2C, "jgtu",            "llb"},
    {  0x2D, "jleu",            "llb"},
    {  0x30, "call",            "fls"},
    {  0x31, "return",          "l"},
    {  0x32, "catch",           "sb"},
    {  0x33, "throw",           "ll"},
    {  0x34, "tailcall",        "fl"},
    {  0x40, "copy",            "ls"},
    {  0x41, "copys",           "ls"},
    {  0x42, "copyb",           "ls"},
    {  0x44, "sexs",            "ls"},
    {  0x45, "sexb",            "ls"},
    {  0x48, "aload",           "mls"},
    {  0x49, "aloads",          "mls"},
    {  0x4A, "aloadb",          "mls"},
    {  0x4B, "aloadbit",        "mls"},
    {  0x4C, "astore",          "mll"},
    {  0x4D, "astores",         "mll"},
    {  0x4E, "astoreb",         "mll"},
    {  0x4F, "astorebit",       "mll"},
    {  0x50, "stkcount",        "s"},
    {  0x51, "stkpeek",         "ls"},
    {  0x52, "stkswap",         ""},
    {  0x53, "stkroll",         "ll"},
    {  0x54, "stkcopy",         "l"},
    {  0x70, "streamchar",      "l"},
    {  0x71, "streamnum",       "l"},
    {  0x72, "streamstr",       "m"},
    {  0x73, "streamunichar",   "l"},
    { 0x100, "gestalt",         "lls"},
    { 0x101, "debugtrap",       "l"},
    { 0x102, "getmemsize",      "s"},
    { 0x103, "setmemsize",      "ls"},
    { 0x104, "jumpabs",         "a"},
    { 0x110, "random",          "ls"},
    { 0x111, "setrandom",       "l"},
    { 0x120, "quit",            ""},
    { 0x121, "verify",          "s"},
    { 0x122, "restart",         ""},
    { 0x123, "save",            "ls"},
    { 0x124, "restore",         "ls"},
    { 0x125, "saveundo",        "s"},
    { 0x126, "restoreundo",     "s"},
    { 0x127, "protect",         "ll"},
    { 0x130, "glk",             "lls"},
    { 0x140, "getstringtbl",    "s"},
    { 0x141, "setstringtbl",    "m"},
    { 0x148, "getiosys",        "ss"},
    { 0x149, "setiosys",        "ll"},
    { 0x150, "linearsearch",    "mlmlllls"},
    { 0x151, "binarysearch",    "mlmlllls"},
    { 0x152, "linkedsearch",    "mlmllls"},
    { 0x160, "callf",           "fs"},
    { 0x161, "callfi",          "fls"},
    { 0x162, "callfii",         "flls"},
    { 0x163, "callfiii",        "fllls"},
    { 0x170, "mzero",           "lm"},
    { 0x171, "mcopy",           "lmm"},
    { 0x178, "malloc",          "ls"},
    { 0x179, "mfree",           "l"},
    { 0x180, "accelfunc",       "lf"},
    { 0x181, "accelparam",      "ll"} };


static const int opcode_to_index[386] = {
   0,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
   1,   2,   3,   4,   5,   6,  -1,  -1,   7,   8,   9,  10,  11,  12,  13,  -1,
  14,  -1,  15,  16,  17,  18,  19,  20,  21,  22,  23,  24,  25,  26,  -1,  -1,
  27,  28,  29,  30,  31,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
  32,  33,  34,  -1,  35,  36,  -1,  -1,  37,  38,  39,  40,  41,  42,  43,  44,
  45,  46,  47,  48,  49,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
  50,  51,  52,  53,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
  54,  55,  56,  57,  58,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
  59,  60,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
  61,  62,  63,  64,  65,  66,  67,  68,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
  69,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
  70,  71,  -1,  -1,  -1,  -1,  -1,  -1,  72,  73,  -1,  -1,  -1,  -1,  -1,  -1,
  74,  75,  76,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
  77,  78,  79,  80,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
  81,  82,  -1,  -1,  -1,  -1,  -1,  -1,  83,  84,  -1,  -1,  -1,  -1,  -1,  -1,
  85,  86 };

extern const struct operation *operation_by_number(int opcode)
{
    int i = (opcode >= 0 || opcode < 386) ? opcode_to_index[opcode] : -1;
    return i >= 0 ? &operations[opcode] : NULL;
}

extern const struct operation *operation_by_name(const char *s)
{
    int i = -1;
    switch (s[0]) {
    case 'a':
        switch (s[1]) {
        case 'c':
            switch (s[5]) {
            case 'f': i = 85; break;
            case 'p': i = 86; break;
            } break;
        case 'd': i = 1; break;
        case 'l':
            switch (s[5]) {
            case '\0': i = 37; break;
            case 'b':
                switch (s[6]) {
                case '\0': i = 39; break;
                case 'i': i = 40; break;
                } break;
            case 's': i = 38; break;
            } break;
        case 's':
            switch (s[6]) {
            case '\0': i = 41; break;
            case 'b':
                switch (s[7]) {
                case '\0': i = 43; break;
                case 'i': i = 44; break;
                } break;
            case 's': i = 42; break;
            } break;
        } break;
    case 'b':
        switch (s[2]) {
        case 'n': i = 75; break;
        case 't':
            switch (s[3]) {
            case 'a': i = 7; break;
            case 'n': i = 10; break;
            case 'o': i = 8; break;
            case 'x': i = 9; break;
            } break;
        } break;
    case 'c':
        switch (s[1]) {
        case 'a':
            switch (s[2]) {
            case 'l':
                switch (s[4]) {
                case '\0': i = 27; break;
                case 'f':
                    switch (s[5]) {
                    case '\0': i = 77; break;
                    case 'i':
                        switch (s[6]) {
                        case '\0': i = 78; break;
                        case 'i':
                            switch (s[7]) {
                            case '\0': i = 79; break;
                            case 'i': i = 80; break;
                            } break;
                        } break;
                    } break;
                } break;
            case 't': i = 29; break;
            } break;
        case 'o':
            switch (s[4]) {
            case '\0': i = 32; break;
            case 'b': i = 34; break;
            case 's': i = 33; break;
            } break;
        } break;
    case 'd':
        switch (s[1]) {
        case 'e': i = 55; break;
        case 'i': i = 4; break;
        } break;
    case 'g':
        switch (s[1]) {
        case 'e':
            switch (s[2]) {
            case 's': i = 54; break;
            case 't':
                switch (s[3]) {
                case 'i': i = 72; break;
                case 'm': i = 56; break;
                case 's': i = 70; break;
                } break;
            } break;
        case 'l': i = 69; break;
        } break;
    case 'j':
        switch (s[1]) {
        case 'e': i = 17; break;
        case 'g':
            switch (s[2]) {
            case 'e':
                switch (s[3]) {
                case '\0': i = 20; break;
                case 'u': i = 24; break;
                } break;
            case 't':
                switch (s[3]) {
                case '\0': i = 21; break;
                case 'u': i = 25; break;
                } break;
            } break;
        case 'l':
            switch (s[2]) {
            case 'e':
                switch (s[3]) {
                case '\0': i = 22; break;
                case 'u': i = 26; break;
                } break;
            case 't':
                switch (s[3]) {
                case '\0': i = 19; break;
                case 'u': i = 23; break;
                } break;
            } break;
        case 'n':
            switch (s[2]) {
            case 'e': i = 18; break;
            case 'z': i = 16; break;
            } break;
        case 'u':
            switch (s[4]) {
            case '\0': i = 14; break;
            case 'a': i = 58; break;
            } break;
        case 'z': i = 15; break;
        } break;
    case 'l':
        switch (s[3]) {
        case 'e': i = 74; break;
        case 'k': i = 76; break;
        } break;
    case 'm':
        switch (s[1]) {
        case 'a': i = 83; break;
        case 'c': i = 82; break;
        case 'f': i = 84; break;
        case 'o': i = 5; break;
        case 'u': i = 3; break;
        case 'z': i = 81; break;
        } break;
    case 'n':
        switch (s[1]) {
        case 'e': i = 6; break;
        case 'o': i = 0; break;
        } break;
    case 'p': i = 68; break;
    case 'q': i = 61; break;
    case 'r':
        switch (s[1]) {
        case 'a': i = 59; break;
        case 'e':
            switch (s[2]) {
            case 's':
                switch (s[4]) {
                case 'a': i = 63; break;
                case 'o':
                    switch (s[7]) {
                    case '\0': i = 65; break;
                    case 'u': i = 67; break;
                    } break;
                } break;
            case 't': i = 28; break;
            } break;
        } break;
    case 's':
        switch (s[1]) {
        case 'a':
            switch (s[4]) {
            case '\0': i = 64; break;
            case 'u': i = 66; break;
            } break;
        case 'e':
            switch (s[2]) {
            case 't':
                switch (s[3]) {
                case 'i': i = 73; break;
                case 'm': i = 57; break;
                case 'r': i = 60; break;
                case 's': i = 71; break;
                } break;
            case 'x':
                switch (s[3]) {
                case 'b': i = 36; break;
                case 's': i = 35; break;
                } break;
            } break;
        case 'h': i = 11; break;
        case 's': i = 12; break;
        case 't':
            switch (s[2]) {
            case 'k':
                switch (s[3]) {
                case 'c':
                    switch (s[5]) {
                    case 'p': i = 49; break;
                    case 'u': i = 45; break;
                    } break;
                case 'p': i = 46; break;
                case 'r': i = 48; break;
                case 's': i = 47; break;
                } break;
            case 'r':
                switch (s[6]) {
                case 'c': i = 50; break;
                case 'n': i = 51; break;
                case 's': i = 52; break;
                case 'u': i = 53; break;
                } break;
            } break;
        case 'u': i = 2; break;
        } break;
    case 't':
        switch (s[1]) {
        case 'a': i = 31; break;
        case 'h': i = 30; break;
        } break;
    case 'u': i = 13; break;
    case 'v': i = 62; break;
    }

    return (i == -1 || strcmp(s, operations[i].mnemonic) != 0)
        ? NULL : &operations[i];
}
