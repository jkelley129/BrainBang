#ifndef COMPILER_H
#define COMPILER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

#define MAX_LINE_LENGTH 1024
#define MAX_OUTPUT_SIZE 65536
#define MAX_LOOPS 128
#define MAX_LINES 1000

typedef struct {
    char *output;
    size_t output_size;
    size_t output_capacity;
    int loop_stack[MAX_LOOPS];
    int loop_stack_size;
} Compiler;

typedef struct {
    int indent_level;
    char content[MAX_LINE_LENGTH];
} ProcessedLine;

// Function prototypes
Compiler* compiler_new(void);
void compiler_free(Compiler *compiler);
int compiler_compile(Compiler *compiler, const char *source_code, char **result);
void append_output(Compiler *compiler, const char *str);
void append_char(Compiler *compiler, char c);
int preprocess_lines(const char *source_code, ProcessedLine *lines, int *line_count);
int process_line(Compiler *compiler, int indent_level, const char *content, int line_num);
int calculate_indent_level(const char *line);
char *trim_whitespace(char *str);
int handle_loop(Compiler *compiler, int indent_level);
int handle_ent(Compiler *compiler, const char *value_str);
int handle_cellin(Compiler *compiler);
int handle_cellout(Compiler *compiler);
int handle_shift(Compiler *compiler, const char *direction);
int handle_multi_shift(Compiler *compiler, const char *shift_str);
int handle_inc(Compiler *compiler, const char *inc_str);
int handle_dec(Compiler *compiler, const char *dec_str);
int handle_clr(Compiler *compiler);
int process_escape_sequences(const char *input, char *output);
char *read_file(const char *filename);
int write_file(const char *filename, const char *content);
int compile(const char* source_file);

#endif //COMPILER_H