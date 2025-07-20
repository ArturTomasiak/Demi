#pragma once

#include "../include.h"
#include "editor.h"

//////////////
// window.c //
//////////////

void platform_msg(_Bool* running);
void platform_swap_buffers(Platform* restrict platform);
void platform_destruct(Platform* restrict platform);
_Bool platform_init(Platform* restrict platform, Editor* restrict editor, DemiFont* restrict font, const char16_t* app_name);
void platform_show_window(Platform* restrict platform);
void platform_hide_window(Platform* restrict platform);
void platform_sleep(uint32_t millis);

////////////
// file.c //
////////////

void file_open_explorer(Editor* restrict editor);
void file_open(Editor* restrict editor, const char16_t* file_path);
void file_close(Editor* restrict editor, uint8_t file);
void file_save(Editor* restrict editor, uint8_t saved_file);
void file_save_as(Editor* restrict editor);
void file_save_all(Editor* restrict editor);