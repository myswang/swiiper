// Copyright (c) 2026 Mike Wang
// SPDX-License-Identifier: MIT

#include <stdio.h>

#include "BMfont5_png.h"
#include "game.h"
#include "menu.h"
#include "renderer.h"
#include "sprite_png.h"

const u32 CELL_SPRITE_SIZE = 24;
const u32 GRID_PADDING = 10;

// RGBA Colors
const u32 GRRLIB_BLACK = 0x000000FF;
const u32 GRRLIB_GREY = 0x7B7B7BFF;
const u32 GRRLIB_WHITE = 0xFFFFFFFF;
const u32 GRRLIB_YELLOW = 0xFFFF00FF;
const u32 GRRLIB_GREEN = 0x00FF00FF;
const u32 GRRLIB_RED = 0xFF0000FF;

const u32 FONT_CHAR_WIDTH = 8;
const u32 FONT_CHAR_HEIGHT = 16;
const u32 FONT_LINE_WIDTH = 10;
const u32 MARGIN_X = 32;
const u32 MARGIN_Y = 32;

void Renderer_Init(Renderer *r) {
    r->is_widescreen = CONF_GetAspectRatio() == CONF_ASPECT_16_9;
    r->screen_width = r->is_widescreen ? SCREEN_WIDTH_16_9 : SCREEN_WIDTH_4_3;
    r->screen_height = 480;
    r->mid_screen_width = r->screen_width / 2;
    r->mid_screen_height = 240;
    r->tex_font = GRRLIB_LoadTexture(BMfont5_png);
    GRRLIB_InitTileSet(r->tex_font, FONT_CHAR_WIDTH, FONT_CHAR_HEIGHT, 0);
    r->tex_sprite = GRRLIB_LoadTexture(sprite_png);
    GRRLIB_InitTileSet(r->tex_sprite, CELL_SPRITE_SIZE, CELL_SPRITE_SIZE, 0);
}

void Renderer_Prelude(Renderer *r) {
    Mtx44 perspective;
    guOrtho(perspective, 0, r->screen_height, 0, r->screen_width, 0, 300);
    GX_LoadProjectionMtx(perspective, GX_ORTHOGRAPHIC);
    GRRLIB_FillScreen(GRRLIB_BLACK);
}

void Renderer_DrawMenu(Renderer *r, const Menu *menu) {
    GRRLIB_Printf(MARGIN_X, MARGIN_Y, r->tex_font, GRRLIB_WHITE, 1,
                  menu->title);

    for (u32 i = 0; i < menu->num_options; i++) {
        u32 colour = (i == (u32)menu->cursor) ? GRRLIB_YELLOW : GRRLIB_WHITE;
        u32 ypos = MARGIN_Y * 2 + (i * FONT_CHAR_HEIGHT * 2);

        GRRLIB_Printf(MARGIN_X, ypos, r->tex_font, colour, 1, "%s  %s",
                      (i == (u32)menu->cursor) ? ">>" : "  ",
                      menu->options[i].label);
    }
}

void Renderer_DrawGame(Renderer *r, const Game *game) {
    u32 grid_start_top =
        r->mid_screen_height - (CELL_SPRITE_SIZE * game->config.num_rows / 2);
    u32 grid_start_left =
        r->mid_screen_width - (CELL_SPRITE_SIZE * game->config.num_cols / 2);
    // background color
    GRRLIB_Rectangle(
        grid_start_left - GRID_PADDING, grid_start_top - GRID_PADDING,
        CELL_SPRITE_SIZE * game->config.num_cols + 2 * GRID_PADDING,
        CELL_SPRITE_SIZE * game->config.num_rows + 2 * GRID_PADDING,
        GRRLIB_GREY, true);

    // game grid
    for (u32 i = 0; i < game->config.num_rows; i++) {
        for (u32 j = 0; j < game->config.num_cols; j++) {
            Cell cell = game->grid[i][j];

            u32 frame = 0;
            switch (cell.state) {
            case CELL_HIDDEN:
                frame = 12;
                break;
            case CELL_FLAGGED:
                frame = 13;
                break;
            case CELL_EXPLODED:
                frame = 10;
                break;
            case CELL_INVALID_FLAG:
                frame = 11;
                break;
            case CELL_REVEALED:
                frame = cell.is_mine ? 9 : cell.num_adjacent;
                break;
            }

            u32 cell_shade = GRRLIB_WHITE;
            if ((game->state == GAME_IDLE || game->state == GAME_RUNNING) &&
                game->cur_x == j && game->cur_y == i) {
                cell_shade = GRRLIB_YELLOW;
            }
            GRRLIB_DrawTile(grid_start_left + CELL_SPRITE_SIZE * j,
                            grid_start_top + CELL_SPRITE_SIZE * i,
                            r->tex_sprite, 0, 1, 1, cell_shade, frame);
        }
    }
}

void Renderer_DrawGameStatus(Renderer *r, const Game *game, u64 seconds) {
    // status message
    char buffer[128] = {0};
    u32 text_colour = GRRLIB_WHITE;
    switch (game->state) {
    case GAME_IDLE:
    case GAME_RUNNING:
    case GAME_PAUSED:
        snprintf(buffer, sizeof(buffer), "Flags: %u Time: %llu seconds",
                 game->num_flags, seconds);
        text_colour = GRRLIB_WHITE;
        break;
    case GAME_LOST:
        snprintf(buffer, sizeof(buffer),
                 "Flags: %u Time: %llu seconds - You hit a mine, unlucky.",
                 game->num_flags, seconds);
        text_colour = GRRLIB_RED;
        break;
    case GAME_WON:
        snprintf(buffer, sizeof(buffer),
                 "Flags: %u Time: %llu seconds - You won, congrats!",
                 game->num_flags, seconds);
        text_colour = GRRLIB_GREEN;
        break;
    }
    GRRLIB_Printf(MARGIN_X, r->screen_height - MARGIN_Y, r->tex_font,
                  text_colour, 1, buffer, game->num_flags, seconds);
}

void Renderer_Free(Renderer *r) {
    GRRLIB_FreeTexture(r->tex_font);
    GRRLIB_FreeTexture(r->tex_sprite);
}