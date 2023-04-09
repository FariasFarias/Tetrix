
#ifndef FUNCTION_GAME
#define FUNCTION_GAME

#define WIDTH 10
#define HEIGHT 20

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ncursesw/ncurses.h>
#include <pthread.h>


//struct coordinates
typedef struct coord
{
    int h;
    int w;
}
coord;

// struct for piece of game
typedef struct piece
{
    coord center;
    coord x;
    coord y;
    coord z;

    char name_piece;
} 
piece;

//range for piece's
typedef struct range
{
    bool matrix[4][4];
}
range;

// prototypes
void load_grid_to_buffer();
void load_grid();
void *update_screen();
void spawn_piece_screen(piece p, int height, int width);
int init_game();
int complete_fall(int velocity);
void solidify_block();
void update_frame_piece_falldown();
void *input_control();
void move_pieces(char direction);
bool is_touching_edge(char edge);
bool is_touching_solid(char side);
void spawn_random_piece();
int exit_game();
void eliminate_line();
bool is_complete_line(int i);
void start_gameplay();
void gravity_solid(int line);
void rotate_piece();
void clear_piece_from_grid();
void drop_piece_instantly();
void respawn_piece_screen(piece t);
bool is_respawnenable(piece p);
piece get_piece_from_range(range r);
piece create_piece(char name);
bool solidify_if_touch_bottom();
int init_interface_game();
void update_interface_game_points(int score, int lines, int level);
void precipitate_piece();
bool lose_game();
void gameover();
void interrupt_game();
void print_piece_screen(piece p, int height, int width);
void update_interface_pieces();
void use_hold_piece();
void print_hold_piece();
int menu();
void show_instructions();
void show_about();

#endif