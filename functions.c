#include <stdio.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdbool.h>
#include <SDL2/SDL_ttf.h>
#include "functions.h"
#include <SDL2/SDL_mixer.h> // Include SDL2_mixer header

// Define sound effects
Mix_Chunk *move_sound = NULL;
Mix_Chunk *invalid_sound = NULL;
Mix_Chunk *promote_sound = NULL;
Mix_Chunk *menu_sound = NULL;

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 640
#define BOARD_SIZE 8

// Define player colors
#define WHITE 'W'
#define BLACK 'B'
// Initialize the turn
extern char turn; // Start with white player



SDL_Window* create_window(const char* title, int width, int height) {
    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        printf("Error initializing SDL: %s\n", SDL_GetError());
        return NULL;
    }

    // Create the window
    SDL_Window* window = SDL_CreateWindow(
        title,
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        width,
        height,
        SDL_WINDOW_BORDERLESS
    );

    // Check if the window was created successfully
    if (!window) {
        printf("Error creating window: %s\n", SDL_GetError());
        SDL_Quit();
        return NULL;
    }

    return window;
}

void load_sounds() {
    move_sound = Mix_LoadWAV("move-self.mp3"); 
    invalid_sound = Mix_LoadWAV("invalid_sound.mp3"); 
    promote_sound = Mix_LoadWAV("promote.mp3"); 
    menu_sound = Mix_LoadWAV("menu.mp3");
}

void render_text(SDL_Renderer* renderer, TTF_Font* font, const char* text, int x, int y) {
    SDL_Color color = {0, 0, 0, 255}; // Black color for text
    SDL_Surface* surface = TTF_RenderText_Solid(font, text, color);
    if (!surface) {
        printf("Error rendering text surface: %s\n", TTF_GetError());
        return;
    }
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (!texture) {
        printf("Error creating texture from surface: %s\n", SDL_GetError());
        SDL_FreeSurface(surface);
        return;
    }
    SDL_Rect textRect = {x, y, 0, 0}; // Position of text
    SDL_QueryTexture(texture, NULL, NULL, &textRect.w, &textRect.h); // Get width and height of text
    SDL_RenderCopy(renderer, texture, NULL, &textRect);
    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);
}

void draw_board(SDL_Renderer* renderer, TTF_Font* font) {
    // Define the square size based on the board size
    int square_width = WINDOW_WIDTH / BOARD_SIZE;
    int square_height = WINDOW_HEIGHT / BOARD_SIZE;

    // Draw the chess board
    for (int row = 0; row < BOARD_SIZE; row++) {
        for (int col = 0; col < BOARD_SIZE; col++) {
            // Alternate between light and dark squares
            if ((row + col) % 2 == 0) {
                SDL_SetRenderDrawColor(renderer, 177, 228,185, 255);
            } else {
                SDL_SetRenderDrawColor(renderer, 112, 162, 163, 255);
            }

            SDL_Rect square = {
                col * square_width,
                row * square_height,
                square_width,
                square_height
            };

            SDL_RenderFillRect(renderer, &square);
        }
    }

    render_text(renderer, font, (turn == 'W' ? "Black's Turn" : "White's Turn"), 5, 5);

    SDL_RenderPresent(renderer);
}

void load_chess_pieces(SDL_Renderer* renderer, SDL_Texture** textures) {
    const char* image_files[] = {
        "images/wp.png",
        "images/bp.png",
        "images/wk.png",
        "images/bk.png",
        "images/wq.png",
        "images/bq.png",
        "images/wr.png",
        "images/br.png",
        "images/wb.png",
        "images/bb.png",
        "images/wh.png",
        "images/bh.png"
    };

    // Load each texture and check for errors
    for (int i = 0; i < 12; i++) {
        textures[i] = IMG_LoadTexture(renderer, image_files[i]);
        if (!textures[i]) {
            printf("Error loading texture from %s: %s\n", image_files[i], SDL_GetError());
        } else {
            printf("Successfully loaded texture from %s\n", image_files[i]);
        }
    }
}

