# Final Makefile
CC = gcc
CFLAGS = -Wall -Wextra -Wpedantic -I./src
LDFLAGS = 
LIBS = `pkg-config --libs gtk+-3.0`
GTK_FLAGS = `pkg-config --cflags gtk+-3.0`
OBJ_DIR = obj
BIN_DIR = bin
SRC_DIR = src
SOURCES = $(SRC_DIR)/main.c $(SRC_DIR)/sudoku.c
OBJECTS = $(SOURCES:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)
EXEC = $(BIN_DIR)/grid_guru

RES_DIR = resources
INSTALL_PREFIX = /usr/local

# Combine all flags
CFLAGS += $(GTK_FLAGS)
LDFLAGS += $(LIBS)

# Ensure directories exist
_DUMMY := $(shell mkdir -p $(OBJ_DIR) $(BIN_DIR))

.PHONY: all clean debug install uninstall

all: $(EXEC)

# Main build rule
$(EXEC): $(OBJECTS)
	$(CC) $^ -o $@ $(LDFLAGS)

# Compilation rule for object files
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# Debug build with symbols
debug: CFLAGS += -g -O0 -DDEBUG
debug: clean all

# Install to /usr/local/bin (optional)
install:
	@mkdir -p $(INSTALL_PREFIX)/bin
	@mkdir -p $(INSTALL_PREFIX)/share/grid-guru/resources
	@cp $(EXEC) $(INSTALL_PREFIX)/bin/grid-guru
	@cp -r $(RES_DIR)/* $(INSTALL_PREFIX)/share/grid-guru/resources
	@echo "Installed to $(INSTALL_PREFIX)"

# Uninstall from system
uninstall:
	@rm -f /usr/local/bin/grid-guru

# Deep clean
clean:
	rm -rf $(OBJ_DIR)/*.o $(EXEC)
	@echo "Cleaned build artifacts"
