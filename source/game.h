// Copyright (c) 2026 Mike Wang
// SPDX-License-Identifier: MIT

#ifndef GAME_H
#define GAME_H

#include "tuxedo/types.h"

enum {
    MAX_W = 30,
    MAX_H = 16,
    MAX_MINES = 99,

    MIN_W = 4,
    MIN_H = 4,
    MIN_MINES = 1
};

typedef enum {
    CELL_HIDDEN = 0,
    CELL_FLAGGED,
    CELL_REVEALED,
    CELL_EXPLODED,
    CELL_INVALID_FLAG
} CellState;

typedef struct {
    u32 num_adjacent : 4;
    u32 is_mine : 1;
    CellState state : 3;
} Cell;

typedef enum {
    GAME_IDLE = 0,
    GAME_RUNNING,
    GAME_PAUSED,
    GAME_WON,
    GAME_LOST
} GameState;

typedef enum {
    GAME_MOVE_UP = 0,
    GAME_MOVE_DOWN,
    GAME_MOVE_LEFT,
    GAME_MOVE_RIGHT,
    GAME_REVEAL_CELL,
    GAME_FLAG_CELL,
    GAME_CHORD_CELLS
} GameAction;

typedef struct GameConfig {
    u32 num_cols;
    u32 num_rows;
    u32 num_mines;
} GameConfig;

static const GameConfig EASY_MODE =
    (GameConfig){.num_cols = 9, .num_rows = 9, .num_mines = 10};
static const GameConfig MEDIUM_MODE =
    (GameConfig){.num_cols = 16, .num_rows = 16, .num_mines = 40};
static const GameConfig HARD_MODE =
    (GameConfig){.num_cols = 30, .num_rows = 16, .num_mines = 99};
static const GameConfig HARD_MODE_4_3 =
    (GameConfig){.num_cols = 30, .num_rows = 16, .num_mines = 99};

static inline GameConfig Game_GetHardMode(bool is_widescreen) {
    return is_widescreen ? HARD_MODE : HARD_MODE_4_3;
}

static inline u32 Game_GetMaxMines(GameConfig config) {
    return config.num_cols * config.num_rows - 9;
}

typedef struct Game {
    u32 num_flags;
    u32 remaining_cells;
    s32 cur_x;
    s32 cur_y;
    u64 start_time;
    u64 elapsed_ticks;
    GameConfig config;
    GameState state;
    Cell grid[MAX_H][MAX_W];
} Game;

void Game_Init(Game *game, GameConfig config);
void Game_HandleAction(Game *game, GameAction action);
void Game_ResumeTime(Game *game);
void Game_PauseTime(Game *game);
u64 Game_GetCurrentTime(const Game *game);

#endif