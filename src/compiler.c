/*
 * BrainBang to Brainfuck Compiler (C Implementation)
 * A compiler that translates BrainBang (a more readable Brainfuck) to standard Brainfuck.
 */
#include "compiler.h"

// Create new compiler instance
Compiler* compiler_new(void) {
    Compiler *compiler = malloc(sizeof(Compiler));
    if (!compiler) return NULL;
    
    compiler->output_capacity = 1024;
    compiler->output = malloc(compiler->output_capacity);
    if (!compiler->output) {
        free(compiler);
        return NULL;
    }
    
    compiler->output[0] = '\0';
    compiler->output_size = 0;
    compiler->loop_stack_size = 0;
    
    return compiler;
}

// Free compiler memory
void compiler_free(Compiler *compiler) {
    if (compiler) {
        if (compiler->output) {
            free(compiler->output);
        }
        free(compiler);
    }
}

// Append string to output buffer
void append_output(Compiler *compiler, const char *str) {
    size_t len = strlen(str);
    size_t required = compiler->output_size + len + 1;
    
    if (required > compiler->output_capacity) {
        while (compiler->output_capacity < required) {
            compiler->output_capacity *= 2;
        }
        compiler->output = realloc(compiler->output, compiler->output_capacity);
    }
    
    strcpy(compiler->output + compiler->output_size, str);
    compiler->output_size += len;
}

// Append single character to output buffer
void append_char(Compiler *compiler, char c) {
    char str[2] = {c, '\0'};
    append_output(compiler, str);
}

// Calculate indentation level of a line
int calculate_indent_level(const char *line) {
    int level = 0;
    for (int i = 0; line[i]; i++) {
        if (line[i] == ' ') {
            level++;
        } else if (line[i] == '\t') {
            level += 4;  // Treat tab as 4 spaces
        } else {
            break;
        }
    }
    return level;
}

// Trim whitespace from string
char *trim_whitespace(char *str) {
    // Trim leading whitespace
    while (isspace(*str)) str++;
    
    if (*str == '\0') return str;
    
    // Trim trailing whitespace
    char *end = str + strlen(str) - 1;
    while (end > str && isspace(*end)) end--;
    *(end + 1) = '\0';
    
    return str;
}

// Process escape sequences in a string
int process_escape_sequences(const char *input, char *output) {
    int i = 0, j = 0;
    
    while (input[i]) {
        if (input[i] == '\\' && input[i + 1]) {
            switch (input[i + 1]) {
                case 'n': output[j++] = '\n'; break;
                case 't': output[j++] = '\t'; break;
                case 'r': output[j++] = '\r'; break;
                case '\\': output[j++] = '\\'; break;
                case '\'': output[j++] = '\''; break;
                case '"': output[j++] = '"'; break;
                case '0': output[j++] = '\0'; break;
                default:
                    // Unknown escape sequence, keep as is
                    output[j++] = input[i];
                    output[j++] = input[i + 1];
                    break;
            }
            i += 2;
        } else {
            output[j++] = input[i++];
        }
    }
    output[j] = '\0';
    return j;
}

