#pragma once

#include "../include.h"
#include "../helper/globjects.h"
#include "render.h"

void font_init(DemiFont* restrict font, int32_t uniform_limit, float dpi_scale, _Bool gl46);
void font_rebuild(DemiFont* restrict font, uint16_t new_font_size, float dpi_scale);
void font_destruct(DemiFont* restrict font);