void render_chess_pieces(SDL_Renderer* renderer, SDL_Texture** textures, char board[8][8]) {
    int square_width = WINDOW_WIDTH / BOARD_SIZE;
    int square_height = WINDOW_HEIGHT / BOARD_SIZE;

    // Set the desired scaling factor for the chess pieces
    float scaling_factor = 0.6f; // Adjust this value as needed

    // Calculate the scaled width and height of the chess pieces
    int scaled_width = square_width * scaling_factor;
    int scaled_height = square_height * scaling_factor;

    SDL_Rect dst_rect;

    for (int row = 0; row < BOARD_SIZE; row++) {
        for (int col = 0; col < BOARD_SIZE; col++) {
            // Calculate the x and y positions to center the image in the tile
            dst_rect.x = col * square_width + (square_width - scaled_width) / 2;
            dst_rect.y = row * square_height + (square_height - scaled_height) / 2;
            dst_rect.w = scaled_width;
            dst_rect.h = scaled_height;

            SDL_Texture* texture = NULL;
            char piece = board[row][col];

            switch (piece) {
                case 'p':
                    texture = textures[0];
                    break;

                case 'P':
                    texture = textures[1];
                    break;

                case 'k':
                    texture = textures[2];
                    break;

                case 'K':
                    texture = textures[3];
                    break;

                case 'q':
                    texture = textures[4];
                    break;

                case 'Q':
                    texture = textures[5];
                    break;

                case 'r':
                    texture = textures[6];
                    break;

                case 'R':
                    texture = textures[7];
                    break;

                case 'b':
                    texture = textures[8];
                    break;

                case 'B':
                    texture = textures[9];
                    break;

                case 'n':
                    texture = textures[10];
                    break;

                case 'N':
                    texture = textures[11];
                    break;

                default:
                    break;
            }

            if (texture) {
                SDL_RenderCopy(renderer, texture, NULL, &dst_rect);
            }
        }
    }
    SDL_RenderPresent(renderer);
}

bool validate_pawn_move(char board[8][8], int fromRow, int fromCol, int toRow, int toCol, bool *promoted) {
    char movingPiece = board[fromRow][fromCol];
    char targetPiece = board[toRow][toCol];


    // Check if the moving piece is a pawn
    if (movingPiece != 'p' && movingPiece != 'P') {
        return false;
    }

    // Calculate the direction based on pawn color (white pawns move up, black pawns move down)
    int direction = (movingPiece == 'p') ? 1 : -1;

    // Check if the pawn is trying to capture diagonally
    if (toCol != fromCol) {
        // Check if the target square is diagonally adjacent
        if (abs(toCol - fromCol) == 1 && abs(toRow - fromRow) == 1) {
            // Check if the target square is occupied by an enemy piece
            if ((movingPiece == 'p' && targetPiece >= 'A' && targetPiece <= 'Z') || (movingPiece == 'P' && targetPiece >= 'a' && targetPiece <= 'z')) {
                if ((movingPiece == 'p' && toRow == 0) || (movingPiece == 'P' && toRow == 7)) {
                *promoted = true;
                printf("Pawn reaches to the end");
                return true;

                } else {
                    *promoted = false;
                    return true; // Valid move: moving one or two squares forward from the starting position
                }
            } else {
                
                return false; // Disallow capturing a piece of the same color or moving diagonally without capture
            }
        } else {
            
            return false; // Invalid diagonal move (not diagonally adjacent)
        }
    }

    if ((movingPiece == 'p' && toRow == 0) || (movingPiece == 'P' && toRow == 7)) {
        *promoted = true;
        printf("Pawn reaches to the end\n");
        return true;
    }
}

