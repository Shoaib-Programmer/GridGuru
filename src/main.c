// main.c
#include <stdbool.h>
#include "sudoku.h"
#include <libgen.h> // For dirname()
#include <unistd.h> // For readlink()
#include <limits.h> // For PATH_MAX
#include <string.h> // For memcpy

#include <gtk/gtk.h>
#include <gdk-pixbuf/gdk-pixbuf.h>

#define SIZE 9

GtkWidget *entries[SIZE][SIZE];
GtkWidget *solve_button, *back_button, *reset_button;
GtkWidget *label;
GtkWidget *window; // Global main window pointer
int initial_grid[SIZE][SIZE];

bool dark_mode_enabled = false; // Global flag to track dark mode state
GtkWidget *theme_icon;
char *light_icon_path = "resources/dark.png"; // Show dark icon in light mode
char *dark_icon_path =  "resources/light.png"; // Show light icon in dark mode

// Fallback CSS paths
#define INSTALL_PREFIX "/usr/local" // Defined here or via Makefile
#define RELATIVE_CSS_PATH "resources/styles.css"

void on_solve_button_clicked(GtkWidget *widget, gpointer data);
void on_reset_button_clicked(GtkWidget *widget, gpointer data);
void on_back_button_clicked(GtkWidget *widget, gpointer data);
void store_initial_grid(int grid[SIZE][SIZE]);
gboolean clear_status_message(); // Change to match definition

