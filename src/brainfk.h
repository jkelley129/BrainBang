#ifndef BRAINFK_H
#define BRAINFK_H

#include <stdio.h>

#define OP_END          0
#define OP_INC_DP       1
#define OP_DEC_DP       2
#define OP_INC_VAL      3
#define OP_DEC_VAL      4
#define OP_OUT          5
#define OP_IN           6
#define OP_JMP_FWD      7
#define OP_JMP_BCK      8

#define SUCCESS         0
#define FAILURE         1

#define PROGRAM_SIZE    4096
#define STACK_SIZE      512
#define DATA_SIZE       65535

#define STACK_PUSH(A)   (STACK[SP++] = A)
#define STACK_POP()     (STACK[--SP])
#define STACK_EMPTY()   (SP == 0)
#define STACK_FULL()    (SP == STACK_SIZE)

struct instruction_t {
    unsigned short operator;
    unsigned short operand;
};

int bf_run(const char* source_file);

#endif //BRAINFK_H