bool validate_rook_move(char board[8][8], int fromRow, int fromCol, int toRow, int toCol) {
    char movingPiece = board[fromRow][fromCol];
    char targetPiece = board[toRow][toCol];

    // Check if the moving piece is a rook
    if (movingPiece != 'r' && movingPiece != 'R') {
        return false;
    }

    // Rooks can move either horizontally or vertically
    if (fromRow != toRow && fromCol != toCol) {
        return false; // Invalid move: rook can only move horizontally or vertically
    }

    // Check if there are any pieces blocking the path
    int deltaRow = (fromRow == toRow) ? 0 : (fromRow < toRow) ? 1 : -1;
    int deltaCol = (fromCol == toCol) ? 0 : (fromCol < toCol) ? 1 : -1;
    int currentRow = fromRow + deltaRow;
    int currentCol = fromCol + deltaCol;
    while (currentRow != toRow || currentCol != toCol) {
        if (board[currentRow][currentCol] != ' ') {
            return false; // Invalid move: path is blocked
        }
        currentRow += deltaRow;
        currentCol += deltaCol;
    }

    // Allow moving to an empty square or capturing an enemy piece
    if (targetPiece == ' ' || (targetPiece >= 'a' && targetPiece <= 'z') != (movingPiece >= 'a' && movingPiece <= 'z')) {
        return true; // Valid move
    } else {
        return false; // Disallow capturing a piece of the same color or moving to non-empty square
    }
}

bool validate_knight_move(char board[8][8], int fromRow, int fromCol, int toRow, int toCol) {
    char movingPiece = board[fromRow][fromCol];
    char targetPiece = board[toRow][toCol];

    // Check if the moving piece is a knight
    if (movingPiece != 'n' && movingPiece != 'N') {
        return false;
    }

    // Knights move in an L shape: two squares in one direction and one square in a perpendicular direction
    int rowDelta = abs(toRow - fromRow);
    int colDelta = abs(toCol - fromCol);
    if (!((rowDelta == 1 && colDelta == 2) || (rowDelta == 2 && colDelta == 1))) {
        return false; // Invalid move: knight moves in an L shape
    }

    // Allow moving to an empty square or capturing an enemy piece
    if (targetPiece == ' ' || (targetPiece >= 'a' && targetPiece <= 'z') != (movingPiece >= 'a' && movingPiece <= 'z')) {
        return true; // Valid move
    } else {
        return false; // Disallow capturing a piece of the same color or moving to non-empty square
    }
}

bool validate_bishop_move(char board[8][8], int fromRow, int fromCol, int toRow, int toCol) {
    char movingPiece = board[fromRow][fromCol];
    char targetPiece = board[toRow][toCol];

    // Check if the moving piece is a bishop
    if (movingPiece != 'b' && movingPiece != 'B') {
        return false;
    }

    // Check if the move is diagonal (equal change in rows and columns)
    if (abs(toRow - fromRow) != abs(toCol - fromCol)) {
        return false; // Invalid move: bishop moves diagonally
    }

    // Check if there are any pieces blocking the path
    int deltaRow = (toRow > fromRow) ? 1 : -1;
    int deltaCol = (toCol > fromCol) ? 1 : -1;
    int currentRow = fromRow + deltaRow;
    int currentCol = fromCol + deltaCol;
    while (currentRow != toRow || currentCol != toCol) {
        if (board[currentRow][currentCol] != ' ') {
            return false; // Invalid move: path is blocked
        }
        currentRow += deltaRow;
        currentCol += deltaCol;
    }

    // Allow moving to an empty square or capturing an enemy piece
    if (targetPiece == ' ' || (targetPiece >= 'a' && targetPiece <= 'z') != (movingPiece >= 'a' && movingPiece <= 'z')) {
        return true; // Valid move
    } else {
        return false; // Disallow capturing a piece of the same color or moving to non-empty square
    }
}

