// Copyright (c) 2026 Mike Wang
// SPDX-License-Identifier: MIT

#ifndef CONFIG_H
#define CONFIG_H

typedef struct GameConfig GameConfig;

void Config_ReadFromFile(GameConfig *config);
void Config_WriteToFile(const GameConfig *config);

#endif