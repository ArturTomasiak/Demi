#pragma once

#include "../include.h"
#include "editor.h"

//////////////
// window.c //
//////////////

void platform_msg();
void platform_swap_buffers(Platform* restrict platform);
void platform_destruct(Platform* restrict platform);
_Bool platform_init(Platform* restrict platform, const char16_t* app_name);
void platform_show_window(Platform* restrict platform);
void platform_hide_window(Platform* restrict platform);
void platform_sleep(uint32_t millis);
void platform_vsync(_Bool on);

////////////
// file.c //
////////////

void file_open_explorer();
void file_open(const char16_t* file_path);
void file_close(uint8_t file);
void file_save(uint8_t saved_file);
void file_save_as();
void file_save_all();