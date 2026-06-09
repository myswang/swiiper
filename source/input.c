#include "input.h"
#include "wiiuse/wpad.h"

const u32 REPEAT_DELAY_INTERVAL = 8;
const s32 GCC_STICK_DEADZONE = 60;
const s32 CLASSIC_STICK_DEADZONE = 15;

void Input_Init(Input *input) {
    WPAD_Init();
    WPAD_SetDataFormat(WPAD_CHAN_0, WPAD_FMT_BTNS);
    PAD_Init();
}

void Input_Read(Input *input) {
    WPAD_ScanPads();
    PAD_ScanPads();

    const u32 wiimote_up = WPAD_ButtonsUp(WPAD_CHAN_0);
    const u32 wiimote_held = WPAD_ButtonsHeld(WPAD_CHAN_0);
    const u32 wiimote_down = WPAD_ButtonsDown(WPAD_CHAN_0);
    const u32 gcc_up = PAD_ButtonsUp(PAD_CHAN0);
    const u32 gcc_held = PAD_ButtonsHeld(PAD_CHAN0);
    const u32 gcc_down = PAD_ButtonsDown(PAD_CHAN0);

    const s8 gcc_stick_x = PAD_StickX(PAD_CHAN0);
    const s8 gcc_stick_y = PAD_StickY(PAD_CHAN0);

    WPADData *wd = WPAD_Data(WPAD_CHAN_0);

    s8 wiichuck_stick_x = 0;
    s8 wiichuck_stick_y = 0;
    s8 classic_stick_x = 0;
    s8 classic_stick_y = 0;

    bool is_nunchuk = wd->exp.type == WPAD_EXP_NUNCHUK;
    bool is_classic = wd->exp.type == WPAD_EXP_CLASSIC;

    if (is_nunchuk) {
        wiichuck_stick_x = wd->exp.nunchuk.js.pos.x - 128;
        wiichuck_stick_y = wd->exp.nunchuk.js.pos.y - 128;
    }

    if (is_classic) {
        classic_stick_x = wd->exp.classic.ljs.pos.x - 32;
        classic_stick_y = wd->exp.classic.ljs.pos.y - 32;
    }

    input->button_home = wiimote_down & WPAD_BUTTON_HOME ||
                         wiimote_down & WPAD_CLASSIC_BUTTON_HOME;
    input->button_plus = wiimote_down & WPAD_BUTTON_PLUS ||
                         wiimote_down & WPAD_CLASSIC_BUTTON_PLUS ||
                         gcc_down & PAD_BUTTON_START;
    input->button_minus = wiimote_down & WPAD_BUTTON_MINUS ||
                          wiimote_down & WPAD_CLASSIC_BUTTON_MINUS;
    input->button_x = wiimote_up & WPAD_CLASSIC_BUTTON_X ||
                      gcc_up & PAD_BUTTON_X ||
                      (is_nunchuk && wiimote_up & WPAD_NUNCHUK_BUTTON_Z);
    input->button_b = wiimote_up & WPAD_BUTTON_1 ||
                      wiimote_up & WPAD_CLASSIC_BUTTON_B ||
                      gcc_up & PAD_BUTTON_B || wiimote_up & WPAD_BUTTON_B;
    input->button_a = wiimote_up & WPAD_BUTTON_2 ||
                      wiimote_up & WPAD_CLASSIC_BUTTON_A ||
                      gcc_up & PAD_BUTTON_A || wiimote_up & WPAD_BUTTON_A;

    bool dpad_held_up = wiimote_held & WPAD_BUTTON_RIGHT ||
                        (is_classic && wiimote_held & WPAD_CLASSIC_BUTTON_UP) ||
                        gcc_held & PAD_BUTTON_UP ||
                        gcc_stick_y >= GCC_STICK_DEADZONE ||
                        wiichuck_stick_y >= GCC_STICK_DEADZONE ||
                        classic_stick_y >= CLASSIC_STICK_DEADZONE;
    bool dpad_held_down = wiimote_held & WPAD_BUTTON_LEFT ||
                          wiimote_held & WPAD_CLASSIC_BUTTON_DOWN ||
                          gcc_held & PAD_BUTTON_DOWN ||
                          gcc_stick_y < (-1 * GCC_STICK_DEADZONE) ||
                          wiichuck_stick_y < (-1 * GCC_STICK_DEADZONE) ||
                          classic_stick_y < (-1 * CLASSIC_STICK_DEADZONE);
    bool dpad_held_left = wiimote_held & WPAD_BUTTON_UP ||
                          wiimote_held & WPAD_CLASSIC_BUTTON_LEFT ||
                          gcc_held & PAD_BUTTON_LEFT ||
                          gcc_stick_x < (-1 * GCC_STICK_DEADZONE) ||
                          wiichuck_stick_x < (-1 * GCC_STICK_DEADZONE) ||
                          classic_stick_x < (-1 * CLASSIC_STICK_DEADZONE);
    bool dpad_held_right = wiimote_held & WPAD_BUTTON_DOWN ||
                           wiimote_held & WPAD_CLASSIC_BUTTON_RIGHT ||
                           gcc_held & PAD_BUTTON_RIGHT ||
                           gcc_stick_x >= GCC_STICK_DEADZONE ||
                           wiichuck_stick_x >= GCC_STICK_DEADZONE ||
                           classic_stick_x >= CLASSIC_STICK_DEADZONE;

    input->dpad_up =
        wiimote_down & WPAD_BUTTON_RIGHT ||
        (is_classic && wiimote_down & WPAD_CLASSIC_BUTTON_UP) ||
        gcc_down & PAD_BUTTON_UP ||
        (gcc_stick_y >= GCC_STICK_DEADZONE && input->repeat_count == 0) ||
        (wiichuck_stick_y >= GCC_STICK_DEADZONE && input->repeat_count == 0) ||
        (classic_stick_y >= CLASSIC_STICK_DEADZONE && input->repeat_count == 0);
    input->dpad_down =
        wiimote_down & WPAD_BUTTON_LEFT ||
        wiimote_down & WPAD_CLASSIC_BUTTON_DOWN || gcc_down & PAD_BUTTON_DOWN ||
        (gcc_stick_y < (-1 * GCC_STICK_DEADZONE) && input->repeat_count == 0) ||
        (wiichuck_stick_y < (-1 * GCC_STICK_DEADZONE) &&
         input->repeat_count == 0) ||
        (classic_stick_y < (-1 * CLASSIC_STICK_DEADZONE) &&
         input->repeat_count == 0);
    input->dpad_left =
        wiimote_down & WPAD_BUTTON_UP ||
        wiimote_down & WPAD_CLASSIC_BUTTON_LEFT || gcc_down & PAD_BUTTON_LEFT ||
        (gcc_stick_x < (-1 * GCC_STICK_DEADZONE) && input->repeat_count == 0) ||
        (wiichuck_stick_x < (-1 * GCC_STICK_DEADZONE) &&
         input->repeat_count == 0) ||
        (classic_stick_x < (-1 * CLASSIC_STICK_DEADZONE) &&
         input->repeat_count == 0);
    input->dpad_right =
        wiimote_down & WPAD_BUTTON_DOWN ||
        wiimote_down & WPAD_CLASSIC_BUTTON_RIGHT ||
        gcc_down & PAD_BUTTON_RIGHT ||
        (gcc_stick_x >= GCC_STICK_DEADZONE && input->repeat_count == 0) ||
        (wiichuck_stick_x >= GCC_STICK_DEADZONE && input->repeat_count == 0) ||
        (classic_stick_x >= CLASSIC_STICK_DEADZONE && input->repeat_count == 0);

    if (!dpad_held_up && !dpad_held_down && !dpad_held_left &&
        !dpad_held_right) {
        input->timer = 0;
        input->repeat_count = 0;
    }

    if (input->timer == 0) {
        input->dpad_held_up = dpad_held_up;
        input->dpad_held_down = dpad_held_down;
        input->dpad_held_left = dpad_held_left;
        input->dpad_held_right = dpad_held_right;
        if (input->dpad_held_up || input->dpad_held_down ||
            input->dpad_held_left || input->dpad_held_right) {
            input->timer = REPEAT_DELAY_INTERVAL;
            input->repeat_count++;
        }
    } else {
        input->dpad_held_up = false;
        input->dpad_held_down = false;
        input->dpad_held_left = false;
        input->dpad_held_right = false;
        input->timer--;
    }
}
