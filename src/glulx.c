#include "c.h"
#define X(f) glulx_##f

#define FUNC_PREFIX  ":func@"
#define LABEL_PREFIX ":label@"
#define VAR_PREFIX   ":var@"

static int cur_seg = 0;

/* Used when emiting a function (and reset for each function): */
static int params_size;     /* size of formal parameter area */
static int locals_size;     /* size of locals area */
static int staging_size;    /* size of call staging area */

/* TODO: later implement blockbeg/blockend to keep track of live stack space,
         so we can reduce the size of the locals area somewhat.
         (See blockbeg()/blockend() implementation in gen.c for sample code.) */
static void X(blockbeg)(Env *e) { (void)e;  /* unused */ }
static void X(blockend)(Env *e) { (void)e;  /* unused */ }

static void X(defaddress)(Symbol p)
{
    assert(p->scope == GLOBAL);
    print("\t\tdc.l %s%s\n", isfunc(p->type) ? FUNC_PREFIX : VAR_PREFIX, p->name);
}

static void X(defconst)(int suffix, int size, Value v)
{
    switch (suffix)
    {
    case I:  /* signed integer */
        switch (size)
        {
        case 1: print("\t\tdc.b %d\n", (signed char)v.i);  return;
        case 2: print("\t\tdc.s %d\n", (signed short)v.i); return;
        case 4: print("\t\tdc.l %d\n", v.i);               return;
        }

        break;
    case U:  /* unsigned integer */
        switch (size)
        {
        case 1: print("\t\tdc.b %u\n", (unsigned char)v.u);  return;
        case 2: print("\t\tdc.s %u\n", (unsigned short)v.u); return;
        case 4: print("\t\tdc.l %u\n", v.u);                 return;
        }

    case P:  /* pointer */
        switch (size)
        {
        case 4: print("\t\tdc.l %p\n", (unsigned)(unsigned long)v.p); return;
        }
    }

    assert(0);  /* unsupported type/size */
}

static void X(defstring)(int n, char *s)
{
    int i;
    while (n > 0)
    {
        print("\t\tdc.b");
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
    print("\t\tds.b %u\n", n);
}

/* No implementation necessary: */
static void X(defsymbol)(Symbol p) { (void)p;  /* unused */ }
static void X(export)(Symbol p)    { (void)p;  /* unused */ }
static void X(import)(Symbol p)    { (void)p;  /* unused */ }

static void X(global)(Symbol p)
{
    print("\t%s%s\n", VAR_PREFIX, p->name);
}

static void X(local)(Symbol p)
{
    assert(p->scope >= LOCAL);
    p->x.offset = roundup(locals_size, p->type->align);
    locals_size = p->x.offset + p->type->size;
}

static void X(address)(Symbol p, Symbol q, long n)
{
    p->name = (n == 0) ? string(q->name) : stringf("%s:%D", q->name, n);
}

static void X(segment)(int seg)
{
    if (seg != cur_seg)
    {
        switch (seg)
        {
        case CODE: print("section code\n");  break;
        case LIT:  print("section data\n");  break;
        case DATA: print("section vdata\n"); break;
        case BSS:  print("section vdata\n"); break; /* FIXME: real BSS? */
        }
        cur_seg = seg;
    }
}

static void X(function)(Symbol f, Symbol caller[], Symbol callee[], int ncalls)
{
    Symbol *pp, p;

    params_size  = 0;
    locals_size  = 0;
    staging_size = 0;

    /* Find offsets for parameters (relative to base pointer) */
    for (pp = callee; (p = *pp) != NULL; ++pp)
    {
        assert(p->scope == PARAM);
        params_size = roundup(params_size + p->type->size, p->type->align);
        p->x.offset = -params_size;
    }

    /* Gencode counts size used by locals/function calls */
    gencode(caller, callee);

    /* Emit prologue */
    print("\t%s%s\n", FUNC_PREFIX , f->name);
    print("\t\tdc.b 0xc1 4 2 0 0\n");
    print("\t\tadd {0}.l %d {4}.l\n", locals_size + staging_size);

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
    print("\t\treturn 0\n\n");
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

static void eval_call(Node p, int keep_result)
{
    assert(generic(p->op) == CALL);
    assert(p->kids[0]->op == ADDRG + P + sizeop(4));
    assert(p->kids[0]->syms[0]->scope == GLOBAL);
    print("\t\tcallfi %s%s.l {4}.l %s\n",
            FUNC_PREFIX, p->kids[0]->syms[0]->name,
            keep_result ? "(sp)" : "~" );
}

static void eval_to_stack(Node p)
{
    switch (generic(p->op))
    {
    case CALL:
        eval_call(p, 1);
        break;

    case ADDRG:  /* address of global variable */
        print("\t\tcopy %s%s.l (sp)  ; global %s\n",
              VAR_PREFIX, p->syms[0]->name, p->syms[0]->name);
        break;

    case ADDRF:  /* address of argument (formal parameter) */
        print("\t\tadd {0}.l %d (sp)  ; param %s\n",
              p->syms[0]->x.offset, p->syms[0]->name);
        break;

    case ADDRL:  /* address of local variable */
        print("\t\tadd {0}.l %d (sp)  ; local %s\n",
              p->syms[0]->x.offset, p->syms[0]->name);
        break;

    case INDIR:  /* fetch value by address */
        eval_to_stack(p->kids[0]);
        switch (p->op)
        {
        case INDIR + I + sizeop(1):
            print("\t\taloadb (sp) 0 (sp)\n"
                  "\t\tsexb (sp) (sp)\n");
            break;
        case INDIR + I + sizeop(2):
            print("\t\taloads (sp) 0 (sp)\n"
                  "\t\tsexs (sp) (sp)\n");
            break;
        case INDIR + U + sizeop(1):
            print("\t\taloadb (sp) 0 (sp)\n");
            break;
        case INDIR + U + sizeop(2):
            print("\t\taloads (sp) 0 (sp)\n");
            break;
        case INDIR + I + sizeop(4):
        case INDIR + U + sizeop(4):
        case INDIR + P + sizeop(4):
            print("\t\taload (sp) 0 (sp)\n");
            break;
        default:
            assert(0);
        }
        break;

    case CNST:
        switch (p->op)
        {
        case CNST + I + sizeop(1):
        case CNST + I + sizeop(2):
        case CNST + I + sizeop(4):
            print("\t\tcopy %d (sp)\n", (int)p->syms[0]->u.c.v.i);
            break;
        case CNST + U + sizeop(1):
        case CNST + U + sizeop(2):
        case CNST + U + sizeop(4):
        case CNST + P + sizeop(4):
            print("\t\tcopy %u (sp)\n", (unsigned)p->syms[0]->u.c.v.u);
            break;
        default: assert(0);
        }
        break;

    case CVF:  /* convert from float */
        assert(0);
        break;

    case CVU:  /* convert from unsigned integer */
    case CVP:  /* convert from pointer */
    case CVI:  /* convert from signed integer */
        {
            assert(optype(p->op) == I || optype(p->op) == U || optype(p->op) == P);
            assert(opsize(p->op) == 4 || opsize(p->kids[0]->op) == 4);
            eval_to_stack(p->kids[0]);
            switch (opkind(p->op))
            {
            case I + sizeop(1):
                print("\t\tsexb (sp) (sp)\n");
                break;
            case U + sizeop(1):
                print("\t\tbitand (sp) 0xff (sp)\n");
                break;
            case I + sizeop(2):
                print("\t\tsexs (sp) (sp)\n");
                break;
            case U + sizeop(2):
                print("\t\tbitand (sp) 0xffff (sp)\n");
                break;
            case I + sizeop(4):
            case U + sizeop(4):
                break;
            default: assert(0);
            }
        }
        break;

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
            eval_to_stack(p->kids[0]);
            print("\t\t%s (sp) (sp)\n", op);
        } break;

    case ADD:
    case SUB:
    case MUL:
    case DIV:
    case MOD:
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
            case MUL:  op = "mul"; assert(optype(p->op) == I); break;
            case DIV:  op = "div"; assert(optype(p->op) == I); break;
            case MOD:  op = "mod"; assert(optype(p->op) == I); break;
            case BAND: op = "bitand"; break;
            case BOR:  op = "bitor"; break;
            case BXOR: op = "bitxor"; break;
            case LSH:  op = "shiftl"; break;
            case RSH:  op = optype(p->op) == I ? "sshifr" : "ushiftr"; break;
            }
            assert(op != NULL);
            eval_to_stack(p->kids[1]);
            eval_to_stack(p->kids[0]);
            print("\t\t%s (sp) (sp) (sp)\n", op);
        } break;

    default: assert(0);
    }
}

