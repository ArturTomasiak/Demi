#pragma once
#include "../include.h"
#include "../helper/globjects.h"

void gui_size(GUI* restrict gui, uint32_t width, uint32_t height, float dpi_scale);
void gui_init(GUI* restrict gui, _Bool gl46);
void gui_destruct(GUI* restrict gui);