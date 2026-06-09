// Copyright (c) 2026 Mike Wang
// SPDX-License-Identifier: MIT

#ifndef RENDERER_H
#define RENDERER_H

#include "grrlib.h"

typedef struct Game Game;
typedef struct Menu Menu;

enum {
    SCREEN_WIDTH_16_9 = 854,
    SCREEN_WIDTH_4_3 = 640
};

typedef struct {
    GRRLIB_texImg *tex_font;
    GRRLIB_texImg *tex_sprite;
    u32 screen_width;
    u32 screen_height;
    u32 mid_screen_width;
    u32 mid_screen_height;
    bool is_widescreen;
} Renderer;

void Renderer_Init(Renderer *r);
void Renderer_Prelude(Renderer *r);
void Renderer_DrawMenu(Renderer *r, const Menu *menu);
void Renderer_DrawGame(Renderer *r, const Game *game);
void Renderer_DrawGameStatus(Renderer *r, const Game *game, u64 seconds);
void Renderer_Free(Renderer *r);

#endif