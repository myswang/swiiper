// Copyright (c) 2026 Mike Wang
// SPDX-License-Identifier: MIT

#include "menu.h"
#include <stdio.h>

static char custom_label_buffer[64];

static MenuOption difficulty_options[] = {
    {"Beginner (9x9, 10 mines)", EASY_MODE},
    {"Intermediate (16x16, 40 mines)", MEDIUM_MODE},
    {"Expert (placeholder)", {0}},
    {"Custom (placeholder)", {0}},
    {"Edit custom settings", {0}}};

Menu difficulty_menu = {.title = "Select game difficulty:",
                        .options = difficulty_options,
                        .num_options = 5,
                        .cursor = 0};

static MenuOption pause_options[] = {
    {"Back to previous game", {0}}, {"New game", {0}}, {"Quit to loader", {0}}};

Menu pause_menu = {.title = "GAME PAUSED!",
                   .options = pause_options,
                   .num_options = 3,
                   .cursor = 0};

static char custom_cols_buffer[64];
static char custom_rows_buffer[64];
static char custom_mines_buffer[64];

static MenuOption custom_options[] = {{"Columns (x): (placeholder)", {0}},
                                      {"Rows (y): (placeholder)", {0}},
                                      {"Mines: (placeholder)", {0}},
                                      {"Confirm", {0}},
                                      {"Back", {0}}};

Menu custom_menu = {.title = "Custom game options:",
                    .options = custom_options,
                    .num_options = 5,
                    .cursor = 0};

void Menu_UpdateCustomLabel(GameConfig new_config) {
    snprintf(custom_label_buffer, sizeof(custom_label_buffer),
             "Custom (%ux%u, %u mines)", new_config.num_cols,
             new_config.num_rows, new_config.num_mines);

    difficulty_menu.options[3].config = new_config;
    difficulty_menu.options[3].label = custom_label_buffer;
}

void Menu_UpdateCustomMenu(GameConfig new_config, u32 max_cols, u32 max_mines) {
    snprintf(custom_cols_buffer, sizeof(custom_cols_buffer),
             "Columns (x): %c %u %c", new_config.num_cols > MIN_W ? '<' : ' ',
             new_config.num_cols, new_config.num_cols < max_cols ? '>' : ' ');
    custom_menu.options[0].label = custom_cols_buffer;

    snprintf(custom_rows_buffer, sizeof(custom_rows_buffer),
             "Rows (y):    %c %u %c", new_config.num_rows > MIN_H ? '<' : ' ',
             new_config.num_rows, new_config.num_rows < MAX_H ? '>' : ' ');
    custom_menu.options[1].label = custom_rows_buffer;

    snprintf(custom_mines_buffer, sizeof(custom_mines_buffer),
             "Mines:       %c %u %c",
             new_config.num_mines > MIN_MINES ? '<' : ' ', new_config.num_mines,
             new_config.num_mines < max_mines ? '>' : ' ');
    custom_menu.options[2].label = custom_mines_buffer;
}