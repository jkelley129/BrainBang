#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include "compiler.h"
#include "brainfk.h"

#ifdef _WIN32
    #define PATH_SEP "\\"
#else
    #define PATH_SEP "/"
#endif

// Print help information
void print_help(const char *program_name) {
    printf("BrainBang - A human-readable Brainfuck compiler\n\n");
    printf("Usage:\n");
    printf("  %s <file.bb>                    Compile and run a BrainBang file\n", program_name);
    printf("  %s compile <file.bb> [-o <output>]  Compile only (no execution)\n", program_name);
    printf("  %s run <file.bf>                Run a compiled Brainfuck file\n", program_name);
    printf("  %s help                         Show this help message\n\n", program_name);
    printf("Options:\n");
    printf("  -o <output>   Specify output file path (default: <filename>.bf in current directory)\n\n");
    printf("Examples:\n");
    printf("  %s program.bb                   # Compile and run\n", program_name);
    printf("  %s compile program.bb           # Compile to program.bf in current directory\n", program_name);
    printf("  %s compile program.bb -o out.bf # Compile to out.bf\n", program_name);
    printf("  %s run program.bf               # Run compiled file\n\n", program_name);
}

// Utility: check if string ends with suffix
int ends_with(const char *str, const char *suffix) {
    size_t len_str = strlen(str);
    size_t len_suffix = strlen(suffix);
    return (len_str >= len_suffix) && (strcmp(str + len_str - len_suffix, suffix) == 0);
}

// Utility: remove file extension (like ".bb")
void remove_extension(char *dest, const char *src, const char *ext) {
    size_t len = strlen(src);
    size_t ext_len = strlen(ext);
    if (len > ext_len && strcmp(src + len - ext_len, ext) == 0) {
        memcpy(dest, src, len - ext_len);
        dest[len - ext_len] = '\0';
    } else {
        strcpy(dest, src);  // fallback
    }
}

// Utility: extract filename from a path (cross-platform)
void get_filename_from_path(char *dest, const char *path) {
    const char *last_sep = strrchr(path, '/');
    const char *last_sep_win = strrchr(path, '\\');

    // Get the rightmost separator
    if (last_sep_win && (!last_sep || last_sep_win > last_sep)) {
        last_sep = last_sep_win;
    }

    if (last_sep) {
        strcpy(dest, last_sep + 1);
    } else {
        strcpy(dest, path);
    }
}

int main(int argc, char *argv[]) {
    const char *command = NULL;
    const char *filename = NULL;
    const char *output_file = NULL;

    // --- Parse arguments ---
    if (argc < 2) {
        fprintf(stderr, "Error: No input specified\n");
        fprintf(stderr, "Try '%s help' for more information.\n", argv[0]);
        return 1;
    }

    // Check if first argument is a command or a file
    if (strcmp(argv[1], "compile") == 0) {
        command = "compile";
        if (argc < 3) {
            fprintf(stderr, "Error: No file specified for compile command\n");
            fprintf(stderr, "Usage: %s compile <file.bb> [-o <output>]\n", argv[0]);
            return 1;
        }
        filename = argv[2];

        // Parse -o option for compile command
        if (argc >= 5 && strcmp(argv[3], "-o") == 0) {
            output_file = argv[4];
        } else if (argc > 3 && strcmp(argv[3], "-o") != 0) {
            fprintf(stderr, "Error: Unknown option '%s'\n", argv[3]);
            fprintf(stderr, "Try '%s help' for more information.\n", argv[0]);
            return 1;
        } else if (argc > 5) {
            fprintf(stderr, "Error: Too many arguments\n");
            fprintf(stderr, "Try '%s help' for more information.\n", argv[0]);
            return 1;
        }
    } else if (strcmp(argv[1], "run") == 0) {
        command = "run";
        if (argc < 3) {
            fprintf(stderr, "Error: No file specified for run command\n");
            fprintf(stderr, "Usage: %s run <file.bf>\n", argv[0]);
            return 1;
        }
        filename = argv[2];

        if (argc > 3) {
            fprintf(stderr, "Error: Too many arguments\n");
            fprintf(stderr, "Try '%s help' for more information.\n", argv[0]);
            return 1;
        }
    } else if (strcmp(argv[1], "help") == 0) {
        print_help(argv[0]);
        return 0;
    } else {
        // No command specified, assume file is given directly
        command = "default";
        filename = argv[1];

        if (argc > 2) {
            fprintf(stderr, "Error: Too many arguments\n");
            fprintf(stderr, "Try '%s help' for more information.\n", argv[0]);
            return 1;
        }
    }

    // --- Execute based on command ---
    if (strcmp(command, "run") == 0) {
        // Run only mode - expecting a .bf file
        if (!ends_with(filename, ".bf")) {
            fprintf(stderr, "Error: File must end with '.bf' for run command\n");
            return 1;
        }

        if (bf_run(filename) != 0) {
            fprintf(stderr, "Error: Failed to run %s\n", filename);
            return 1;
        }
    } else {
        // Compile mode or default mode - expecting a .bb file
        if (!ends_with(filename, ".bb")) {
            fprintf(stderr, "Error: Source file must end with '.bb'\n");
            return 1;
        }

        // Determine output filename
        char bf_filename[512];
        if (output_file) {
            // Use user-specified output file
            snprintf(bf_filename, sizeof(bf_filename), "%s", output_file);
        } else {
            // Default: extract just the filename (not full path) and place in current directory
            char just_filename[256];
            get_filename_from_path(just_filename, filename);

            char base_filename[256];
            remove_extension(base_filename, just_filename, ".bb");

            snprintf(bf_filename, sizeof(bf_filename), "%s.bf", base_filename);
        }

        printf("Compiling %s...\n", filename);
        if (compile(filename, bf_filename) != 0) {
            fprintf(stderr, "Error: Compilation failed\n");
            return 1;
        }

        printf("Generated %s\n", bf_filename);

        // Run the compiled file if in default mode
        if (strcmp(command, "default") == 0) {
            printf("Running %s...\n", bf_filename);
            if (bf_run(bf_filename) != 0) {
                fprintf(stderr, "Error: Failed to run %s\n", bf_filename);
                return 1;
            }
        }
    }

    return 0;
}