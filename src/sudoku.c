// sudoku.c
// Helper functions for Sudoku. For the application's entry point using the UI, refer to main.c
#include <stdbool.h>
#include <stdio.h>

#define SIZE 9 


// Check if the current Sudoku grid is valid
bool isValid(int grid[SIZE][SIZE]) {
    // Check each row
    for (int i = 0; i < SIZE; i++) {
        bool used[SIZE + 1] = {false}; // Array to track used numbers
        for (int j = 0; j < SIZE; j++) {
            int num = grid[i][j];
            if (num != 0) {
                if (used[num]) {
                    return false; // Duplicate found
                }
                used[num] = true;
            }
        }
    }

    // Check each column
    for (int j = 0; j < SIZE; j++) {
        bool used[SIZE + 1] = {false};
        for (int i = 0; i < SIZE; i++) {
            int num = grid[i][j];
            if (num != 0) {
                if (used[num]) {
                    return false; // Duplicate found
                }
                used[num] = true;
            }
        }
    }

    // Check each 3x3 sub-grid
    for (int blockRow = 0; blockRow < 3; blockRow++) {
        for (int blockCol = 0; blockCol < 3; blockCol++) {
            bool used[SIZE + 1] = {false};
            for (int i = 0; i < 3; i++) {
                for (int j = 0; j < 3; j++) {
                    int num = grid[blockRow * 3 + i][blockCol * 3 + j];
                    if (num != 0) {
                        if (used[num]) {
                            return false; // Duplicate found
                        }
                        used[num] = true;
                    }
                }
            }
        }
    }

    return true; // Grid is valid
}

// Solve the Sudoku using backtracking
bool solveSudoku(int grid[SIZE][SIZE]) {
    int row = -1, col = -1;
    bool isEmpty = false;

    // Find an empty cell
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            if (grid[i][j] == 0) {
                row = i;
                col = j;
                isEmpty = true;
                break;
            }
        }
        if (isEmpty) break;
    }

    // No empty cells left, puzzle is solved
    if (!isEmpty) {
        return true;
    }

    // Try numbers 1 through 9
    for (int num = 1; num <= 9; num++) {
        if (isValid(grid)) {
            grid[row][col] = num;

            // Recurse with this choice
            if (solveSudoku(grid)) {
                return true;
            }

            // Backtrack if this choice doesn't work
            grid[row][col] = 0;
        }
    }

    return false; // Trigger backtracking
}

// Utility to print the grid with separators
void printGrid(int grid[SIZE][SIZE]) {
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            if (j % 3 == 0 && j != 0) {
                printf("| "); // Print vertical separator every 3 columns
            }
            printf("%d ", grid[i][j]);
        }

        // Print horizontal separator every 3 rows
        if ((i + 1) % 3 == 0 && i != SIZE - 1) {
            printf("\n---------------------\n");
        } else {
            printf("\n");
        }
    }
}

// Prompt the user for the existing Sudoku values
void inputSudoku(int grid[SIZE][SIZE]) {
    printf("Enter the Sudoku puzzle (0 for empty cells):\n");

    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            printf("Enter value for cell (%d, %d): ", i + 1, j + 1);
            scanf("%d", &grid[i][j]);
        }
    }
}