bool validate_queen_move(char board[8][8], int fromRow, int fromCol, int toRow, int toCol) {
    char movingPiece = board[fromRow][fromCol];
    char targetPiece = board[toRow][toCol];

    // Check if the moving piece is a queen
    if (movingPiece != 'q' && movingPiece != 'Q') {
        return false;
    }

    // Check if the move is either like a rook or a bishop
    if (!((fromRow == toRow || fromCol == toCol) || (abs(toRow - fromRow) == abs(toCol - fromCol)))) {
        return false; // Invalid move: queen can move like a rook or a bishop
    }

    // Check if there are any pieces blocking the path (if moving like a rook)
    if (fromRow == toRow || fromCol == toCol) {
        int deltaRow = (fromRow == toRow) ? 0 : (fromRow < toRow) ? 1 : -1;
        int deltaCol = (fromCol == toCol) ? 0 : (fromCol < toCol) ? 1 : -1;
        int currentRow = fromRow + deltaRow;
        int currentCol = fromCol + deltaCol;
        while (currentRow != toRow || currentCol != toCol) {
            if (board[currentRow][currentCol] != ' ') {
                return false; // Invalid move: path is blocked
            }
            currentRow += deltaRow;
            currentCol += deltaCol;
        }
    }
    
    // Check if there are any pieces blocking the path (if moving like a bishop)
    if (abs(toRow - fromRow) == abs(toCol - fromCol)) {
        int deltaRow = (toRow > fromRow) ? 1 : -1;
        int deltaCol = (toCol > fromCol) ? 1 : -1;
        int currentRow = fromRow + deltaRow;
        int currentCol = fromCol + deltaCol;
        while (currentRow != toRow || currentCol != toCol) {
            if (board[currentRow][currentCol] != ' ') {
                return false; // Invalid move: path is blocked
            }
            currentRow += deltaRow;
            currentCol += deltaCol;
        }
    }

    // Allow moving to an empty square or capturing an enemy piece
    if (targetPiece == ' ' || (targetPiece >= 'a' && targetPiece <= 'z') != (movingPiece >= 'a' && movingPiece <= 'z')) {
        return true; // Valid move
    } else {
        return false; // Disallow capturing a piece of the same color or moving to non-empty square
    }
}


bool validate_king_move(char board[8][8], int fromRow, int fromCol, int toRow, int toCol) {
    char movingPiece = board[fromRow][fromCol];
    char targetPiece = board[toRow][toCol];

    // Check if the moving piece is a king
    if (movingPiece != 'k' && movingPiece != 'K') {
        return false;
    }

    // Kings can move one square in any direction
    int rowDelta = abs(toRow - fromRow);
    int colDelta = abs(toCol - fromCol);
    if (rowDelta > 1 || colDelta > 1) {
        return false; // Invalid move: king can only move one square
    }

    // Allow moving to an empty square or capturing an enemy piece
    if (targetPiece == ' ' || (targetPiece >= 'a' && targetPiece <= 'z') != (movingPiece >= 'a' && movingPiece <= 'z')) {
        return true; // Valid move
    } else {
        return false; // Disallow capturing a piece of the same color or moving to non-empty square
    }
}


bool validate_move(char board[8][8], int fromRow, int fromCol, int toRow, int toCol, bool* promoted) {
    char movingPiece = board[fromRow][fromCol];
    char targetPiece = board[toRow][toCol];

    // Check if the moving piece is a pawn
    if (movingPiece == 'P' || movingPiece == 'p') {
        // Call the validate_pawn_move function
        return validate_pawn_move(board, fromRow, fromCol, toRow, toCol, promoted);
    } 
    // Check if the moving piece is a rook
    else if (movingPiece == 'R' || movingPiece == 'r') {
        // Rook movement validation
        return validate_rook_move(board, fromRow, fromCol, toRow, toCol);
    } 
    // Check if the moving piece is a knight
    else if (movingPiece == 'N' || movingPiece == 'n') {
        // Knight movement validation
        return validate_knight_move(board, fromRow, fromCol, toRow, toCol);
    } 
    // Check if the moving piece is a bishop
    else if (movingPiece == 'B' || movingPiece == 'b') {
        // Bishop movement validation
        return validate_bishop_move(board, fromRow, fromCol, toRow, toCol);
    } 
    // Check if the moving piece is a queen
    else if (movingPiece == 'Q' || movingPiece == 'q') {
        // Queen movement validation
        return validate_queen_move(board, fromRow, fromCol, toRow, toCol);
    } 
    // Check if the moving piece is a king
    else if (movingPiece == 'K' || movingPiece == 'k') {
        // King movement validation
        return validate_king_move(board, fromRow, fromCol, toRow, toCol);
    } 
    // Invalid piece
    else {
        return false;
    }

    // If all checks pass, the move is valid
    return true;
}