static void X(emit)(Node p)
{
    for (; p; p = p->link)
    {
        switch (generic(p->op))
        {
        case CALL:
            eval_call(p, 0);
            break;

        case ARG:
            assert(opsize(p->op) == 4);
            assert(p->x.argno > 0);
            eval_to_stack(p->kids[0]);
            print("\t\tastore {4}.l %d (sp)\n", -p->x.argno);
            break;

        case ASGN:
            {
                eval_to_stack(p->kids[1]);  /* source value */
                eval_to_stack(p->kids[0]);  /* destination address */
                switch (p->op)
                {
                case ASGN + I + sizeop(1):
                case ASGN + U + sizeop(1):
                    print("\t\tastoreb (sp) 0 (sp)\n");
                    break;
                case ASGN + I + sizeop(2):
                case ASGN + U + sizeop(2):
                    print("\t\tastores (sp) 0 (sp)\n");
                    break;
                case ASGN + I + sizeop(4):
                case ASGN + U + sizeop(4):
                case ASGN + P + sizeop(4):
                    print("\t\tastore (sp) 0 (sp)\n");
                    break;

                case ASGN + B:
                    print("; TODO: struct assignment (size=%s)\n", p->syms[0]->name);
                    assert(0);
                    break;

                default:
                    assert(0);
                }
            } break;

        case JUMP:
            assert(p->kids[0]->op == ADDRG + P + sizeop(4));
            assert(p->kids[0]->syms[0]->scope == LABELS);
            print("\t\tjump %s%s\n", LABEL_PREFIX, p->kids[0]->syms[0]->name);
            break;

        case LABEL:
            print("\t%s%s\n", LABEL_PREFIX, p->syms[0]->name);
            break;

        case EQ:
        case GE:
        case GT:
        case LE:
        case LT:
        case NE:
            {
                const char *op = NULL;

                eval_to_stack(p->kids[1]);  /* second operand */
                eval_to_stack(p->kids[0]);  /* first operand */

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

                print("\t\t%s (sp) (sp) %s%s\n",
                      op, LABEL_PREFIX, p->syms[0]->name);
            } break;

        case RET:
            eval_to_stack(p->kids[0]);
            print("\t\treturn (sp)\n");
            break;

        default:
            assert(0);
        }
    }
}

static void X(progbeg)(int argc, char *argv[])
{
    parseflags(argc, argv);

    X(segment)(CODE);
}

static void X(progend)(void)
{
    X(segment)(BSS);
    print("\t:callstack\n"
          "\t\tds.l 15360   ; 60 KB\n");

    X(segment)(0);
    print("ext 0\n");
    print("stack 4096\n");
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
