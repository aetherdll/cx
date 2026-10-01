// 7 DEVELOMENT. ALL RIGHTS RESERVED.

#ifndef CX_INPUT_H
#define CX_INPUT_H

#ifdef __cplusplus
extern "C" {
#endif

#define CX_KEY_UNKNOWN 0
#define CX_KEY_BACKSPACE 0x08
#define CX_KEY_TAB 0x09
#define CX_KEY_ENTER 0x0D
#define CX_KEY_SHIFT 0x10
#define CX_KEY_CONTROL 0x11
#define CX_KEY_ALT 0x12
#define CX_KEY_ESCAPE 0x1B
#define CX_KEY_SPACE 0x20

#define CX_KEY_LEFT 0x25
#define CX_KEY_UP 0x26
#define CX_KEY_RIGHT 0x27
#define CX_KEY_DOWN 0x28

#define CX_KEY_0 0x30
#define CX_KEY_1 0x31
#define CX_KEY_2 0x32
#define CX_KEY_3 0x33
#define CX_KEY_4 0x34
#define CX_KEY_5 0x35
#define CX_KEY_6 0x36
#define CX_KEY_7 0x37
#define CX_KEY_8 0x38
#define CX_KEY_9 0x39

#define CX_KEY_A 0x41
#define CX_KEY_B 0x42
#define CX_KEY_C 0x43
#define CX_KEY_D 0x44
#define CX_KEY_E 0x45
#define CX_KEY_F 0x46
#define CX_KEY_G 0x47
#define CX_KEY_H 0x48
#define CX_KEY_I 0x49
#define CX_KEY_J 0x4A
#define CX_KEY_K 0x4B
#define CX_KEY_L 0x4C
#define CX_KEY_M 0x4D
#define CX_KEY_N 0x4E
#define CX_KEY_O 0x4F
#define CX_KEY_P 0x50
#define CX_KEY_Q 0x51
#define CX_KEY_R 0x52
#define CX_KEY_S 0x53
#define CX_KEY_T 0x54
#define CX_KEY_U 0x55
#define CX_KEY_V 0x56
#define CX_KEY_W 0x57
#define CX_KEY_X 0x58
#define CX_KEY_Y 0x59
#define CX_KEY_Z 0x5A

#define CX_MOUSE_LEFT 0
#define CX_MOUSE_RIGHT 1
#define CX_MOUSE_MIDDLE 2

void cx_input_init(void);
void cx_input_update(void);
int cx_input_key_down(int key);
int cx_input_key_pressed(int key);
int cx_input_key_released(int key);
int cx_input_mouse_down(int button);
int cx_input_mouse_pressed(int button);
int cx_input_mouse_released(int button);
void cx_input_get_mouse_pos(int* x, int* y);

#ifdef __cplusplus
}
#endif

#endif

#ifdef CX_INPUT_IMPLEMENTATION
#ifndef CX_INPUT_IMPLEMENTATION_ONCE
#define CX_INPUT_IMPLEMENTATION_ONCE

#include 

#ifdef _WIN32
#include 
#endif

typedef struct {
    unsigned char current_keys[256];
    unsigned char previous_keys[256];
    unsigned char current_mouse[3];
    unsigned char previous_mouse[3];
    int mouse_x, mouse_y;
} cx_InputState;

static cx_InputState cx__input = {0};

void cx_input_init(void) {
    memset(&cx__input, 0, sizeof(cx_InputState));
}

void cx_input_update(void) {
    memcpy(cx__input.previous_keys, cx__input.current_keys, sizeof(cx__input.current_keys));
    memcpy(cx__input.previous_mouse, cx__input.current_mouse, sizeof(cx__input.current_mouse));

    #ifdef _WIN32
    for (int i = 0; i < 256; i++) {
        cx__input.current_keys[i] = (GetAsyncKeyState(i) & 0x8000) ? 1 : 0;
    }

    cx__input.current_mouse[CX_MOUSE_LEFT] = (GetAsyncKeyState(VK_LBUTTON) & 0x8000) ? 1 : 0;
    cx__input.current_mouse[CX_MOUSE_RIGHT] = (GetAsyncKeyState(VK_RBUTTON) & 0x8000) ? 1 : 0;
    cx__input.current_mouse[CX_MOUSE_MIDDLE] = (GetAsyncKeyState(VK_MBUTTON) & 0x8000) ? 1 : 0;

    POINT p;
    if (GetCursorPos(&p)) {
        cx__input.mouse_x = p.x;
        cx__input.mouse_y = p.y;
    }
    #endif
}

int cx_input_key_down(int key) {
    if (key < 0 || key >= 256) return 0;
    return cx__input.current_keys[key];
}

int cx_input_key_pressed(int key) {
    if (key < 0 || key >= 256) return 0;
    return cx__input.current_keys[key] && !cx__input.previous_keys[key];
}

int cx_input_key_released(int key) {
    if (key < 0 || key >= 256) return 0;
    return !cx__input.current_keys[key] && cx__input.previous_keys[key];
}

int cx_input_mouse_down(int button) {
    if (button < 0 || button >= 3) return 0;
    return cx__input.current_mouse[button];
}

int cx_input_mouse_pressed(int button) {
    if (button < 0 || button >= 3) return 0;
    return cx__input.current_mouse[button] && !cx__input.previous_mouse[button];
}

int cx_input_mouse_released(int button) {
    if (button < 0 || button >= 3) return 0;
    return !cx__input.current_mouse[button] && cx__input.previous_mouse[button];
}

void cx_input_get_mouse_pos(int* x, int* y) {
    if (x) *x = cx__input.mouse_x;
    if (y) *y = cx__input.mouse_y;
}

#endif
#endif