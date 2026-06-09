// Copyright (c) 2026 Mike Wang
// SPDX-License-Identifier: MIT

#ifndef MENU_H
#define MENU_H

#include "game.h"

typedef struct MenuOption {
    const char *label;
    GameConfig config;
} MenuOption;

typedef struct Menu {
    const char *title;
    MenuOption *options;
    u32 num_options;
    s32 cursor;
} Menu;

inline void Menu_IncCursor(Menu *menu) {
    menu->cursor = (menu->cursor + 1) % menu->num_options;
}

inline void Menu_DecCursor(Menu *menu) {
    menu->cursor = (menu->cursor - 1 + menu->num_options) % menu->num_options;
}

extern Menu difficulty_menu;
extern Menu pause_menu;
extern Menu custom_menu;

void Menu_UpdateCustomLabel(GameConfig new_config);
void Menu_UpdateCustomMenu(GameConfig new_config, u32 max_rows, u32 max_mines);

#endif