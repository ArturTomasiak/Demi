#pragma once
#include "../include.h"

void buffer_init(StringBuffer* restrict string);
void buffer_init_str(StringBuffer* restrict string, uint64_t len, char16_t* str);
void buffer_destruct(StringBuffer* restrict string);
void buffer_add_char(StringBuffer* restrict string, char16_t ch);
void buffer_rem_char(StringBuffer* restrict string);
void buffer_add_string(StringBuffer* restrict string, uint64_t len, char16_t* str);

uint64_t u_strlen(const char16_t *str);