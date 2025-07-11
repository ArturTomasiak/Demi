#pragma once

#include "../include.h"
#include "../helper/globjects.h"
#include "render.h"
#include "gui.h"

void font_init(DemiFont* restrict font, int32_t uniform_limit, _Bool gl46);
void font_rebuild(DemiFont* restrict font, Editor* restrict editor, uint16_t new_font_size, float dpi_scale);
void font_destruct(DemiFont* restrict font);