// Preprocess lines to handle comments and indentation
int preprocess_lines(const char *source_code, ProcessedLine *lines, int *line_count) {
    char *source_copy = malloc(strlen(source_code) + 1);
    strcpy(source_copy, source_code);
    
    char *line = strtok(source_copy, "\n");
    *line_count = 0;
    
    while (line && *line_count < MAX_LINES) {
        // Remove comments
        char *comment_pos = strstr(line, "//");
        if (comment_pos) {
            *comment_pos = '\0';
        }
        
        // Skip empty lines
        char *trimmed = trim_whitespace(line);
        if (strlen(trimmed) == 0) {
            line = strtok(NULL, "\n");
            continue;
        }
        
        int indent_level = calculate_indent_level(line);
        char *content = trim_whitespace(line);
        
        // Handle multi-line statements separated by semicolons
        if (strchr(content, ';')) {
            // Split on semicolons, but be careful about strings
            char *pos = content;
            while (*pos && *line_count < MAX_LINES) {
                char stmt[MAX_LINE_LENGTH] = {0};
                int stmt_pos = 0;
                bool in_string = false;
                bool in_char = false;
                
                // Extract one statement
                while (*pos) {
                    if (*pos == '"' && !in_char && (stmt_pos == 0 || stmt[stmt_pos-1] != '\\')) {
                        in_string = !in_string;
                    } else if (*pos == '\'' && !in_string && (stmt_pos == 0 || stmt[stmt_pos-1] != '\\')) {
                        in_char = !in_char;
                    } else if (*pos == ';' && !in_string && !in_char) {
                        stmt[stmt_pos++] = *pos;  // Include the semicolon
                        pos++;
                        break;
                    }
                    
                    if (stmt_pos < MAX_LINE_LENGTH - 1) {
                        stmt[stmt_pos++] = *pos;
                    }
                    pos++;
                }
                
                char *trimmed_stmt = trim_whitespace(stmt);
                if (strlen(trimmed_stmt) > 0) {
                    lines[*line_count].indent_level = indent_level;
                    strcpy(lines[*line_count].content, trimmed_stmt);
                    (*line_count)++;
                }
                
                // Skip whitespace after semicolon
                while (*pos && isspace(*pos)) pos++;
                if (!*pos) break;
            }
        } else {
            // Single statement
            if (strlen(content) > 0 && !strstr(content, ":") && content[strlen(content) - 1] != ';') {
                free(source_copy);
                return -1; // Line must end with semicolon
            }
            
            lines[*line_count].indent_level = indent_level;
            strcpy(lines[*line_count].content, content);
            (*line_count)++;
        }
        
        line = strtok(NULL, "\n");
    }
    
    free(source_copy);
    return 0;
}

// Handle loop declaration
int handle_loop(Compiler *compiler, int indent_level) {
    if (compiler->loop_stack_size >= MAX_LOOPS) {
        return -1; // Too many nested loops
    }
    
    append_char(compiler, '[');
    compiler->loop_stack[compiler->loop_stack_size++] = indent_level;
    return 0;
}

// Handle ent statement for entering values
int handle_ent(Compiler *compiler, const char *value_str) {
    char *trimmed = trim_whitespace((char*)value_str);
    
    if (strcmp(trimmed, "input") == 0) {
        append_output(compiler, "[-]");  // Clear current cell
        append_char(compiler, ',');
    } else if (trimmed[0] == '\'' && trimmed[strlen(trimmed) - 1] == '\'' && strlen(trimmed) >= 3) {
        // Character literal
        char content[MAX_LINE_LENGTH];
        strncpy(content, trimmed + 1, strlen(trimmed) - 2);
        content[strlen(trimmed) - 2] = '\0';
        
        char processed[MAX_LINE_LENGTH];
        int len = process_escape_sequences(content, processed);
        
        if (len != 1) {
            return -1; // Character literal must contain exactly one character
        }
        
        int ascii_val = (unsigned char)processed[0];
        append_output(compiler, "[-]");  // Clear current cell
        for (int i = 0; i < ascii_val; i++) {
            append_char(compiler, '+');
        }
    } else if (trimmed[0] == '"' && trimmed[strlen(trimmed) - 1] == '"') {
        // String literal
        char content[MAX_LINE_LENGTH];
        strncpy(content, trimmed + 1, strlen(trimmed) - 2);
        content[strlen(trimmed) - 2] = '\0';
        
        char processed[MAX_LINE_LENGTH];
        int processed_len = process_escape_sequences(content, processed);
        
        for (int i = 0; i < processed_len; i++) {
            if (i > 0) {
                append_char(compiler, '>');  // Move to next cell
            }
            int ascii_val = (unsigned char)processed[i];
            append_output(compiler, "[-]");  // Clear current cell
            for (int j = 0; j < ascii_val; j++) {
                append_char(compiler, '+');
            }
        }
    } else if (strspn(trimmed, "0123456789") == strlen(trimmed)) {
        // Integer literal
        int num = atoi(trimmed);
        if (num > 255) {
            return -1; // Value too large for a single cell
        }
        append_output(compiler, "[-]");  // Clear current cell
        for (int i = 0; i < num; i++) {
            append_char(compiler, '+');
        }
    } else {
        return -1; // Invalid value for ent
    }
    
    return 0;
}

