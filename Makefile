CC = gcc
CFLAGS = `pkg-config --cflags gtk+-3.0` -Wall -Wextra
LIBS = `pkg-config --libs gtk+-3.0`
OBJ = main.o sudoku.o
EXEC = sudoku_solver

# Default target to build the program
all: $(EXEC)

$(EXEC): $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) -o $(EXEC) $(LIBS)

# Compile main.o
main.o: src/main.c src/sudoku.h
	$(CC) $(CFLAGS) -c src/main.c

# Compile sudoku.o
sudoku.o: src/sudoku.c src/sudoku.h
	$(CC) $(CFLAGS) -c src/sudoku.c

# Clean up object files and executable
clean:
	rm -f $(OBJ) $(EXEC)
