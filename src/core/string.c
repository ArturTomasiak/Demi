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
    string->allocated_memory = len;
    string->buffer = realloc(string->buffer, sizeof(char16_t) * string->allocated_memory);
    string->color_map = realloc(string->color_map, sizeof(int32_t) * string->allocated_memory);
    if (!string->buffer || !string->color_map) {
        fatal_error(u"memory_allocation_failed");
        return;
    }
    memcpy(string->buffer, str, len * sizeof(char16_t));
    buffer_update_color_map(string);
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
    string->buffer[string->position] = ch;
    string->buffer[string->length] = '\0';
    string->position++;
    buffer_update_color_map(string);
}

void buffer_rem_char(StringBuffer* restrict string) {
    if (string->position > string->length || string->position == 0)
        return;
    memmove(string->buffer + string->position - 1, string->buffer + string->position, (string->length - string->position) * sizeof(char16_t));
    string->length--;
    string->position--;
    string->buffer[string->length] = '\0';
    buffer_update_color_map(string);
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
        memcpy(string->buffer + string->position, str, len * sizeof(char16_t));
        string->buffer[string->length] = '\0';
        string->position += len;
    }
    buffer_update_color_map(string);
}

void buffer_update_color_map(StringBuffer* restrict string) {
    memset(string->color_map, 0, string->length * sizeof(int32_t)); // placeholder
}

uint64_t u_strlen(const char16_t* str) {
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

extern inline void setup_lines_rendering(Editor* restrict editor) {
    StringBuffer* current = &editor->files[editor->current_file].string;
    uint64_t lines = 1;
    uint64_t alloc = 2;
    for (int i = 0; i < current->length; i++) {
        if (current->buffer[i] == u'\n') {
            lines++;
            alloc += num_count(lines) + 1;
        }
    }
    RenderData* data = &editor->data;
    data->lines     = realloc(data->lines, (alloc + 1) * sizeof(char16_t));
    data->color_map = realloc(data->color_map, alloc * sizeof(int32_t));
    for (uint64_t i = 0; i < alloc; i++)
        data->color_map[i] = 1;
    uint64_t index = 0;
    uint16_t temp_len;
    for (int i = 0; i < lines; i++) {
        char16_t* temp = num_to_ustr(i + 1, &temp_len);
        memcpy(data->lines + index, temp, temp_len * sizeof(char16_t));
        index += temp_len;
        free(temp);
    }
    data->lines_len = alloc;
    data->last_line = lines;
}