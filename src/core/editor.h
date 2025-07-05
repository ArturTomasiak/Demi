#pragma once
#include "../include.h"
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

_Bool is_printable(const DemiFont* restrict font, char16_t ch);