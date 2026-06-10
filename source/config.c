// Copyright (c) 2026 Mike Wang
// SPDX-License-Identifier: MIT

#include "config.h"
#include "game.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const char *SAVE_PATH = "sd:/apps/swiiper/config.ini";

void Config_ReadFromFile(GameConfig *config) {
    FILE *file = fopen(SAVE_PATH, "r");
    if (file == NULL)
        return;

    char line[128];
    char key[64];
    char val_str[64];

    while (fgets(line, sizeof(line), file) != NULL) {
        if (line[0] == '\n' || line[0] == '\r' || line[0] == '#')
            continue;

        if (sscanf(line, "%63[^=] = %63[^\n]", key, val_str) == 2) {
            if (strcmp(key, "num_cols") == 0)
                config->num_cols = (u32)atoi(val_str);
            else if (strcmp(key, "num_rows") == 0)
                config->num_rows = (u32)atoi(val_str);
            else if (strcmp(key, "num_mines") == 0)
                config->num_mines = (u32)atoi(val_str);
        }
    }
    fclose(file);
}

void Config_WriteToFile(const GameConfig *config) {
    FILE *file = fopen(SAVE_PATH, "w");
    if (file == NULL)
        return;

    fprintf(file, "num_cols=%u\n", config->num_cols);
    fprintf(file, "num_rows=%u\n", config->num_rows);
    fprintf(file, "num_mines=%u\n", config->num_mines);

    fclose(file);
}