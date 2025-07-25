#pragma once

#include "../include.h"
#include "globjects.h"
#include "render.h"
#include "gui.h"
#include <math.h>

void font_init(int32_t uniform_limit, _Bool gl46);
void font_rebuild(uint16_t new_font_size, float dpi_scale);
void font_destruct();