// Handle cellin statement
int handle_cellin(Compiler *compiler) {
    append_output(compiler, "[-]");  // Clear current cell
    append_char(compiler, ',');
    return 0;
}

// Handle cellout statement
int handle_cellout(Compiler *compiler) {
    append_char(compiler, '.');
    return 0;
}

// Handle shift operators
int handle_shift(Compiler *compiler, const char *direction) {
    append_char(compiler, direction[0]);
    return 0;
}

// Handle multi-shift operators
int handle_multi_shift(Compiler *compiler, const char *shift_str) {
    char direction;
    int num_shifts;
    
    if (strncmp(shift_str, "<<", 2) == 0) {
        direction = '<';
        num_shifts = atoi(shift_str + 2);
    } else if (strncmp(shift_str, ">>", 2) == 0) {
        direction = '>';
        num_shifts = atoi(shift_str + 2);
    } else {
        return -1; // Invalid shift operator
    }
    
    if (num_shifts <= 0) {
        return -1; // Number of shifts must be positive
    }
    
    for (int i = 0; i < num_shifts; i++) {
        append_char(compiler, direction);
    }
    
    return 0;
}

// Handle inc statement
int handle_inc(Compiler *compiler, const char *inc_str) {
    if (strcmp(inc_str, "inc") == 0) {
        append_char(compiler, '+');
    } else if (strncmp(inc_str, "inc ", 4) == 0) {
        const char *amount_str = inc_str + 4;
        char *trimmed = trim_whitespace((char*)amount_str);
        
        if (strspn(trimmed, "0123456789") != strlen(trimmed)) {
            return -1; // Invalid increment amount
        }
        
        int amount = atoi(trimmed);
        if (amount <= 0) {
            return -1; // Increment amount must be positive
        }
        
        for (int i = 0; i < amount; i++) {
            append_char(compiler, '+');
        }
    } else {
        return -1; // Invalid inc statement
    }
    
    return 0;
}

// Handle dec statement
int handle_dec(Compiler *compiler, const char *dec_str) {
    if (strcmp(dec_str, "dec") == 0) {
        append_char(compiler, '-');
    } else if (strncmp(dec_str, "dec ", 4) == 0) {
        const char *amount_str = dec_str + 4;
        char *trimmed = trim_whitespace((char*)amount_str);
        
        if (strspn(trimmed, "0123456789") != strlen(trimmed)) {
            return -1; // Invalid decrement amount
        }
        
        int amount = atoi(trimmed);
        if (amount <= 0) {
            return -1; // Decrement amount must be positive
        }
        
        for (int i = 0; i < amount; i++) {
            append_char(compiler, '-');
        }
    } else {
        return -1; // Invalid dec statement
    }
    
    return 0;
}

// Handle clr statement
int handle_clr(Compiler *compiler) {
    append_output(compiler, "[-]");
    return 0;
}

