#include "stringbuffer.h"

void buffer_init(StringBuffer* restrict string) {
    string->allocated_memory = 500;
    string->buffer = realloc(string->buffer, sizeof(char16_t) * string->allocated_memory);
    if (!string->buffer)
        fatal_error(u"memory_allocation_failed");
}

void buffer_init_str(StringBuffer* restrict string, uint64_t len, char16_t* str) {
    string->length = len;
    string->allocated_memory = len;
    string->buffer = realloc(string->buffer, sizeof(char16_t) * string->allocated_memory);
    if (!string->buffer) {
        fatal_error(u"memory_allocation_failed");
        return;
    }
    memcpy(string->buffer, str, len * sizeof(char16_t));
}

void buffer_destruct(StringBuffer* restrict string) {
    if (string->buffer)
        free(string->buffer);
}

void buffer_add_char(StringBuffer* restrict string, char16_t ch) {
    if (string->position > string->length)
        string->position = string->length;
    if (++string->length >= string->allocated_memory) {
        string->allocated_memory += 500;
        string->buffer = realloc(string->buffer, sizeof(char16_t) * string->allocated_memory);
        if (!string->buffer) {
            fatal_error(u"memory_allocation_failed");
            return;
        }
    }
    memmove(string->buffer + string->position + 1, string->buffer + string->position, (string->length - string->position) * sizeof(char16_t));
    string->buffer[string->position] = ch;
    string->buffer[string->length] = '\0';
    string->position++;
}

void buffer_rem_char(StringBuffer* restrict string) {
    if (string->position > string->length || string->position == 0)
        return;
    memmove(string->buffer + string->position - 1, string->buffer + string->position, (string->length - string->position) * sizeof(char16_t));
    string->length--;
    string->position--;
    string->buffer[string->length] = '\0';
} 

void buffer_add_string(StringBuffer* restrict string, uint64_t len, char16_t* str) {
    if (!str || !*str)
        return;
    if (string->position > string->length)
        string->position = string->length;
    if (len >= 1) {
        string->length += len;
        string->buffer = realloc(string->buffer, string->length * sizeof(char16_t));
        if (!string->buffer) {
            fatal_error(u"failed to initialized content");
            return;
        }
        memmove(string->buffer + string->position + len, string->buffer + string->position, (string->length - string->position - len) * sizeof(char16_t));
        memcpy(string->buffer + string->position, str, len * sizeof(char16_t));
        string->buffer[string->length] = '\0';
        string->position = len;
    }
}

uint64_t u_strlen(const char16_t* str) {
    const char16_t* s = str;
    while (*s) ++s;
    return s - str;
}