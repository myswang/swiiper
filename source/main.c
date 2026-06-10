// Copyright (c) 2026 Mike Wang
// SPDX-License-Identifier: MIT

#include <assert.h>
#include <fat.h>
#include <stdlib.h>

#include "config.h"
#include "game.h"
#include "grrlib.h"
#include "input.h"
#include "menu.h"
#include "ogc/lwp_watchdog.h"
#include "renderer.h"

typedef enum {
    SCENE_DIFFICULTY_MENU = 0,
    SCENE_GAME,
    SCENE_PAUSE_MENU,
    SCENE_CUSTOM_MENU
} Scene;

int main(void) {
    GRRLIB_Init();
    srand((u32)gettime());

    assert(fatInitDefault() && "Failed to initialize FAT");

    Renderer r;
    Renderer_Init(&r);
    Input input;
    Input_Init(&input);

    GameConfig hard_config = Game_GetHardMode(r.is_widescreen);

    difficulty_menu.options[2].config = hard_config;
    difficulty_menu.options[2].label = r.is_widescreen
                                           ? "Expert (30x16, 99 mines)"
                                           : "Expert (24x16, 79 mines)";

    GameConfig custom_options = MEDIUM_MODE;
    Config_ReadFromFile(&custom_options);

    GameConfig new_custom_options = custom_options;
    u32 cur_max_mines = Game_GetMaxMines(custom_options);

    Menu_UpdateCustomLabel(custom_options);
    Menu_UpdateCustomMenu(custom_options,
                          Game_GetHardMode(r.is_widescreen).num_cols,
                          cur_max_mines);

    Scene cur_scene = SCENE_DIFFICULTY_MENU;
    Game game;

    while (SYS_MainLoop()) {
        Renderer_Prelude(&r);
        Input_Read(&input);
        u64 current_ticks = Game_GetCurrentTime(&game);
        u64 total_millis = ticks_to_millisecs(current_ticks);
        u64 seconds = total_millis / 1000;
        if (input.button_home)
            break;
        switch (cur_scene) {
        case SCENE_DIFFICULTY_MENU:
            Renderer_DrawMenu(&r, &difficulty_menu);
            if (input.dpad_down)
                Menu_IncCursor(&difficulty_menu);
            if (input.dpad_up)
                Menu_DecCursor(&difficulty_menu);
            if (input.button_a) {
                if (difficulty_menu.cursor == 4) {
                    cur_scene = SCENE_CUSTOM_MENU;
                } else {
                    Game_Init(
                        &game,
                        difficulty_menu.options[difficulty_menu.cursor].config);
                    cur_scene = SCENE_GAME;
                }
            }
            break;
        case SCENE_GAME:
            Renderer_DrawGame(&r, &game);
            Renderer_DrawGameStatus(&r, &game, seconds);
            if (input.button_plus) {
                if (game.state == GAME_RUNNING) {
                    game.state = GAME_PAUSED;
                    Game_PauseTime(&game);
                }
                cur_scene = SCENE_PAUSE_MENU;
                break;
            }
            if (input.button_minus)
                Game_Init(&game, game.config);
            if (game.state == GAME_WON || game.state == GAME_LOST)
                break;

            if (input.dpad_held_up)
                Game_HandleAction(&game, GAME_MOVE_UP);
            if (input.dpad_held_down)
                Game_HandleAction(&game, GAME_MOVE_DOWN);
            if (input.dpad_held_left)
                Game_HandleAction(&game, GAME_MOVE_LEFT);
            if (input.dpad_held_right)
                Game_HandleAction(&game, GAME_MOVE_RIGHT);

            if (input.button_a)
                Game_HandleAction(&game, GAME_REVEAL_CELL);
            if (input.button_b)
                Game_HandleAction(&game, GAME_FLAG_CELL);
            if (input.button_x)
                Game_HandleAction(&game, GAME_CHORD_CELLS);
            break;
        case SCENE_PAUSE_MENU:
            Renderer_DrawMenu(&r, &pause_menu);
            Renderer_DrawGameStatus(&r, &game, seconds);
            if (input.button_plus || input.button_b) {
                if (game.state == GAME_PAUSED) {
                    game.state = GAME_RUNNING;
                    Game_ResumeTime(&game);
                }
                cur_scene = SCENE_GAME;
                pause_menu.cursor = 0;
                break;
            }
            if (input.dpad_down)
                Menu_IncCursor(&pause_menu);
            if (input.dpad_up)
                Menu_DecCursor(&pause_menu);
            if (input.button_a) {
                switch (pause_menu.cursor) {
                case 0:
                    if (game.state == GAME_PAUSED) {
                        game.state = GAME_RUNNING;
                        Game_ResumeTime(&game);
                    }
                    cur_scene = SCENE_GAME;
                    pause_menu.cursor = 0;
                    break;
                case 1:
                    game.state = GAME_IDLE;
                    cur_scene = SCENE_DIFFICULTY_MENU;
                    pause_menu.cursor = 0;
                    break;
                case 2:
                    Renderer_Free(&r);
                    GRRLIB_Exit();
                    exit(0);
                    break;
                }
            }
            break;
        case SCENE_CUSTOM_MENU:
            Renderer_DrawMenu(&r, &custom_menu);
            if (input.button_b) {
                cur_scene = SCENE_DIFFICULTY_MENU;
                break;
            }
            if (input.dpad_down)
                Menu_IncCursor(&custom_menu);
            if (input.dpad_up)
                Menu_DecCursor(&custom_menu);
            if (input.button_a) {
                switch (custom_menu.cursor) {
                case 3:
                    cur_scene = SCENE_DIFFICULTY_MENU;
                    custom_options = new_custom_options;
                    Menu_UpdateCustomLabel(custom_options);
                    Config_WriteToFile(&custom_options);
                    break;
                case 4:
                    cur_scene = SCENE_DIFFICULTY_MENU;
                    break;
                }
                custom_menu.cursor = 0;
            }
            if (input.dpad_held_left) {
                switch (custom_menu.cursor) {
                case 0:
                    new_custom_options.num_cols =
                        MAX(new_custom_options.num_cols - 1, MIN_W);
                    break;
                case 1:
                    new_custom_options.num_rows =
                        MAX(new_custom_options.num_rows - 1, MIN_H);
                    break;
                case 2:;
                    u32 dec_count = input.repeat_count > 10 ? 20 : 1;
                    s32 new_mines = new_custom_options.num_mines - dec_count;
                    new_custom_options.num_mines = MAX(new_mines, MIN_MINES);
                    break;
                }

                cur_max_mines = Game_GetMaxMines(new_custom_options);
                if (new_custom_options.num_mines >= cur_max_mines)
                    new_custom_options.num_mines = cur_max_mines;
                Menu_UpdateCustomMenu(new_custom_options, hard_config.num_cols,
                                      cur_max_mines);
            }
            if (input.dpad_held_right) {
                switch (custom_menu.cursor) {
                case 0:
                    new_custom_options.num_cols =
                        MIN(new_custom_options.num_cols + 1,
                            Game_GetHardMode(r.is_widescreen).num_cols);
                    break;
                case 1:
                    new_custom_options.num_rows =
                        MIN(new_custom_options.num_rows + 1, MAX_H);
                    break;
                case 2:;
                    u32 inc_count = input.repeat_count > 10 ? 20 : 1;
                    new_custom_options.num_mines =
                        MIN(new_custom_options.num_mines + inc_count,
                            cur_max_mines);
                    break;
                }

                cur_max_mines = Game_GetMaxMines(new_custom_options);
                Menu_UpdateCustomMenu(new_custom_options, hard_config.num_cols,
                                      cur_max_mines);
            }
            break;
        }
        GRRLIB_Render();
    }

    // Clean up
    Renderer_Free(&r);
    GRRLIB_Exit();
    exit(0);
}