#pragma once
#include "../include.h"
#include "platform_layer.h"
#include "render.h"
#include "string.h"
#include "font.h"
#include "gui.h"

void editor_init(Editor* restrict editor);
void editor_destruct(Editor* restrict editor);

void editor_resize(Editor* restrict editor);
void editor_dpi_change(Editor* restrict editor, DemiFont* restrict font, uint32_t dpi);

void editor_backspace(Editor* restrict editor);
void editor_tab(Editor* restrict editor);
void editor_enter(Editor* restrict editor);
void editor_input(Editor* restrict editor, char16_t ch);
void editor_paste(Editor* restrict editor, char16_t* str);
void editor_mouse_wheel(Editor* restrict editor, int32_t delta);
void editor_left_click(Editor* restrict editor, DemiFont* restrict font, float x, float y);

void editor_up(Editor* restrict editor);
void editor_left(Editor* restrict editor);
void editor_right(Editor* restrict editor);
void editor_down(Editor* restrict editor);
void editor_camera_to_cursor(Editor* restrict editor, float x, float y, float advance, float nl_height, float min_x);