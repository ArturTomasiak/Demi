#pragma once

#include "../include.h"
#include "editor.h"

void platform_msg(_Bool* running);
void platform_swap_buffers(Platform* restrict platform);
void platform_destruct(Platform* restrict platform);
_Bool platform_init(Platform* restrict platform, Editor* restrict editor, DemiFont* restrict font, const char16_t* app_name);
void platform_show_window(Platform* restrict platform);