// Loads the CSS file for styling
void load_css()
{
    GtkCssProvider *provider = gtk_css_provider_new();
    GError *error = NULL;
    gchar *css_path = NULL;

    // 1. Try relative path first (for development)
    if (g_file_test(RELATIVE_CSS_PATH, G_FILE_TEST_EXISTS))
    {
        css_path = g_strdup(RELATIVE_CSS_PATH);
    }
    // 2. Try installed location
    else
    {
        css_path = g_strdup_printf("%s/share/grid-guru/resources/styles.css", INSTALL_PREFIX);
    }

    if (!gtk_css_provider_load_from_path(provider, css_path, &error))
    {
        g_printerr("CSS loading failed: %s\n", error ? error->message : "Unknown error");
        if (error)
            g_error_free(error);
        g_free(css_path);
        return;
    }

    GdkScreen *screen = gdk_screen_get_default();
    gtk_style_context_add_provider_for_screen(
        screen,
        GTK_STYLE_PROVIDER(provider),
        GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

    g_free(css_path);
}

// Toggle dark mode by adding or removing the "dark" CSS class from the main window.
void toggle_dark_mode(GtkWidget *widget, gpointer data)
{
    (void)widget;
    (void)data;

    GtkStyleContext *context = gtk_widget_get_style_context(window);
    if (dark_mode_enabled)
    {
        gtk_style_context_remove_class(context, "dark");
        gtk_image_set_from_file(GTK_IMAGE(theme_icon), light_icon_path);
        dark_mode_enabled = false;
    }
    else
    {
        gtk_style_context_add_class(context, "dark");
        gtk_image_set_from_file(GTK_IMAGE(theme_icon), dark_icon_path);
        dark_mode_enabled = true;
    }
}

// Function to get the Sudoku grid from the GTK entries
void get_grid_from_entries(int grid[SIZE][SIZE])
{
    for (int i = 0; i < SIZE; i++)
    {
        for (int j = 0; j < SIZE; j++)
        {
            const gchar *text = gtk_entry_get_text(GTK_ENTRY(entries[i][j]));
            grid[i][j] = (text[0] == '\0' || text[0] < '1' || text[0] > '9') ? 0 : text[0] - '0';
        }
    }
}

// Function to display the grid in the GTK entry widgets
void set_grid_in_entries(int grid[SIZE][SIZE])
{
    for (int i = 0; i < SIZE; i++)
    {
        for (int j = 0; j < SIZE; j++)
        {
            gchar text[2] = {grid[i][j] ? grid[i][j] + '0' : '\0', '\0'};
            gtk_entry_set_text(GTK_ENTRY(entries[i][j]), text);
        }
    }
}

static guint status_timeout_id = 0;

void set_status_message(const char *message, const char *style_class)
{
    // Clear previous timeout
    if (status_timeout_id > 0)
    {
        g_source_remove(status_timeout_id);
        status_timeout_id = 0;
    }

    // Clear existing classes
    GtkStyleContext *context = gtk_widget_get_style_context(label);
    gtk_style_context_remove_class(context, "success");
    gtk_style_context_remove_class(context, "error");

    // Set new message and class
    gtk_label_set_text(GTK_LABEL(label), message);
    if (style_class)
    {
        gtk_style_context_add_class(context, style_class);
    }

    // Set timeout to clear message after 3 seconds
    status_timeout_id = g_timeout_add_seconds(3, (GSourceFunc)clear_status_message, NULL);
}

gboolean clear_status_message()
{
    gtk_label_set_text(GTK_LABEL(label), "");
    GtkStyleContext *context = gtk_widget_get_style_context(label);
    gtk_style_context_remove_class(context, "success");
    gtk_style_context_remove_class(context, "error");
    status_timeout_id = 0;
    return G_SOURCE_REMOVE;
}

// Function to create the GUI window (CSS-ready version)
GtkWidget *create_main_window()
{
    // Use the global window variable (do not shadow it locally)
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Sudoku Solver");
    gtk_window_set_default_size(GTK_WINDOW(window), 500, 600);
    gtk_widget_set_name(window, "main-window"); // CSS ID for main window
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    // Main vertical container
    GtkWidget *main_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_container_add(GTK_CONTAINER(window), main_box);

    // Create header box for title and theme toggle
    GtkWidget *header_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_widget_set_name(header_box, "header-box");
    gtk_box_pack_start(GTK_BOX(main_box), header_box, FALSE, FALSE, 15);

    // Add title to header box
    GtkWidget *title = gtk_label_new("SUDOKU SOLVER");
    gtk_widget_set_name(title, "app-title");
    gtk_label_set_xalign(GTK_LABEL(title), 0.5);
    gtk_box_pack_start(GTK_BOX(header_box), title, TRUE, TRUE, 0);

    // Create theme toggle button with icon
    GtkWidget *toggle_dark_button = gtk_button_new();
    gtk_widget_set_name(toggle_dark_button, "toggle-dark-button");
    theme_icon = gtk_image_new_from_file(light_icon_path);
    gtk_button_set_image(GTK_BUTTON(toggle_dark_button), theme_icon);
    gtk_box_pack_end(GTK_BOX(header_box), toggle_dark_button, FALSE, FALSE, 0);
    g_signal_connect(toggle_dark_button, "clicked", G_CALLBACK(toggle_dark_mode), NULL);

    // Sudoku grid container
    GtkWidget *grid = gtk_grid_new();
    gtk_widget_set_name(grid, "sudoku-grid");
    gtk_grid_set_row_spacing(GTK_GRID(grid), 2);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 2);
    gtk_widget_set_halign(grid, GTK_ALIGN_CENTER);
    gtk_box_pack_start(GTK_BOX(main_box), grid, TRUE, TRUE, 20);

    // Status label with initial CSS class
    label = gtk_label_new("Enter your puzzle and click Solve!");
    gtk_widget_set_name(label, "status-label");
    gtk_label_set_xalign(GTK_LABEL(label), 0.5);
    gtk_box_pack_start(GTK_BOX(main_box), label, FALSE, FALSE, 10);

    // Create Sudoku cells with proper CSS targeting
    for (int i = 0; i < SIZE; i++)
    {
        for (int j = 0; j < SIZE; j++)
        {
            entries[i][j] = gtk_entry_new();

            // Base CSS class for all cells
            gtk_widget_set_name(entries[i][j], "sudoku-cell");

            // Alternate block styling
            if ((i / 3 + j / 3) % 2 == 0)
            {
                gtk_style_context_add_class(gtk_widget_get_style_context(entries[i][j]), "alternate-block");
            }

            // Special CSS classes for borders
            if (j % 3 == 2 && j != 8)
            {
                gtk_style_context_add_class(gtk_widget_get_style_context(entries[i][j]), "right-border");
            }
            if (i % 3 == 2 && i != 8)
            {
                gtk_style_context_add_class(gtk_widget_get_style_context(entries[i][j]), "bottom-border");
            }

            // Cell properties
            gtk_entry_set_alignment(GTK_ENTRY(entries[i][j]), 0.5);
            gtk_widget_set_hexpand(entries[i][j], TRUE);
            gtk_widget_set_vexpand(entries[i][j], TRUE);
            gtk_entry_set_max_length(GTK_ENTRY(entries[i][j]), 1);

            gtk_grid_attach(GTK_GRID(grid), entries[i][j], j, i, 1, 1);
        }
    }

    // Button container with CSS ID
    GtkWidget *button_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_widget_set_name(button_box, "button-container");
    gtk_box_set_homogeneous(GTK_BOX(button_box), TRUE);
    gtk_box_pack_start(GTK_BOX(main_box), button_box, FALSE, FALSE, 15);

    // Solve Button
    solve_button = gtk_button_new_with_label("Solve");
    gtk_widget_set_name(solve_button, "solve-button");
    gtk_box_pack_start(GTK_BOX(button_box), solve_button, TRUE, TRUE, 0);

    // Reset Button
    reset_button = gtk_button_new_with_label("Reset");
    gtk_widget_set_name(reset_button, "reset-button");
    gtk_box_pack_start(GTK_BOX(button_box), reset_button, TRUE, TRUE, 0);

    // Back Button
    back_button = gtk_button_new_with_label("Back");
    gtk_widget_set_name(back_button, "back-button");
    gtk_box_pack_start(GTK_BOX(button_box), back_button, TRUE, TRUE, 0);
    gtk_widget_set_visible(back_button, FALSE);

    // Connect signals for primary buttons
    g_signal_connect(solve_button, "clicked", G_CALLBACK(on_solve_button_clicked), NULL);
    g_signal_connect(reset_button, "clicked", G_CALLBACK(on_reset_button_clicked), NULL);
    g_signal_connect(back_button, "clicked", G_CALLBACK(on_back_button_clicked), NULL);

    return window;
}

