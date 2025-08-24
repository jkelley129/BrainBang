#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#ifdef _WIN32
    #define PATH_SEP "\\"
#else
    #define PATH_SEP "/"
#endif

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
        strncpy(dest, src, len - ext_len);
        dest[len - ext_len] = '\0';
    } else {
        strcpy(dest, src);  // fallback
    }
}

int main(int argc, char *argv[]) {
    bool run_compiled = true;
    const char *filename = NULL;

    // --- Parse arguments ---
    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "-c") == 0) {
            run_compiled = false;
        } else if (filename == NULL) {
            filename = argv[i];
        } else {
            fprintf(stderr, "Unexpected argument: %s\n", argv[i]);
        }
    }

    // --- Validate input ---
    if (filename == NULL) {
        fprintf(stderr, "Usage: %s [-c] source.bb\n", argv[0]);
        return 1;
    }

    if (!ends_with(filename, ".bb")) {
        fprintf(stderr, "Error: source file must end with '.bb'\n");
        return 1;
    }

    // --- Compile with Python compiler ---
    char command[512];
    snprintf(command, sizeof(command),
             "build%sbrainbang_compiler \"%s\"",
             PATH_SEP, filename);

    int compile_status = system(command);
    if (compile_status != 0) {
        fprintf(stderr, "Error: Compilation failed.\n");
        return compile_status;
    }

    // --- Get base filename without .bb extension ---
    char base_filename[256];
    remove_extension(base_filename, filename, ".bb");

    // --- Optionally run compiled .bf file ---
    if (run_compiled) {
        snprintf(command, sizeof(command),
                 "build%sbrainfk \"%s.bf\"",
                 PATH_SEP, base_filename);

        return system(command);
    }

    return 0;
}

