/* Glulx backend by Maks Verver <maksverver@geocities.com> July 2009 */

#include "c.h"
#define X(f) glulx_##f

/* Used when emiting a function (and reset for each function): */
static int params_size;     /* size of formal parameter area */
static int locals_size;     /* size of locals area */
static int staging_size;    /* size of call staging area */
static int reg_offset;      /* offset "registers" (local variables) */

/* TODO: later implement blockbeg/blockend to keep track of live stack space,
         so we can reduce the size of the locals area somewhat.
         (See blockbeg()/blockend() implementation in gen.c for sample code.) */
static void X(blockbeg)(Env *e) { (void)e;  /* unused */ }
static void X(blockend)(Env *e) { (void)e;  /* unused */ }

static void X(defaddress)(Symbol p)
{
    print("\tdc.l :%s\n", p->name);
}

static void X(defconst)(int suffix, int size, Value v)
{
    switch (suffix)
    {
    case I:  /* signed integer */
        switch (size)
        {
        case 1: print("\tdc.b %d\n", (signed char)v.i);  return;
        case 2: print("\tdc.s %d\n", (signed short)v.i); return;
        case 4: print("\tdc.l %d\n", v.i);               return;
        }

        break;
    case U:  /* unsigned integer */
        switch (size)
        {
        case 1: print("\tdc.b %u\n", (unsigned char)v.u);  return;
        case 2: print("\tdc.s %u\n", (unsigned short)v.u); return;
        case 4: print("\tdc.l %u\n", v.u);                 return;
        }

    case P:  /* pointer */
        switch (size)
        {
        case 4: print("\tdc.l %p\n", (unsigned)(unsigned long)v.p); return;
        }
    }

    assert(0);  /* unsupported type/size */
}

static void X(defstring)(int n, char *s)
{
    int i;
    while (n > 0)
    {
        print("\tdc.b");
        for (i = 0; i < 8 && i < n; ++i)
        {
            print(" %d", (int)s[i]);
        }
        s += i;
        n -= i;
        print("\n");
    }
}

static void X(space)(int n)
{
    /* NOTE: space() should allocate  zero bytes; we can use a block of
             unitialized data here because Glulx will initialize it to zero. */
    print("\tds.b %u\n", n);
}

/* No implementation necessary: */
static void X(defsymbol)(Symbol p) { (void)p;  /* unused */ }
static void X(import)(Symbol p)    { (void)p;  /* unused */ }

static void X(export)(Symbol p)
{
    print("export %s\n", p->name);
}

static void X(global)(Symbol p)
{
    print(":%s\n", p->name);
}

static int upgrade_to_register(Symbol p)
{
    assert(p->sclass == AUTO || p->sclass == REGISTER);
    if (isint(p->type) && !p->addressed)
    {
        p->sclass   = REGISTER;
        p->x.offset = reg_offset;
        reg_offset  += 4;
        return 1;
    }
    else
    {
        p->sclass = AUTO;
        return 0;
    }
}

static void X(local)(Symbol p)
{
    assert(p->scope >= LOCAL);
    if (!upgrade_to_register(p))
    {
        p->x.offset = roundup(locals_size, p->type->align);
        locals_size = p->x.offset + p->type->size;
    }
}

static void X(address)(Symbol p, Symbol q, long n)
{
    p->name = (n == 0) ? string(q->name) : stringf("%s:%D", q->name, n);
    p->x.offset = q->x.offset + n;
}

static void X(segment)(int seg)
{
    switch (seg)
    {
    case CODE: print("section code\n");  break;
    case LIT:  print("section data\n");  break;
    case DATA: print("section vdata\n"); break;
    case BSS:  print("section bss\n");   break;
    default: assert(0);
    }
}

