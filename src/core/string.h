#pragma once
#include "../include.h"

void buffer_init(StringBuffer* restrict string);
void buffer_init_str(StringBuffer* restrict string, uint64_t len, char16_t* str);
void buffer_destruct(StringBuffer* restrict string);
void buffer_add_char(StringBuffer* restrict string, char16_t ch);
void buffer_rem_char(StringBuffer* restrict string);
void buffer_add_string(StringBuffer* restrict string, uint64_t len, char16_t* str);

uint64_t u_strlen(const char16_t* restrict str);
uint16_t num_count(uint16_t x);
char16_t* num_to_ustr(uint16_t num, uint16_t* result_len);
_Bool is_printable(char16_t ch);

// color_map.c
typedef struct {
    char16_t*** map;
    uint16_t*   alloc_map;
    uint8_t**   color_map;
    uint8_t     last_color;
    uint8_t     last_len; 
    uint8_t     map_len;
} KeywordMap;

void color_map_update(StringBuffer* restrict string);
void keyword_map_init(KeywordMap* restrict map);
void keyword_map_destruct();
_Bool is_keyword(char16_t* word);