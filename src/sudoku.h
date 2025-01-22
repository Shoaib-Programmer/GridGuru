#ifndef SUDOKU_H
#define SUDOKU_H

#define SIZE 9

// Function declarations
bool is_valid(int grid[SIZE][SIZE]);
bool solveSudoku(int grid[SIZE][SIZE]);
void printGrid(int grid[SIZE][SIZE]);
void inputSudoku(int grid[SIZE][SIZE]);

#endif // SUDOKU_H
