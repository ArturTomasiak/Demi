#include "string.h"

void buffer_init(StringBuffer* restrict string) {
    string->allocated_memory = 500;
    string->buffer           = realloc(string->buffer, sizeof(char16_t) * string->allocated_memory);
    string->color_map        = realloc(string->color_map, sizeof(int32_t) * string->allocated_memory);
    if (!string->buffer || !string->color_map)
        fatal_error(u"memory allocation failed\nstring.c");
}

void buffer_init_str(StringBuffer* restrict string, uint64_t len, char16_t* str) {
    string->length = len;
    string->allocated_memory = len + 1;
    string->buffer           = realloc(string->buffer, sizeof(char16_t) * string->allocated_memory);
    string->color_map        = realloc(string->color_map, sizeof(int32_t) * string->allocated_memory);
    if (!string->buffer || !string->color_map) {
        fatal_error(u"memory allocation failed\nstring.c");
        return;
    }
    memcpy(string->buffer, str, len * sizeof(char16_t));
    string->last_change[0] = 0;
    string->last_change[1] = len;
    color_map_update(string);
}

void buffer_destruct(StringBuffer* restrict string) {
    string_history_destruct(string);
    if (string->buffer)
        free(string->buffer);
    if (string->color_map)
        free(string->color_map);
}

void buffer_add_char(StringBuffer* restrict string, char16_t ch, _Bool undo) {
    if (string->position > string->length)
        string->position = string->length;
    if (++string->length >= string->allocated_memory) {
        string->allocated_memory += 500;
        string->buffer            = realloc(string->buffer, sizeof(char16_t) * string->allocated_memory);
        string->color_map         = realloc(string->color_map, sizeof(int32_t) * string->allocated_memory);
        if (!string->buffer || !string->color_map) {
            fatal_error(u"memory allocation failed\nstring.c");
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

    if (!undo)
        undo_push(string, 0, string->position, 1, ch, NULL);
    color_map_update(string);
}

void buffer_rem_char(StringBuffer* restrict string, _Bool undo) {
    if (string->position > string->length || string->position == 0)
        return;
    if (!undo)
        undo_push(string, 1, string->position, 1, string->buffer[string->position - 1], NULL);
    memmove(string->buffer + string->position - 1, string->buffer + string->position, (string->length - string->position) * sizeof(char16_t));
    memmove(string->color_map + string->position - 1, string->color_map + string->position, (string->length - string->position) * sizeof(int32_t));
    string->length--;
    string->position--;
    string->buffer[string->length] = '\0';

    string->last_change[0] = string->position > 0 ? string->position - 1 : 0;
    string->last_change[1] = string->last_change[0];
    color_map_update(string);
} 

void buffer_add_string(StringBuffer* restrict string, uint64_t len, char16_t* str, _Bool undo) {
    if (!str || !*str)
        return;
    if (string->position > string->length)
        string->position = string->length;
    if (len >= 1) {
        string->length += len;
        string->allocated_memory += len;
        string->buffer            = realloc(string->buffer, string->allocated_memory * sizeof(char16_t));
        string->color_map         = realloc(string->color_map, string->allocated_memory * sizeof(int32_t));
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

        if (!undo)
            undo_push(string, 0, string->position, len, 0, str);
        color_map_update(string);
    }
}

void string_history_destruct(StringBuffer* restrict string) {
    for (uint64_t i = 0; i < string->undo_len; i++)
        if (string->undo[i].str)
            free(string->undo[i].str);
    for (uint64_t i = 0; i < string->redo_len; i++)
        if (string->redo[i].str)
            free(string->redo[i].str);
    if (string->undo)
        free(string->undo);
    if (string->redo)
        free(string->redo);
}

void undo_push(StringBuffer* restrict string, _Bool rem_char, uint64_t pos, uint64_t change_len, char16_t ch, char16_t* str) {
    if (string->undo_len == string->undo_alloc) {
        string->undo_alloc += 20;
        string->undo = realloc(string->undo, string->undo_alloc * sizeof(StringHistory));
    }
    StringHistory* current = &string->undo[string->undo_len++];
    current->rem_ch = rem_char;
    current->len    = change_len;
    current->pos    = pos;
    current->ch     = ch;
    if (str) {
        uint64_t len = sizeof(char16_t) * u_strlen(str);
        current->str = realloc(current->str, len + sizeof(char16_t));
        memcpy(current->str, str, len);
    }
}

void undo_pop(StringBuffer* restrict string) {
    if (string->undo_len == 0)
        return;
    StringHistory* current = &string->undo[--string->undo_len];
    string->position = current->pos;
    if (current->rem_ch && current->ch)
        buffer_add_char(string, current->ch, 1);
    else
        for (uint64_t i = 0; i < current->len; i++)
            buffer_rem_char(string, 1);
    redo_push(string, !current->rem_ch, current->pos, current->len, current->ch, current->str);
}

void redo_push(StringBuffer* restrict string, _Bool rem_char, uint64_t pos, uint64_t change_len, char16_t ch, char16_t* str) {
    if (string->redo_len == string->redo_alloc) {
        string->redo_alloc += 20;
        string->redo = realloc(string->redo, string->redo_alloc * sizeof(StringHistory));
    }
    StringHistory* current = &string->redo[string->redo_len++];
    current->rem_ch = rem_char;
    current->len    = change_len;
    current->pos    = pos;
    current->ch     = ch;
    if (str) {
        uint64_t len = sizeof(char16_t) * u_strlen(str);
        current->str = realloc(current->str, len + sizeof(char16_t));
        memcpy(current->str, str, len);
    }
}

void redo_pop(StringBuffer* restrict string) {
    if (string->redo_len == 0)
        return;
    StringHistory* current = &string->redo[--string->redo_len];
    if (current->rem_ch && current->ch)
        buffer_add_char(string, current->ch, 0);
    else if (current->rem_ch) 
        buffer_add_string(string, u_strlen(current->str), current->str, 0);
    else 
        for (uint64_t i = 0; i < current->len; i++)
            buffer_rem_char(string, 1);
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
    static uint32_t max = 0;
    if (max == 0)
        max = font->range[font->texture_count - 1][1];
    _Bool processed = font->character[ch].processed;

    if (ch == u' ')
        return 1;
    int16_t val = (int16_t)ch;
    if (val < 20 || val > max || !processed || iswcntrl(ch))
        return 0;
    return 1;
}

_Bool is_word_boundary(char16_t ch) {
    return ch == u' ' || ch == u'\n' ||
           ch == u'(' || ch == u')' || ch == u'{' || ch == u'}' || 
           ch == u'[' || ch == u']' || ch == u';' || ch == u',' ||
           ch == u'.' || ch == u':' || ch == u'*' || ch == u'\0';
}