void draw_button(SDL_Renderer* renderer, TTF_Font* font, Button* button) {
    SDL_Color textColor = {255, 255, 255}; // White color for text
    SDL_Surface* surface = TTF_RenderText_Solid(font, button->text, textColor);
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);

    SDL_QueryTexture(texture, NULL, NULL, &(button->rect.w), &(button->rect.h));
    SDL_RenderCopy(renderer, texture, NULL, &(button->rect));

    SDL_DestroyTexture(texture);
}

bool handle_button_click(Button* button, int mouseX, int mouseY) {
    if (mouseX >= button->rect.x && mouseX <= button->rect.x + button->rect.w &&
        mouseY >= button->rect.y && mouseY <= button->rect.y + button->rect.h) {
        button->clicked = true;
        return true;
    }
    return false;
}

void reset_board(char board[8][8]) {
    char initial_board[8][8] = {
        {'R', 'N', 'B', 'Q', 'K', 'B', 'N', 'R'},
        {'P', 'P', 'P', 'P', 'P', 'P', 'P', 'P'},
        {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '},
        {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '},
        {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '},
        {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '},
        {'p', 'p', 'p', 'p', 'p', 'p', 'p', 'p'},
        {'r', 'n', 'b', 'q', 'k', 'b', 'n', 'r'}
    };

    memcpy(board, initial_board, sizeof(char) * 8 * 8);
    turn = BLACK;
}


bool menu_screen(TTF_Font* font, SDL_Renderer* renderer){

    SDL_Color backgroundColor = {0, 0, 0, 255};
    SDL_SetRenderDrawColor(renderer, backgroundColor.r, backgroundColor.g, backgroundColor.b, backgroundColor.a);
    SDL_RenderClear(renderer);

    SDL_Color textColor = {255, 255, 255}; // White color for text

    Button startButton = {{WINDOW_WIDTH / 2 - 110, WINDOW_HEIGHT / 2 - 100, 100, 30}, "Start Game", false};
    Button exitButton = {{WINDOW_WIDTH / 2 - 95, WINDOW_HEIGHT / 2 - 25, 100, 30}, "Exit Game", false};
    

    SDL_Event event;
    bool running = true;
    bool start_clicked = false;

    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            } else if (event.type == SDL_MOUSEBUTTONDOWN) {
                int mouseX, mouseY;
                SDL_GetMouseState(&mouseX, &mouseY);

                // Check if the mouse click is inside the button rectangles
                if (handle_button_click(&startButton, mouseX, mouseY)) {
                    Mix_PlayChannel(-1, menu_sound, 0);
                    start_clicked = true;
                    running = false;
                } else if (handle_button_click(&exitButton, mouseX, mouseY)) {
                    running = false;
                }
            }
        }
    
        // Clear screen
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        
        draw_button(renderer, font, &startButton);
        draw_button(renderer, font, &exitButton);
        
        

        SDL_RenderPresent(renderer);

    }    

    return start_clicked;

}

void show_popup_menu(SDL_Renderer* renderer, TTF_Font* font, TTF_Font* popUp_font, SDL_Texture** textures, char board[8][8], SDL_Window* window) {
    SDL_Color backgroundColor = {0, 0, 0, 255};
    SDL_SetRenderDrawColor(renderer, backgroundColor.r, backgroundColor.g, backgroundColor.b, backgroundColor.a);
    SDL_RenderClear(renderer);

    SDL_Color textColor = {255, 255, 255}; // White color for text

    Button resumeButton = {{WINDOW_WIDTH / 2 - 75, WINDOW_HEIGHT / 2 - 100, 100, 30}, "Resume", false};
    Button newGameButton = {{WINDOW_WIDTH / 2 - 75, WINDOW_HEIGHT / 2 - 50, 100, 30}, "New Game", false};
    Button exitButton = {{WINDOW_WIDTH / 2 - 75, WINDOW_HEIGHT / 2, 100, 30}, "Exit", false};

    bool running = true;
    SDL_Event event;

    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            } else if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT) {
                int mouseX, mouseY;
                SDL_GetMouseState(&mouseX, &mouseY);

                if (handle_button_click(&resumeButton, mouseX, mouseY)) {
                    printf("Resuming Game\n");
                    Mix_PlayChannel(-1, menu_sound, 0);
                    game_event(renderer, font, popUp_font, textures, board, window);                  
                    running = false;

                } else if (handle_button_click(&newGameButton, mouseX, mouseY)) {
                    printf("Starting a New Game\n");
                    Mix_PlayChannel(-1, menu_sound, 0);
                    reset_board(board);
                    game_event(renderer, font, popUp_font, textures, board, window);
                    running = false;
                } else if (handle_button_click(&exitButton, mouseX, mouseY)) {
                    printf("Exiting Game\n");
                    exit(0);
                }
            }
        }

        draw_button(renderer, popUp_font, &resumeButton);
        draw_button(renderer, popUp_font, &newGameButton);
        draw_button(renderer, popUp_font, &exitButton);

        SDL_RenderPresent(renderer);
    }
}

