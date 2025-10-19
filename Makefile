# Directories and tools
BUILD_DIR = build
SRC = src
OUTPUT = brainbang

CC = gcc
CFLAGS = -Wall -Wextra -O2
LDFLAGS =

# Source files and object files
SRCS = $(SRC)/brainbang.c $(SRC)/compiler.c $(SRC)/brainfk.c
OBJS = $(patsubst $(SRC)/%.c, $(BUILD_DIR)/%.o, $(SRCS))

# Default target
all: $(OUTPUT)

# Link
$(OUTPUT): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $^

# Compile each .c file into .o file in build/
$(BUILD_DIR)/%.o: $(SRC)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Ensure build directory exists
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Clean rule
clean:
	rm -rf $(BUILD_DIR) $(OUTPUT)

.PHONY: all clean
