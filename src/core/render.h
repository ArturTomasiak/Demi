#pragma once
#include "../include.h"
#include "../helper/math.h"
#include "../helper/globjects.h"

void render_init(int32_t width, int32_t height, DemiFont* restrict font);
void render_update_projection(int32_t width, int32_t height, DemiFont* restrict font);
void render_text_bind(DemiFont* restrict font, _Bool gui);
void render_text(DemiFont* restrict font, StringBuffer* restrict string, float start_x, float start_y, float dpi_scale);