static void X(function)(Symbol f, Symbol caller[], Symbol callee[], int ncalls)
{
    params_size  = 0;
    locals_size  = 0;
    staging_size = 0;
    reg_offset   = ncalls == 0 ? 4 : 8;

    /* Find offsets for parameters (relative to base pointer) */
    {
        int i;
        for (i = 0; caller[i] != NULL && callee[i] != NULL; ++i)
        {
            assert(callee[i]->scope == PARAM);
            assert(callee[i]->type->size <= 4);
            assert(caller[i]->type->size <= 4);
            params_size = roundup(params_size + 4, 4);
            caller[i]->x.offset = -params_size;
            callee[i]->x.offset = -params_size;
            upgrade_to_register(callee[i]);
        }
        assert(caller[i] == NULL && callee[i] == NULL);
    }

    /* Gencode counts size used by locals/function calls */
    gencode(caller, callee);

    /* Emit prologue */
    assert(reg_offset >= 4 && reg_offset%4 == 0);
    print("func_local %s %d\n", f->name, reg_offset/4);
    if (ncalls > 0)
        print("\tadd {0} %d {4}\n", locals_size + staging_size);

    /* Caller passes BP in loc(0), callee computes SP in loc(1)

              | +---------+
    incoming  | | arg 2.. |
    arguments | +---------+
              v |  arg 1  |     offset: e.g. -4 (relative to BP)
             -- +---------+ <-- BP == local addr 0
              ^ | local 1 |     offset: e.g 0 (relative to BP)
       local  | +---------+
   variables  | |   ...   |
              | +---------+ 
              v | local N |
             -- +---------+
    outgoing  ^ |  arg N  |
   arguments  | +---------+
              | |   ...   |
              | +---------|
              v |  arg 1  |     offset: e.g. -4 (relative to SP)
             -- +---------+ <-- SP == local addr 4


    SP - BP = local_size + staging_size

    Since lcc unnests function calls (e.g. f(g()) becomes tmp=g();f(tmp); )
    the size of the staging area is equal to the maximum size of the argument
    set of all functions called from this function.
    */

    emitcode();

    /* Emit epilogue */
    print("\treturn 0\n\n");
}

static void gen_visit(Node p)
{
    static int num_args = 0;

    if (p == NULL || p->x.listed) return;
    p->x.listed = 1;

    gen_visit(p->kids[0]);
    gen_visit(p->kids[1]);

    switch (generic(p->op))
    {
    case CALL:
        if (4*num_args > staging_size)
            staging_size = 4*num_args;
        num_args = 0;
        break;

    case ARG:
        assert(opsize(p->op) == 4);
        ++num_args;
        assert(p->x.argno == 0);
        p->x.argno = num_args;
        break;
    }
}

static Node X(gen)(Node start)
{
    Node p;
    for (p = start; p; p = p->link)
    {
        gen_visit(p);
    }
    return start;
}

/* Evaluate a node to a value that can be used as an operand in an instruction.
   This may generate code to evaluate to the stack and then return (sp), or it
   could return a constant or memory or local variable reference.

   If dest is not NULL, the call result must be placed in that operand, and
   dest is returned. Otherwise, the function returns a value suitable to be used
   as an instruction operand.

   NB. when this function is called to evaluate the arguments to another opcode,
    order of evaluation matters! e.g. this is the correct:
        arg2 = eval_value(p->kids[1]);
        arg1 = eval_value(p->kids[0]);
        print("add %s %s (sp)", arg1, arg2);
    But this isn't:
        arg1 = eval_value(p->kids[0]);
        arg2 = eval_value(p->kids[1]);
        print("add %s %s (sp)", arg1, arg2);
    Since both arg1 and arg2 may return (sp) in which case arg2 should be popped
    of the stack first. */
static const char *eval_value(Node p, const char *dest);

static const char *eval_call(Node p, const char *dest)
{
    assert(generic(p->op) == CALL);
    if (dest == NULL) dest = "(sp)";
    switch (p->kids[0]->op)
    {
    case ADDRG + P + sizeop(4):  /* call to label */
        print("\tcallfi :%s {4} %s\n", p->kids[0]->syms[0]->name, dest);
        break;

    case INDIR + P + sizeop(4):  /* call through function pointer */
        print("\tcallfi %s {4} %s\n", eval_value(p->kids[0], NULL), dest);
        break;

    default:
        assert(0);
    }
    return dest;
}

/* Returns whether the given node is actually a register access: an ADDRL or
   ADDRF node with an associated symbol with REGISTER storage class. */
static int access_register(Node p)
{
    return (generic(p->op) == ADDRL || generic(p->op) == ADDRF) &&
        p->syms[0]->sclass == REGISTER;
}

static int access_cstack(Node p)
{
    return (generic(p->op) == ADDRL || generic(p->op) == ADDRF) &&
        p->syms[0]->sclass == AUTO;
}

