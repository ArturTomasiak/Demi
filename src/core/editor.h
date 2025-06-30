#pragma once
#include "../include.h"
#include "render.h"
#include "../helper/stringbuffer.h"

void editor_init(Editor* restrict editor);
void editor_destruct(Editor* restrict editor);

void editor_resize(Editor* restrict editor, DemiFont* restrict font);

void editor_backspace(Editor* restrict editor);
void editor_tab(Editor* restrict editor);
void editor_enter(Editor* restrict editor);
void editor_input(Editor* restrict editor, char16_t ch);
void editor_paste(Editor* restrict editor, char16_t* str);
_Bool is_printable(const DemiFont* restrict font, char16_t ch);