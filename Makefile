# Compiler and compiler flags
CC := gcc
CFLAGS :=# -Wall -Wextra
DEBUG_CFLAGS := -g

# Directories
SRC_DIR := src
OBJ_DIR := obj

# Source files
SRCS := $(wildcard $(SRC_DIR)/*.c)
OBJS := $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SRCS))

# Target binary
TARGET := compiler

# Phony targets
.PHONY: all debug clean

# Default target
all: $(TARGET)

# Debug target
debug: CFLAGS += $(DEBUG_CFLAGS)
debug: all

# Compile and build the target binary
$(TARGET): $(OBJS) $(MAIN_OBJ)
	$(CC) $(CFLAGS) $^ -o $@

# Compile source files into object files (excluding main.c)
$(OBJS): $(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Clean the object files and the target binary
clean:
	rm -rf $(OBJ_DIR) $(TARGET)