// Callback function for the "Solve" button
void on_solve_button_clicked(GtkWidget *widget, gpointer data)
{
    int grid[SIZE][SIZE];
    get_grid_from_entries(grid);
    store_initial_grid(grid);

    if (!isValid(grid))
    {
        set_status_message("Invalid Sudoku grid!", "error");
        return;
    }

    // Make a copy for solving
    int solve_grid[SIZE][SIZE];
    memcpy(solve_grid, grid, sizeof(grid[0][0]) * SIZE * SIZE);

    if (solveSudoku(solve_grid))
    {
        set_grid_in_entries(solve_grid); // Update with solved grid
        gtk_widget_set_visible(solve_button, FALSE);
        gtk_widget_set_visible(back_button, TRUE);
        set_status_message("Solution found!", "success");

        // Force UI refresh
        gtk_widget_queue_draw(gtk_widget_get_parent(window));
    }
    else
    {
        set_status_message("No solution exists.", "error");
    }

    (void)widget; // Mark unused parameter
    (void)data;   // Mark unused parameter
}

// Callback for the "Back" button
void on_back_button_clicked(GtkWidget *widget, gpointer data)
{
    (void)widget;
    (void)data;
    set_grid_in_entries(initial_grid);          // Restore the board to its initial state
    gtk_widget_set_visible(solve_button, TRUE); // Show solve button again
    gtk_widget_set_visible(back_button, FALSE); // Hide back button
    gtk_label_set_text(GTK_LABEL(label), "");   // Clear the status label

    // Clear status classes
    GtkStyleContext *context = gtk_widget_get_style_context(label);
    gtk_style_context_remove_class(context, "success");
    gtk_style_context_remove_class(context, "error");
}

// Callback for the "Reset" button
void on_reset_button_clicked(GtkWidget *widget, gpointer data)
{
    (void)widget;
    (void)data;
    int grid[SIZE][SIZE] = {0}; // Reset grid to all zeros
    set_grid_in_entries(grid);
    gtk_label_set_text(GTK_LABEL(label), "");
}

// Store the initial state of the grid for the "Back" button
void store_initial_grid(int grid[SIZE][SIZE])
{
    for (int i = 0; i < SIZE; i++)
    {
        for (int j = 0; j < SIZE; j++)
        {
            initial_grid[i][j] = grid[i][j];
        }
    }
}

int main(int argc, char *argv[])
{
    gtk_init(&argc, &argv);

    // Load CSS before creating any widgets
    load_css();

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
    for (int i = 0; i < SIZE; i++)
    {
        for (int j = 0; j < SIZE; j++)
        {
            initial_grid[i][j] = initial_state[i][j];
        }
    }

    // Create the main window (using the global variable)
    create_main_window();

    // Load the predefined board into the GUI
    set_grid_in_entries(initial_grid);

    // Show the window and all its widgets
    gtk_widget_show_all(window);

    gtk_main();
    return 0;
}