void show_promotion_menu(SDL_Renderer* renderer, TTF_Font* font, TTF_Font* popUp_font, SDL_Texture** textures, char board[8][8], SDL_Window* window, int clickedRow, int clickedCol, int selectedRow, int selectedCol) {
    SDL_Color backgroundColor = {0, 0, 0, 255};
    SDL_SetRenderDrawColor(renderer, backgroundColor.r, backgroundColor.g, backgroundColor.b, backgroundColor.a);
    SDL_RenderClear(renderer);

    SDL_Color textColor = {255, 255, 255}; // White color for text

    Button queenButton = {{WINDOW_WIDTH / 2 - 150, WINDOW_HEIGHT / 2 - 100, 100, 30}, "Promote To Queen", false};
    Button knightButton = {{WINDOW_WIDTH / 2 - 150, WINDOW_HEIGHT / 2, 100, 30}, "Promote To Knight", false};
    

    bool running = true;
    SDL_Event event;

    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            } else if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT) {
                int mouseX, mouseY;
                SDL_GetMouseState(&mouseX, &mouseY);

                if (handle_button_click(&queenButton, mouseX, mouseY)) {
                    printf("Pawn promoted to Queen\n");
                    printf("Selected Row: %d, Selected Col: %d\n", selectedRow, selectedCol);
                    Mix_PlayChannel(-1, menu_sound, 0);
                    // Check the color of the pawn and promote accordingly
                    if (board[selectedRow][selectedCol] == 'p') {
                        board[clickedRow][clickedCol] = 'q'; // Black pawn promoted to queen
                    } else if (board[selectedRow][selectedCol] == 'P') {
                        board[clickedRow][clickedCol] = 'Q'; // White pawn promoted to queen
                    }
                    printf("Promoted piece: %c\n", board[clickedRow][clickedCol]);
                    running = false;


                } else if (handle_button_click(&knightButton, mouseX, mouseY)) {
                    printf("Pawn promoted to Knight\n");
                    printf("Selected Row: %d, Selected Col: %d\n", selectedRow, selectedCol);
                    Mix_PlayChannel(-1, menu_sound, 0);
                    // Check the color of the pawn and promote accordingly
                    if (board[selectedRow][selectedCol] == 'p') {
                        board[clickedRow][clickedCol] = 'n'; // Black pawn promoted to knight
                    } else if (board[selectedRow][selectedCol] == 'P') {
                        board[clickedRow][clickedCol] = 'N'; // White pawn promoted to knight
                    }

                    printf("Promoted piece: %c\n", board[clickedRow][clickedCol]);
                    running = false;
                } 
                
            }
        }

        draw_button(renderer, popUp_font, &queenButton);
        draw_button(renderer, popUp_font, &knightButton);
        

        SDL_RenderPresent(renderer);
    }
}

