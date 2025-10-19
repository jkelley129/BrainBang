# Directories and tools
BUILD_DIR = build
OUTPUT_DIR = dist
SRC = src
OUTPUT = $(OUTPUT_DIR)/brainbang

CC = gcc
CFLAGS = -Wall -Wextra -O2
LDFLAGS =

# Source files and object files
SRCS = $(SRC)/brainbang.c $(SRC)/compiler.c $(SRC)/brainfk.c
OBJS = $(patsubst $(SRC)/%.c, $(BUILD_DIR)/%.o, $(SRCS))

# Default target
all: $(OUTPUT)

# Link
$(OUTPUT): $(OBJS) | $(OUTPUT_DIR)
	$(CC) $(LDFLAGS) -o $@ $^

# Compile each .c file into .o file in build/
$(BUILD_DIR)/%.o: $(SRC)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Ensure build directory exists
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(OUTPUT_DIR):
	mkdir -p $(OUTPUT_DIR)

# Clean rule
clean:
	rm -rf $(BUILD_DIR) $(OUTPUT_DIR)

clean-all:
	rm -rf $(BUILD_DIR) $(OUTPUT_DIR) *.bf

clean-objs:
	rm -rf $(BUILD_DIR)

clean-output:
	rm -rf $(OUTPUT_DIR)
.PHONY: all clean
