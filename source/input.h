// Copyright (c) 2026 Mike Wang
// SPDX-License-Identifier: MIT

#ifndef INPUT_H
#define INPUT_H

#include "tuxedo/types.h"

typedef struct Input {
    bool dpad_held_up;
    bool dpad_held_down;
    bool dpad_held_left;
    bool dpad_held_right;
    bool dpad_up;
    bool dpad_down;
    bool dpad_left;
    bool dpad_right;
    bool button_a;
    bool button_b;
    bool button_x;
    bool button_minus;
    bool button_plus;
    bool button_home;
    u32 timer;
    u32 repeat_count;
} Input;

void Input_Init(Input *input);
void Input_Read(Input *input);

#endif