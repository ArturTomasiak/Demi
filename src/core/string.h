#pragma once
#include "../include.h"

void buffer_init(StringBuffer* restrict string);
void buffer_init_str(StringBuffer* restrict string, uint64_t len, char16_t* str);
void buffer_destruct(StringBuffer* restrict string);
void buffer_add_char(StringBuffer* restrict string, char16_t ch, _Bool undo);
void buffer_rem_char(StringBuffer* restrict string, _Bool undo);
void buffer_rem_len(StringBuffer* restrict string, uint64_t len, _Bool undo);
void buffer_add_string(StringBuffer* restrict string, uint64_t len, char16_t* str, _Bool undo);

void string_history_destruct(StringBuffer* restrict string);
void undo_push(StringBuffer* restrict string, _Bool rem_char, uint64_t pos, uint64_t change_len, char16_t ch, char16_t* str);
void undo_pop(StringBuffer* restrict string);
void redo_push(StringBuffer* restrict string, _Bool rem_char, uint64_t pos, uint64_t change_len, char16_t ch, char16_t* str);
void redo_pop(StringBuffer* restrict string);

uint64_t u_strlen(const char16_t* restrict str);
uint16_t num_count(uint16_t x);
char16_t* num_to_ustr(uint16_t num, uint16_t* result_len);
_Bool is_printable(char16_t ch);
_Bool is_word_boundary(char16_t ch);

// color_map.c

void color_map_update(StringBuffer* restrict string);
void color_map_comment(StringBuffer* restrict string);
void keyword_map_add_arr(char16_t** arr, uint8_t color_val);
void keyword_map_add(char16_t* str, uint8_t color);
void keyword_map_init(KeywordMap* restrict map);
void keyword_map_destruct();
_Bool is_keyword(char16_t* word);