void game_event(SDL_Renderer* renderer, TTF_Font* font, TTF_Font* popUp_font, SDL_Texture** textures, char board[8][8], SDL_Window* window){

            int running = 1;
            // Draw the chess board and render chess pieces

            draw_board(renderer, font);
            render_chess_pieces(renderer, textures, board);
            
            // Initialize the selected piece variables
            int selectedRow = -1;
            int selectedCol = -1;
            char selectedPiece = ' ';
            bool showingMenu = false;
            bool promoted = false; //

            // Initialization
            SDL_Renderer* offscreen_renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
            SDL_SetRenderDrawBlendMode(offscreen_renderer, SDL_BLENDMODE_BLEND);

            SDL_Event event;

            // Event loop
            while (running) {
                while (SDL_PollEvent(&event)) {
                    switch (event.type) {

                       case SDL_QUIT:
                            // Check if the event was triggered by the close button
                            if (!showingMenu && event.window.event == SDL_WINDOWEVENT_CLOSE) {
                                running = 0;
                            }
                            break;
                        case SDL_KEYDOWN:
                            if (!showingMenu && event.key.keysym.sym == SDLK_ESCAPE) {
                                // Handle showing the popup menu
                                show_popup_menu(renderer, font, popUp_font, textures, board, window);
                                showingMenu = true;
                            }
                            break;

                        case SDL_MOUSEBUTTONDOWN:
                            if (event.button.button == SDL_BUTTON_LEFT) {
                                // Get the mouse position
                                int mouseX = event.button.x;
                                int mouseY = event.button.y;

                                // Calculate the clicked tile's row and column
                                int clickedCol = mouseX / (WINDOW_WIDTH / BOARD_SIZE);
                                int clickedRow = mouseY / (WINDOW_HEIGHT / BOARD_SIZE);

                                // Handle the piece selection and movement
                                if (selectedRow == -1 && selectedCol == -1) {
                                    // Selecting a piece
                                    if ((turn == WHITE && isupper(board[clickedRow][clickedCol])) ||
                                        (turn == BLACK && islower(board[clickedRow][clickedCol]))) {
                                        selectedRow = clickedRow;
                                        selectedCol = clickedCol;
                                        selectedPiece = board[selectedRow][selectedCol];
                                        Mix_PlayChannel(-1, move_sound, 0);
                                        
                                    }
                                } else {
                                    // Attempting to move the selected piece
                                    if (validate_move(board, selectedRow, selectedCol, clickedRow, clickedCol, &promoted)) {
                                        // Move the piece if the move is valid
                                        board[clickedRow][clickedCol] = selectedPiece;
                                        board[selectedRow][selectedCol] = ' ';
                                        Mix_PlayChannel(-1, move_sound, 0);
 
                                        // Reset the selection
                                        selectedRow = -1;
                                        selectedCol = -1;
                                        selectedPiece = ' ';
                                        
                                        
                                        // Switch turns
                                        turn = (turn == WHITE) ? BLACK : WHITE;

                                        // Check if the pawn has been promoted
                                        if (promoted) {
                                            selectedRow = clickedRow;
                                            selectedCol = clickedCol;
                                            Mix_PlayChannel(-1, promote_sound, 0);
                                            show_promotion_menu(renderer, font, popUp_font, textures, board, window, clickedRow, clickedCol, selectedRow, selectedCol);
                                            printf("Pawn reaches to the end\n");
                                            promoted = false;
                                        }
                                        
                                        // Redraw the board and pieces
                                        draw_board(renderer, font);
                                        render_chess_pieces(renderer, textures, board);
                                        printf("Current player's turn: %c\n", turn);
                                    } else {
                                        // If the move is invalid, reset the selection
                                        selectedRow = -1;
                                        selectedCol = -1;
                                        selectedPiece = ' ';
                                        Mix_PlayChannel(-1, invalid_sound, 0);
                                    }
                                }
                            }
                            break;

                        
                        default:
                            break;
                    }
                }


                SDL_Delay(100); // Adjust this delay as needed;
            }

}