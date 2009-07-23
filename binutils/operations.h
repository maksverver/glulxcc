#ifndef OPERATIONS_H_INCLUDED
#define OPERATIONS_H_INCLUDED

struct operation {
    int opcode;
    const char *mnemonic;
    const char *parameters;
};

extern const int noperation;
extern const struct operation operations[];
extern const struct operation *operation_by_name(const char *s);
extern const struct operation *operation_by_number(int opcode);

#endif /* ndef OPERATIONS_H_INCLUDED */
