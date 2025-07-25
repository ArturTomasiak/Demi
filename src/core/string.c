#include "string.h"

void buffer_init(StringBuffer* restrict string) {
    string->allocated_memory = 500;
    string->buffer    = realloc(string->buffer, sizeof(char16_t) * string->allocated_memory);
    string->color_map = realloc(string->color_map, sizeof(int32_t) * string->allocated_memory);
    if (!string->buffer || !string->color_map)
        fatal_error(u"memory_allocation_failed");
}

void buffer_init_str(StringBuffer* restrict string, uint64_t len, char16_t* str) {
    string->length = len;
    string->allocated_memory = len + 1;
    string->buffer = realloc(string->buffer, sizeof(char16_t) * string->allocated_memory);
    string->color_map = realloc(string->color_map, sizeof(int32_t) * string->allocated_memory);
    if (!string->buffer || !string->color_map) {
        fatal_error(u"memory_allocation_failed");
        return;
    }
    memcpy(string->buffer, str, len * sizeof(char16_t));
    string->last_change[0] = 0;
    string->last_change[1] = len;
    color_map_update(string);
}

void buffer_destruct(StringBuffer* restrict string) {
    if (string->buffer)
        free(string->buffer);
    if (string->color_map)
        free(string->color_map);
}

void buffer_add_char(StringBuffer* restrict string, char16_t ch) {
    if (string->position > string->length)
        string->position = string->length;
    if (++string->length >= string->allocated_memory) {
        string->allocated_memory += 500;
        string->buffer = realloc(string->buffer, sizeof(char16_t) * string->allocated_memory);
        string->color_map = realloc(string->color_map, sizeof(int32_t) * string->allocated_memory);
        if (!string->buffer || !string->color_map) {
            fatal_error(u"memory_allocation_failed");
            return;
        }
    }
    memmove(string->buffer + string->position + 1, string->buffer + string->position, (string->length - string->position) * sizeof(char16_t));
    memmove(string->color_map + string->position + 1, string->color_map + string->position, (string->length - string->position) * sizeof(int32_t));
    string->buffer[string->position] = ch;
    string->buffer[string->length] = '\0';
    string->last_change[0] = string->position;
    string->last_change[1] = string->position;
    string->position++;

    if (ch == u'\n')
        return;

    color_map_update(string);
}

void buffer_rem_char(StringBuffer* restrict string) {
    if (string->position > string->length || string->position == 0)
        return;
    memmove(string->buffer + string->position - 1, string->buffer + string->position, (string->length - string->position) * sizeof(char16_t));
    memmove(string->color_map + string->position - 1, string->color_map + string->position, (string->length - string->position) * sizeof(int32_t));
    string->length--;
    string->position--;
    string->buffer[string->length] = '\0';

    string->last_change[0] = string->position > 0 ? string->position - 1 : 0;
    string->last_change[1] = string->last_change[0];
    color_map_update(string);
} 

void buffer_add_string(StringBuffer* restrict string, uint64_t len, char16_t* str) {
    if (!str || !*str)
        return;
    if (string->position > string->length)
        string->position = string->length;
    if (len >= 1) {
        string->length += len;
        string->allocated_memory += len;
        string->buffer = realloc(string->buffer, string->allocated_memory * sizeof(char16_t));
        string->color_map = realloc(string->color_map, string->allocated_memory * sizeof(int32_t));
        if (!string->buffer || !string->color_map) {
            fatal_error(u"failed to initialized content");
            return;
        }
        memmove(string->buffer + string->position + len, string->buffer + string->position, (string->length - string->position - len) * sizeof(char16_t));
        memmove(string->color_map + string->position + len, string->color_map + string->position, (string->length - string->position - len) * sizeof(int32_t));
        memcpy(string->buffer + string->position, str, len * sizeof(char16_t));
        string->buffer[string->length] = '\0';
        string->last_change[0] = string->position;
        string->position += len;
        string->last_change[1] = string->position;
        color_map_update(string);
    }
}

uint64_t u_strlen(const char16_t* restrict str) {
    const char16_t* s = str;
    while (*s) ++s;
    return s - str;
}

uint16_t num_count(uint16_t x) {
    uint16_t result = 0;
    while (x != 0) {
        x /= 10;
        result++;
    }
    return result;
}

char16_t* num_to_ustr(uint16_t num, uint16_t* result_len) {
    uint16_t len  = num_count(num);
    *result_len = len + 1;
    char16_t* str = malloc((len + 1) * sizeof(char16_t));
    int16_t i = len - 1;
    while(num != 0) {
        uint16_t digit = num % 10;
        str[i--] = u'0' + digit;
        num /= 10; 
    }
    str[len] = u'\n';
    return str;
}

_Bool is_printable(char16_t ch) {
    if (ch == u' ')
        return 1;
    int16_t val = (int16_t)ch;
    if (val < 20 || val > font->range[font->texture_count - 1][1] || !font->character[ch].processed)
        return 0;
    return 1;
}