static const char *copy(const char *src, const char *dest)
{
    if (dest == NULL) return src;
    if (strcmp(src, dest) != 0) print("\tcopy %s %s\n", src, dest);
    return dest;
}

static const char *eval_value(Node p, const char *dest)
{
    const char *arg1, *arg2;

    switch (generic(p->op))
    {
    case CALL:
        return eval_call(p, dest);

    case ADDRG:  /* address of global variable */
        return copy(stringf(":%s", p->syms[0]->name), dest);

    case ADDRF:  /* address of argument (formal parameter) */
        assert(p->syms[0]->sclass == AUTO);
        if (dest == NULL) dest = "(sp)";
        print("\tadd {0} %d %s  ; param %s\n",
            p->syms[0]->x.offset, dest, p->syms[0]->name);
        return dest;

    case ADDRL:  /* address of local variable */
        assert(p->syms[0]->sclass == AUTO);
        if (dest == NULL) dest = "(sp)";
        print("\tadd {0} %d %s  ; local %s\n",
            p->syms[0]->x.offset, dest, p->syms[0]->name);
        return dest;

    case INDIR:  /* fetch value by address */
        if (access_register(p->kids[0]))
        {
            return copy(stringf("{%d}", p->kids[0]->syms[0]->x.offset), dest);
        }
        else
        if (access_cstack(p->kids[0]))
        {
            assert(opsize(p->op) == 4);
            if (dest == NULL) dest = "(sp)";
            assert(p->kids[0]->syms[0]->x.offset%4 == 0);
            print("\taload {0} %d %s\n", p->kids[0]->syms[0]->x.offset/4, dest);
            return dest;
        }
        else
        {
            if (dest == NULL) dest = "(sp)";
            arg1 = eval_value(p->kids[0], NULL);
            switch (p->op)
            {
            case INDIR + I + sizeop(1):
                print("\taloadb %s 0 (sp)\n"
                      "\tsexb (sp) %s\n", arg1, dest);
                return dest;
            case INDIR + I + sizeop(2):
                print("\taloads %s 0 (sp)\n"
                    "\tsexs (sp) %s\n", arg1, dest);
                return dest;
            case INDIR + U + sizeop(1):
                print("\taloadb %s 0 %s\n", arg1, dest);
                return dest;
            case INDIR + U + sizeop(2):
                print("\taloads %s 0 %s\n", arg1, dest);
                return dest;
            case INDIR + I + sizeop(4):
            case INDIR + U + sizeop(4):
            case INDIR + P + sizeop(4):
                print("\taload %s 0 %s\n", arg1, dest);
                return dest;
            }
        }
        break;

    case CNST:
        switch (p->op)
        {
        case CNST + I + sizeop(1):
        case CNST + I + sizeop(2):
        case CNST + I + sizeop(4):
            return copy(stringf("%d", (int)p->syms[0]->u.c.v.i), dest);
        case CNST + U + sizeop(1):
        case CNST + U + sizeop(2):
        case CNST + U + sizeop(4):
        case CNST + P + sizeop(4):
            return copy(stringf("%u", (unsigned)p->syms[0]->u.c.v.u), dest);
        }
        break;

    case CVF:  /* convert from float */
        assert(0);
        break;

    case CVU:  /* convert from unsigned integer */
    case CVP:  /* convert from pointer */
    case CVI:  /* convert from signed integer */
        assert(optype(p->op) == I || optype(p->op) == U || optype(p->op) == P);
        assert(opsize(p->op) == 4 || opsize(p->kids[0]->op) == 4);
        arg1 = eval_value(p->kids[0], dest);
        switch (opkind(p->op))
        {
        case I + sizeop(1):
            if (dest == NULL) dest = "(sp)";
            print("\tsexb %s %s\n", arg1, dest);
            return dest;
        case U + sizeop(1):
            if (dest == NULL) dest = "(sp)";
            print("\tbitand %s 0xff %s\n", arg1, dest);
            return dest;
        case I + sizeop(2):
            if (dest == NULL) dest = "(sp)";
            print("\tsexs %s %s\n", arg1, dest);
            return dest;
        case U + sizeop(2):
            if (dest == NULL) dest = "(sp)";
            print("\tbitand %s 0xffff %s\n", arg1, dest);
            return dest;
        case I + sizeop(4):
        case U + sizeop(4):
            return copy(arg1, dest);
        }

    case BCOM:
    case NEG:
        {   /* Unary math operators */
            const char *op = NULL;
            assert(optype(p->op) == I || optype(p->op) == U);
            assert(opsize(p->op) == 4);
            assert(opkind(p->op) == opkind(p->kids[0]->op));
            switch (generic(p->op))
            {
            case BCOM: op = "bitnot"; break;
            case NEG:  op = "neg";    break;
            }
            assert(op != NULL);
            arg1 = eval_value(p->kids[0], NULL);
            if (dest == NULL) dest = "(sp)";
            print("\t%s %s %s\n", op, arg1, dest);
            return dest;
        }

    case ADD:
    case SUB:
    case MUL:
    case BAND:
    case BOR:
    case BXOR:
    case LSH:
    case RSH:
        {   /* Binary math operators */
            const char *op = NULL;
            assert(optype(p->op) == I || optype(p->op) == U || optype(p->op) == P);
            assert(opsize(p->op) == 4);
            switch (generic(p->op))
            {
            case ADD:  op = "add"; break;
            case SUB:  op = "sub"; break;
            case MUL:  op = "mul"; break;
            case BAND: op = "bitand"; break;
            case BOR:  op = "bitor"; break;
            case BXOR: op = "bitxor"; break;
            case LSH:  op = "shiftl"; break;
            case RSH:  op = optype(p->op) == I ? "sshiftr" : "ushiftr"; break;
            }
            assert(op != NULL);
            arg2 = eval_value(p->kids[1], NULL);
            arg1 = eval_value(p->kids[0], NULL);
            if (dest == NULL) dest = "(sp)";
            print("\t%s %s %s %s\n", op, arg1, arg2, dest);
            return dest;
        }
    case DIV:
    case MOD:
        {
            const char *op = generic(p->op) == DIV ? "div" : "mod";
            assert(opsize(p->op) == 4);
            arg2 = eval_value(p->kids[1], NULL);
            arg1 = eval_value(p->kids[0], NULL);
            if (dest == NULL) dest = "(sp)";
            if (optype(p->op) == I)  /* signed div/mod */
            {
                print("\t%s %s %s %s\n", op, arg1, arg2, dest);
                return dest;
            }
            else  /* unsigned div/mod */
            {
                int label, stack_args;
                assert(optype(p->op) == U || optype(p->op) == P);
                stack_args = 0;
                if (strcmp(arg1, "(sp)") == 0) ++stack_args;
                if (strcmp(arg2, "(sp)") == 0) ++stack_args;
                if (stack_args > 0) print("\tstkcopy %d\n", stack_args);
                label = genlabel(2);
                print("\tbitor %s %s (sp)\n"          /* arg1 arg2 */
                      "\tjge (sp) 0 :%d\n"            /* l0 */
                      "\tcallfii :_u%s %s %s %s\n"    /* op arg1 arg2 dest */
                      "\tjump :%d\n"                  /* l1 */
                      ":%d\n\t%s %s %s %s\n"          /* l0 op arg1 arg2 dest */
                      ":%d\n",                        /* l1 */
                      arg1, arg2,
                      label+0,
                      op, arg1, arg2, dest,
                      label+1,
                      label+0, op, arg1, arg2, dest,
                      label+1 );
                return dest;
#if 0
                /* Alternative: always perform the call, which generates less
                   code and executes less instructions when the _udiv/_umod has
                   to be called anyway, but is more expensive when used with
                   small integers. */
                print("\tcallfii :_u%s %s %s %s\n", op, arg1, arg2, dest);
                return dest;
#endif
            }
        }
    }

    fprintf(stderr, "unhandled op: %#x\n", p->op);
    assert(0);
}

