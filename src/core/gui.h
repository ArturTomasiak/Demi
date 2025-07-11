#pragma once
#include "../include.h"
#include "../helper/globjects.h"

void gui_size(Editor* restrict editor, uint16_t font_size);
void gui_init(GUI* restrict gui, _Bool gl46);
void gui_destruct(GUI* restrict gui);