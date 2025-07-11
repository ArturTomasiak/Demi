#pragma once
#include "../include.h"
#include "../helper/math.h"
#include "string.h"
#include "../helper/globjects.h"

void render_init(int32_t width, int32_t height, DemiFile* restrict file, DemiFont* restrict font_ptr);
void render_gui_projection(int32_t width, int32_t height);
void render_content_projection(int32_t width, int32_t height, DemiFile* restrict file);
void render_text_bind(_Bool gui);
void render_text(char16_t* restrict buffer, int32_t* restrict color_map, uint64_t len, float start_x, float start_y, Editor* restrict editor);
void render_gui(Editor* restrict editor);