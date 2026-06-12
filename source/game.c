// Copyright (c) 2026 Mike Wang
// SPDX-License-Identifier: MIT

#include <assert.h>
#include <stdlib.h>

#include "game.h"
#include "ogc/lwp_watchdog.h"

typedef struct {
    u32 x : 8;
    u32 y : 8;
} CellIndex;

typedef struct {
    CellIndex items[MAX_W * MAX_H * 9];
    u32 top;
} Stack;

void Stack_Push(Stack *stack, CellIndex item) {
    assert(stack->top < MAX_W * MAX_H * 9 && "Stack overflow");
    stack->items[stack->top++] = item;
}

CellIndex Stack_Pop(Stack *stack) {
    assert(stack->top > 0 && "Stack underflow");
    return stack->items[--stack->top];
}

void Game_Init(Game *game, GameConfig config) {
    game->config = config;
    game->elapsed_ticks = 0;
    game->start_time = 0;
    game->num_flags = config.num_mines;
    game->remaining_cells = 0;
    game->cur_x = 0;
    game->cur_y = 0;
    game->state = GAME_IDLE;
    // clear old grid
    for (u32 i = 0; i < MAX_H; i++) {
        for (u32 j = 0; j < MAX_W; j++) {
            game->grid[i][j] = (Cell){
                .num_adjacent = 0, .is_mine = false, .state = CELL_HIDDEN};
        }
    }
}

void Game_SpawnMines(Game *game) {
    // generate mine indices, avoiding the selected cursor
    u32 idx_length = game->config.num_cols * game->config.num_rows;
    game->remaining_cells = idx_length - game->config.num_mines;
    CellIndex indices[idx_length];
    u32 count = 0;
    for (s32 i = 0; i < game->config.num_cols; i++) {
        for (s32 j = 0; j < game->config.num_rows; j++) {
            if (i >= game->cur_x - 1 && i <= game->cur_x + 1 &&
                j >= game->cur_y - 1 && j <= game->cur_y + 1)
                continue;
            indices[count].x = i;
            indices[count].y = j;
            count++;
        }
    }
    // shuffle indices
    for (u32 i = count - 1; i > 0; i--) {
        u32 j = rand() % i;
        CellIndex tmp = indices[i];
        indices[i] = indices[j];
        indices[j] = tmp;
    }
    // place mines
    for (u32 i = 0; i < game->config.num_mines; i++) {
        game->grid[indices[i].y][indices[i].x].is_mine = true;

        for (s32 y = indices[i].y - 1; y <= indices[i].y + 1; y++) {
            for (s32 x = indices[i].x - 1; x <= indices[i].x + 1; x++) {
                if (x >= 0 && x < (s32)game->config.num_cols && y >= 0 &&
                    y < (s32)game->config.num_rows) {
                    game->grid[y][x].num_adjacent++;
                }
            }
        }
    }
}

// reveals a cell, performing flood-fill if necessary
// then checks if the game should end
void Game_RevealCell(Game *game, s32 x, s32 y) {
    Stack stack = {0};
    Stack_Push(&stack, (CellIndex){.x = x, .y = y});

    while (stack.top > 0) {
        CellIndex cur = Stack_Pop(&stack);

        Cell *cell = &game->grid[cur.y][cur.x];
        if (cell->state == CELL_FLAGGED || cell->state == CELL_REVEALED)
            continue;

        cell->state = CELL_REVEALED;
        if (cell->is_mine) {
            Game_PauseTime(game);
            game->state = GAME_LOST;
            cell->state = CELL_EXPLODED;
            continue;
        }
        game->remaining_cells--;
        if (cell->num_adjacent > 0)
            continue;

        for (s32 y = cur.y - 1; y <= cur.y + 1; y++) {
            for (s32 x = cur.x - 1; x <= cur.x + 1; x++) {
                if (x >= 0 && x < (s32)game->config.num_cols && y >= 0 &&
                    y < (s32)game->config.num_rows) {
                    Stack_Push(&stack, (CellIndex){.x = x, .y = y});
                }
            }
        }
    }
}