// Process a single line of BrainBang code
int process_line(Compiler *compiler, int indent_level, const char *content, int line_num) {
    if (strlen(content) == 0) {
        return 0;
    }
    
    // Handle loop end based on indentation
    while (compiler->loop_stack_size > 0 && 
           indent_level <= compiler->loop_stack[compiler->loop_stack_size - 1]) {
        append_char(compiler, ']');
        compiler->loop_stack_size--;
    }
    
    // Remove trailing semicolon for processing
    char content_copy[MAX_LINE_LENGTH];
    strcpy(content_copy, content);
    if (content_copy[strlen(content_copy) - 1] == ';') {
        content_copy[strlen(content_copy) - 1] = '\0';
    }
    
    // Parse the statement
    if (strcmp(content_copy, "loop:") == 0) {
        return handle_loop(compiler, indent_level);
    } else if (strncmp(content_copy, "ent ", 4) == 0) {
        return handle_ent(compiler, content_copy + 4);
    } else if (strcmp(content_copy, "cellin") == 0) {
        return handle_cellin(compiler);
    } else if (strcmp(content_copy, "cellout") == 0) {
        return handle_cellout(compiler);
    } else if (strcmp(content_copy, "<") == 0 || strcmp(content_copy, ">") == 0) {
        return handle_shift(compiler, content_copy);
    } else if (strncmp(content_copy, "<<", 2) == 0 || strncmp(content_copy, ">>", 2) == 0) {
        return handle_multi_shift(compiler, content_copy);
    } else if (strncmp(content_copy, "inc", 3) == 0) {
        return handle_inc(compiler, content_copy);
    } else if (strncmp(content_copy, "dec", 3) == 0) {
        return handle_dec(compiler, content_copy);
    } else if (strcmp(content_copy, "clr") == 0) {
        return handle_clr(compiler);
    } else {
        printf("Error on line %d: Unknown statement: %s\n", line_num, content_copy);
        return -1;
    }
}

// Compile BrainBang source code to Brainfuck
int compiler_compile(Compiler *compiler, const char *source_code, char **result) {
    ProcessedLine lines[MAX_LINES];
    int line_count;
    
    compiler->output[0] = '\0';
    compiler->output_size = 0;
    compiler->loop_stack_size = 0;
    
    if (preprocess_lines(source_code, lines, &line_count) != 0) {
        return -1;
    }
    
    for (int i = 0; i < line_count; i++) {
        if (process_line(compiler, lines[i].indent_level, lines[i].content, i + 1) != 0) {
            return -1;
        }
    }
    
    // Close any remaining loops
    while (compiler->loop_stack_size > 0) {
        append_char(compiler, ']');
        compiler->loop_stack_size--;
    }
    
    *result = malloc(strlen(compiler->output) + 1);
    if (*result) {
        strcpy(*result, compiler->output);
    }
    
    return 0;
}

// Read file contents
char *read_file(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        return NULL;
    }
    
    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    fseek(file, 0, SEEK_SET);
    
    char *content = malloc(size + 1);
    if (content) {
        size_t bytes_read = fread(content, 1, size, file);
        content[bytes_read] = '\0';
        if (bytes_read != (size_t)size && !feof(file)) {
            // Read error occurred
            free(content);
            fclose(file);
            return NULL;
        }
    }

    fclose(file);
    return content;
}

// Write file contents
int write_file(const char *filename, const char *content) {
    FILE *file = fopen(filename, "w");
    if (!file) {
        return -1;
    }
    
    size_t len = strlen(content);
    size_t written = fwrite(content, 1, len, file);
    fclose(file);
    
    return (written == len) ? 0 : -1;
}

// Main function
int compile(const char* source_file, const char* output_file) {
    char *source_code = read_file(source_file);
    if (!source_code) {
        printf("Error: File '%s' not found\n", source_file);
        return 1;
    }

    Compiler *compiler = compiler_new();
    if (!compiler) {
        printf("Error: Failed to create compiler\n");
        free(source_code);
        return 1;
    }

    char *brainfuck_code;
    if (compiler_compile(compiler, source_code, &brainfuck_code) != 0) {
        printf("Compilation failed\n");
        compiler_free(compiler);
        free(source_code);
        return 1;
    }

    if (write_file(output_file, brainfuck_code) != 0) {
        printf("Error: Failed to write output file '%s'\n", output_file);
        free(brainfuck_code);
        compiler_free(compiler);
        free(source_code);
        return 1;
    }


    // Cleanup
    free(brainfuck_code);
    compiler_free(compiler);
    free(source_code);

    return 0;
}
