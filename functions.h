#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include <stdio.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdbool.h>
#include <ctype.h>
#include <SDL2/SDL_ttf.h>

typedef struct {
    SDL_Rect rect;
    char* text;
    bool clicked;
} Button;

typedef struct {
    SDL_Rect rect;
    char text[100]; // Adjust the size according to your needs
} TextInputField;

SDL_Window* create_window(const char* title, int width, int height);
void load_sounds();
void render_text(SDL_Renderer* renderer, TTF_Font* font, const char* text, int x, int y);
void draw_board(SDL_Renderer* renderer, TTF_Font* font);
void load_chess_pieces(SDL_Renderer* renderer, SDL_Texture** textures);
void render_chess_pieces(SDL_Renderer* renderer, SDL_Texture** textures, char board[8][8]);
void promote_pawn(char board[8][8], int row, int col, char promotionPiece);
bool validate_pawn_move(char board[8][8], int fromRow, int fromCol, int toRow, int toCol, bool *promoted);
bool validate_rook_move(char board[8][8], int fromRow, int fromCol, int toRow, int toCol);
bool validate_knight_move(char board[8][8], int fromRow, int fromCol, int toRow, int toCol);
bool validate_bishop_move(char board[8][8], int fromRow, int fromCol, int toRow, int toCol);
bool validate_queen_move(char board[8][8], int fromRow, int fromCol, int toRow, int toCol);
bool validate_king_move(char board[8][8], int fromRow, int fromCol, int toRow, int toCol);
bool validate_move(char board[8][8], int fromRow, int fromCol, int toRow, int toCol, bool* promoted);
void draw_button(SDL_Renderer* renderer, TTF_Font* font, Button* button);
bool handle_button_click(Button* button, int mouseX, int mouseY);
void reset_board(char board[8][8]);
void draw_text_input_field(SDL_Renderer* renderer, TTF_Font* font, TextInputField* inputField);
bool handle_text_input_event(SDL_Event* event, TextInputField* inputField);
void save_game(const char* filename, char board[8][8], char turn);
void load_game(const char* filename, char board[8][8], char* turn);
bool menu_screen(TTF_Font* font, SDL_Renderer* renderer);
void show_popup_menu(SDL_Renderer* renderer, TTF_Font* font, TTF_Font* popUp_font, SDL_Texture** textures, char board[8][8], SDL_Window* window);
void show_promotion_menu(SDL_Renderer* renderer, TTF_Font* font, TTF_Font* popUp_font, SDL_Texture** textures, char board[8][8], SDL_Window* window, int clickedRow, int clickedCol, int selectedRow, int selectedCol);
void game_event(SDL_Renderer* renderer, TTF_Font* font, TTF_Font* popUp_font, SDL_Texture** textures, char board[8][8], SDL_Window* window);

#endif