void Game_CheckGameOver(Game *game) {
    // check if the game should end
    if (game->state == GAME_LOST) { // you hit a mine, unlucky
        for (u32 i = 0; i < game->config.num_rows; i++) {
            for (u32 j = 0; j < game->config.num_cols; j++) {
                Cell *cell = &game->grid[i][j];
                if (cell->is_mine && cell->state == CELL_HIDDEN)
                    cell->state = CELL_REVEALED;
                if (!cell->is_mine && cell->state == CELL_FLAGGED)
                    cell->state = CELL_INVALID_FLAG;
            }
        }
    } else if (game->remaining_cells == 0) { // you won the game! nice
        Game_PauseTime(game);
        game->state = GAME_WON;
        game->num_flags = 0;
        for (u32 i = 0; i < game->config.num_rows; i++) {
            for (u32 j = 0; j < game->config.num_cols; j++) {
                Cell *cell = &game->grid[i][j];
                if (cell->is_mine)
                    cell->state = CELL_FLAGGED;
            }
        }
    }
}

void Game_ChordCells(Game *game) {
    Cell *cur_cell = &game->grid[game->cur_y][game->cur_x];

    if (cur_cell->state != CELL_REVEALED || cur_cell->num_adjacent == 0)
        return;

    u32 count = 0;
    for (s32 y = game->cur_y - 1; y <= game->cur_y + 1; y++) {
        for (s32 x = game->cur_x - 1; x <= game->cur_x + 1; x++) {
            if (x >= 0 && x < (s32)game->config.num_cols && y >= 0 &&
                y < (s32)game->config.num_rows &&
                game->grid[y][x].state == CELL_FLAGGED) {
                count++;
            }
        }
    }

    if (count == cur_cell->num_adjacent) {
        for (s32 y = game->cur_y - 1; y <= game->cur_y + 1; y++) {
            for (s32 x = game->cur_x - 1; x <= game->cur_x + 1; x++) {
                if (x >= 0 && x < (s32)game->config.num_cols && y >= 0 &&
                    y < (s32)game->config.num_rows) {
                    Game_RevealCell(game, x, y);
                }
            }
        }
        Game_CheckGameOver(game);
    }
}

void Game_HandleAction(Game *game, GameAction action) {
    Cell *cur_cell = &game->grid[game->cur_y][game->cur_x];
    switch (action) {
    case GAME_MOVE_UP:
        if (game->cur_y > 0)
            game->cur_y--;
        break;
    case GAME_MOVE_DOWN:
        if (game->cur_y < game->config.num_rows - 1)
            game->cur_y++;
        break;
    case GAME_MOVE_LEFT:
        if (game->cur_x > 0)
            game->cur_x--;
        break;
    case GAME_MOVE_RIGHT:
        if (game->cur_x < game->config.num_cols - 1)
            game->cur_x++;
        break;
    case GAME_REVEAL_CELL:        
        Game_ChordCells(game);
        if (cur_cell->state == CELL_HIDDEN) {
            if (game->state == GAME_IDLE) {
                Game_SpawnMines(game);
                game->state = GAME_RUNNING;
                Game_ResumeTime(game);
            }
            Game_RevealCell(game, game->cur_x, game->cur_y);
            Game_CheckGameOver(game);
        }
        break;
    case GAME_FLAG_CELL:
        if (cur_cell->state == CELL_HIDDEN) {
            game->num_flags--;
            cur_cell->state = CELL_FLAGGED;
        } else if (cur_cell->state == CELL_FLAGGED) {
            game->num_flags++;
            cur_cell->state = CELL_HIDDEN;
        }
        break;
    case GAME_CHORD_CELLS:
        Game_ChordCells(game);
        break;
    }
}

void Game_ResumeTime(Game *game) {
    game->start_time = gettime() - game->elapsed_ticks;
}

void Game_PauseTime(Game *game) {
    game->elapsed_ticks = gettime() - game->start_time;
}

u64 Game_GetCurrentTime(const Game *game) {
    u64 current_ticks = game->elapsed_ticks;
    if (game->state == GAME_RUNNING)
        current_ticks = gettime() - game->start_time;
    return current_ticks;
}