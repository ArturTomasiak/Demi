#pragma once
#include "../include.h"
#include "platform_layer.h"
#include "render.h"
#include "string.h"
#include "font.h"
#include "gui.h"

void editor_init();
void editor_destruct();

void editor_resize();
void editor_dpi_change(uint32_t dpi);

void editor_backspace();
void editor_tab();
void editor_enter();
void editor_input(char16_t ch);
void editor_paste(char16_t* str);
void editor_undo();
void editor_redo();
void editor_jump_top();
void editor_jump_bottom();
void editor_mouse_wheel(int32_t delta);
void editor_left_click(float x, float y);

void editor_up();
void editor_left();
void editor_right();
void editor_down();
void editor_camera_to_cursor(float x, float y, float advance, float nl_height, float min_x);