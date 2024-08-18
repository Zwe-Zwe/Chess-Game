#include <stdio.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdbool.h>
#include <ctype.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_mixer.h>
#include "functions.h"

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 640
#define BOARD_SIZE 8

// Define player colors
#define WHITE 'W'
#define BLACK 'B'
// Initialize the turn
char turn = BLACK; // Start with white player

int main(int argc, char* argv[]) {
    // Initialize TTF
    TTF_Init();

    SDL_Window* window = create_window("Chess", WINDOW_WIDTH, WINDOW_HEIGHT);
    if (!window) {
        return 1;
    }

    // Create a renderer and check for errors
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        printf("Error creating renderer: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    // Initialize SDL_image and check for errors
    if (IMG_Init(IMG_INIT_PNG) != IMG_INIT_PNG) {
        printf("Error initializing SDL_image: %s\n", IMG_GetError());
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "100");

    // Initialize SDL2_mixer library
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        printf("SDL_mixer could not initialize! SDL_mixer Error: %s\n", Mix_GetError());
        // Handle error
    }

    // Load sound effects
    load_sounds();


    // Load font
    TTF_Font* font = TTF_OpenFont("Atop-R99O3.ttf", 32);
    if (!font) {
        printf("Failed to load font: %s\n", TTF_GetError());
        return 1;
    }

    TTF_Font* turn_font = TTF_OpenFont("Atop-R99O3.ttf", 15);
    if (!font) {
        printf("Failed to load font: %s\n", TTF_GetError());
        return 1;
    }
 
    // Load textures for the chess pieces
    SDL_Texture* textures[12];
    load_chess_pieces(renderer, textures);

    char board[8][8] = {
            {'R', 'N', 'B', 'Q', 'K', 'B', 'N', 'R'},
            {'P', 'P', 'P', 'P', 'P', 'P', 'P', 'P'},
            {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '},
            {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '},
            {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '},
            {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '},
            {'p', 'p', 'p', 'p', 'p', 'p', 'p', 'p'},
            {'r', 'n', 'b', 'q', 'k', 'b', 'n', 'r'}
        };

    if(menu_screen(font, renderer)){

        game_event(renderer, turn_font, font, textures, board, window);

    }

    // Clean up textures
    for (int i = 0; i < 12; i++) {
        if (textures[i]) {
            SDL_DestroyTexture(textures[i]);
        }
    } 

    // Clean up renderer and window
   
    TTF_CloseFont(font);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    IMG_Quit();
    SDL_Quit();

    return 0;

}