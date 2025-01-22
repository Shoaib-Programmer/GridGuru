// main.c
#include <stdbool.h>
#include <gtk/gtk.h>
#include "sudoku.h"

#define SIZE 9

GtkWidget *entries[SIZE][SIZE]; // Array to hold the entry widgets
GtkWidget *solve_button, *back_button, *reset_button;
GtkWidget *label;             // Label to show messages (e.g., "No solution exists.")
int initial_grid[SIZE][SIZE]; // Store initial grid for reset functionality

void on_solve_button_clicked(GtkWidget *widget, gpointer data);
void on_reset_button_clicked(GtkWidget *widget, gpointer data);
void on_back_button_clicked(GtkWidget *widget, gpointer data);

// Function to get the Sudoku grid from the GTK entries
void get_grid_from_entries(int grid[SIZE][SIZE]) {
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            const gchar *text = gtk_entry_get_text(GTK_ENTRY(entries[i][j]));
            grid[i][j] = (text[0] == '\0' || text[0] < '1' || text[0] > '9') ? 0 : text[0] - '0';
        }
    }
}

// Function to display the grid in the GTK entry widgets
void set_grid_in_entries(int grid[SIZE][SIZE]) {
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            gchar text[2] = {grid[i][j] ? grid[i][j] + '0' : '\0', '\0'};
            gtk_entry_set_text(GTK_ENTRY(entries[i][j]), text);
        }
    }
}

// Function to create the GUI window
GtkWidget *create_main_window() {
    GtkWidget *window, *grid;
    GtkWidget *hbox, *vbox;

    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Sudoku Solver");
    gtk_window_set_default_size(GTK_WINDOW(window), 450, 550);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    grid = gtk_grid_new();
    gtk_container_add(GTK_CONTAINER(window), grid);

    // Create a label to display messages (like "No solution exists")
    label = gtk_label_new("");
    gtk_grid_attach(GTK_GRID(grid), label, 0, SIZE, 9, 1);

    // Create the grid of entry widgets (9x9)
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            entries[i][j] = gtk_entry_new();
            gtk_entry_set_width_chars(GTK_ENTRY(entries[i][j]), 2);
            gtk_entry_set_max_length(GTK_ENTRY(entries[i][j]), 1);
            if ((i / 3 + j / 3) % 2 == 0) {
                gtk_widget_override_background_color(entries[i][j], GTK_STATE_FLAG_NORMAL, &(GdkRGBA){0.9, 0.9, 0.9, 1.0});
            }
            // Add border to separate the 9 sub-grids
            if (i % 3 == 2 || j % 3 == 2) {
                gtk_widget_set_margin_end(entries[i][j], 2);
                gtk_widget_set_margin_bottom(entries[i][j], 2);
            }
            gtk_grid_attach(GTK_GRID(grid), entries[i][j], j, i, 1, 1);
        }
    }

    // Solve button
    solve_button = gtk_button_new_with_label("Solve");
    g_signal_connect(solve_button, "clicked", G_CALLBACK(on_solve_button_clicked), NULL);
    gtk_grid_attach(GTK_GRID(grid), solve_button, 0, SIZE + 1, 3, 1);

    // Reset button
    reset_button = gtk_button_new_with_label("Reset");
    g_signal_connect(reset_button, "clicked", G_CALLBACK(on_reset_button_clicked), NULL);
    gtk_grid_attach(GTK_GRID(grid), reset_button, 3, SIZE + 1, 3, 1);

    // Back button (hidden initially)
    back_button = gtk_button_new_with_label("Back");
    g_signal_connect(back_button, "clicked", G_CALLBACK(on_back_button_clicked), NULL);
    gtk_grid_attach(GTK_GRID(grid), back_button, 6, SIZE + 1, 3, 1);
    gtk_widget_set_visible(back_button, FALSE); // Hide initially

    return window;
}

// Callback function for the "Solve" button
void on_solve_button_clicked(GtkWidget *widget, gpointer data) {
    (void)widget;
    (void)data;
    int grid[SIZE][SIZE];

    // Get grid data from the GTK entries
    get_grid_from_entries(grid);

    // Save the user's input as the new initial state
    store_initial_grid(grid); // <-- Added this line

    // Check if the grid is valid
    if (!is_valid(grid)) {
        gtk_label_set_text(GTK_LABEL(label), "Invalid Sudoku grid.");
        return;
    }

    // Solve the Sudoku puzzle
    if (solveSudoku(grid)) {
        // Display the solution in the entries
        set_grid_in_entries(grid);
        gtk_widget_set_visible(solve_button, FALSE); // Hide solve button
        gtk_widget_set_visible(back_button, TRUE);   // Show back button
        gtk_label_set_text(GTK_LABEL(label), "Solution found!");
    } else {
        gtk_label_set_text(GTK_LABEL(label), "No solution exists.");
    }
}

// Callback for the "Back" button
void on_back_button_clicked(GtkWidget *widget, gpointer data) {
    (void)widget;
    (void)data;
    set_grid_in_entries(initial_grid);          // Set the board to initial state
    gtk_widget_set_visible(solve_button, TRUE); // Show solve button
    gtk_widget_set_visible(back_button, FALSE); // Hide back button
    gtk_label_set_text(GTK_LABEL(label), "");   // Clear the label
}

// main.c (updated sections)

// ...

// Callback for the "Reset" button
void on_reset_button_clicked(GtkWidget *widget, gpointer data) {
    (void)widget;
    (void)data;
    int grid[SIZE][SIZE] = {0}; // Reset to all zeros
    set_grid_in_entries(grid);
    gtk_label_set_text(GTK_LABEL(label), "");
}

// Store the initial state of the grid for the "Back" button
void store_initial_grid(int grid[SIZE][SIZE]) { // <-- Changed to take a parameter
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            initial_grid[i][j] = grid[i][j];
        }
    }
}

int main(int argc, char *argv[]) {
    gtk_init(&argc, &argv);

    // Initialize `initial_grid` with the predefined board
    int initial_state[SIZE][SIZE] = {
        {5, 3, 0, 0, 7, 0, 0, 0, 0},
        {6, 0, 0, 1, 9, 5, 0, 0, 0},
        {0, 9, 8, 0, 0, 0, 0, 6, 0},
        {8, 0, 0, 0, 6, 0, 0, 0, 3},
        {4, 0, 0, 8, 0, 3, 0, 0, 1},
        {7, 0, 0, 0, 2, 0, 0, 0, 6},
        {0, 6, 0, 0, 0, 0, 2, 8, 0},
        {0, 0, 0, 4, 1, 9, 0, 0, 5},
        {0, 0, 0, 0, 8, 0, 0, 7, 9}};

    // Save the predefined board to `initial_grid`
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            initial_grid[i][j] = initial_state[i][j];
        }
    }

    // Create the main window
    GtkWidget *window = create_main_window();

    // Load the predefined board into the GUI
    set_grid_in_entries(initial_grid);

    // Show the window
    gtk_widget_show_all(window);

    gtk_main();

    return 0;
}