static void X(emit)(Node p)
{
    for (; p; p = p->link)
    {
        switch (generic(p->op))
        {
        case CALL:
            eval_call(p, "~");
            break;

        case ARG:
            assert(opsize(p->op) == 4);
            assert(p->x.argno > 0);
            print("\tastore {4} %d %s\n",
                  -p->x.argno, eval_value(p->kids[0], NULL));
            break;

        case ASGN:;
            if (access_register(p->kids[0]))
            {
                /* Assign to register */
                eval_value(p->kids[1],
                           stringf("{%d}", p->kids[0]->syms[0]->x.offset));
            }
            else
            {
                const char *lhs, *rhs;
                if (p->op == ASGN + B)
                {
                    assert(p->kids[1]->op == INDIR + B);
                    lhs = eval_value(p->kids[0], NULL);
                    rhs = eval_value(p->kids[1]->kids[0], NULL);
                    print("\tmcopy %d %s %s\n", p->syms[0]->u.c.v.i, rhs, lhs);
                }
                else
                {
                    rhs = eval_value(p->kids[1], NULL);
                    lhs = eval_value(p->kids[0], NULL);
                    switch (p->op)
                    {
                    case ASGN + I + sizeop(1):
                    case ASGN + U + sizeop(1):
                        print("\tastoreb %s 0 %s\n", lhs, rhs);
                        break;
                    case ASGN + I + sizeop(2):
                    case ASGN + U + sizeop(2):
                        print("\tastores %s 0 %s\n", lhs, rhs);
                        break;
                    case ASGN + I + sizeop(4):
                    case ASGN + U + sizeop(4):
                    case ASGN + P + sizeop(4):
                        print("\tastore %s 0 %s\n", lhs, rhs);
                        break;
                    default:
                        assert(0);
                    }
                }
            } break;

        case JUMP:
            if (p->kids[0]->op == ADDRG + P + sizeop(4))
            {
                print("\tjump :%s\n", p->kids[0]->syms[0]->name);
            }
            else
            {
                assert(p->kids[0]->op == INDIR + P + sizeop(4));
                print("\tjumpabs %s\n", eval_value(p->kids[0], NULL));
            }
            break;

        case LABEL:
            print(":%s\n", p->syms[0]->name);
            break;

        case EQ:
        case GE:
        case GT:
        case LE:
        case LT:
        case NE:
            {
                const char *op = NULL, *arg1, *arg2;

                /* TODO: generate jz/jnz if applicable? */

                switch (p->op)
                {
                case EQ + I + sizeop(4):
                case EQ + U + sizeop(4): op = "jeq"; break;

                case NE + I + sizeop(4):
                case NE + U + sizeop(4): op = "jne"; break;

                case GE + I + sizeop(4): op = "jge"; break;
                case GT + I + sizeop(4): op = "jgt"; break;
                case LE + I + sizeop(4): op = "jle"; break;
                case LT + I + sizeop(4): op = "jlt"; break;

                case GE + U + sizeop(4): op = "jgeu"; break;
                case GT + U + sizeop(4): op = "jgtu"; break;
                case LE + U + sizeop(4): op = "jleu"; break;
                case LT + U + sizeop(4): op = "jltu"; break;
                }

                assert(op != NULL);
                assert(p->syms[0]->scope == LABELS);
                arg2 = eval_value(p->kids[1], NULL);
                arg1 = eval_value(p->kids[0], NULL);
                print("\t%s %s %s :%s\n", op, arg1, arg2, p->syms[0]->name);
            } break;

        case RET:
            print("\treturn %s\n", eval_value(p->kids[0], NULL));
            break;

        default:
            assert(0);
        }
    }
}

static void X(progbeg)(int argc, char *argv[])
{
    parseflags(argc, argv);
}

static void X(progend)(void)
{
}

Interface glulxIR = {
    1, 1, 0,  /* char */
    2, 2, 0,  /* short */
    4, 4, 0,  /* int */
    4, 4, 0,  /* long */
    4, 4, 0,  /* long long */
    4, 4, 1,  /* float */
    8, 4, 1,  /* double */
    8, 4, 1,  /* long double */
    4, 4, 0,  /* T * */
    0, 1, 0,  /* struct */
    0,        /* little_endian */
    0,        /* mulops_calls */
    0,        /* wants_callb */
    0,        /* wants_argb */
    1,        /* left_to_right */
    0,        /* wants_dag */
    1,        /* unsigned_char */
    X(address),
    X(blockbeg),
    X(blockend),
    X(defaddress),
    X(defconst),
    X(defstring),
    X(defsymbol),
    X(emit),
    X(export),
    X(function),
    X(gen),
    X(global),
    X(import),
    X(local),
    X(progbeg),
    X(progend),
    X(segment),
    X(space),
    0, 0, 0, 0, 0, 0, 0,
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
};
