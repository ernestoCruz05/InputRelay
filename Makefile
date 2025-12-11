CC = gcc
CFLAGS = -Wall -Wextra -I./src -O2
LIBS = -lncurses

SRC_DIR = src
OBJ_DIR = obj
BIN = inputrelay

SRCS := $(shell find $(SRC_DIR) -name '*.c')
OBJS := $(SRCS:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)

all: $(BIN)

$(BIN): $(OBJS)
	@echo "Linking $@"
	@$(CC) $(OBJS) -o $@ $(LIBS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	@echo "Compiling $<"
	@$(CC) $(CFLAGS) -c $< -o $@

clean:
	@echo "Cleaning files..."
	@rm -rf $(OBJ_DIR) $(BIN)

.PHONY: all clean
