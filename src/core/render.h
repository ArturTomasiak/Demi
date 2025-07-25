#pragma once
#include "../include.h"
#include "math.h"
#include "string.h"
#include "globjects.h"

void render_init(int32_t width, int32_t height, DemiFile* restrict file);
void render_update_nl_height(float new_height);
void render_gui_projection(int32_t width, int32_t height);
void render_content_projection(int32_t width, int32_t height, DemiFile* restrict file);
void render_text_bind(_Bool gui);
void render_text(char16_t* restrict buffer, int32_t* restrict color_map, uint64_t len, float start_x, float start_y, _Bool gui);
void render_gui();