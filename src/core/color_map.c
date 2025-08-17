#include "string.h"

static KeywordMap* restrict keyword;

static inline void search_nl(StringBuffer* restrict string, uint64_t* prev, uint64_t* next) {
    while (*prev > 0 && string->buffer[(*prev)--] != u'\n') ;
    while (*next < string->length && string->buffer[(*next)++] != u'\n') ;
}

void color_map_update(StringBuffer* restrict string) {
    uint64_t i   = string->last_change[0];
    uint64_t end = string->last_change[1];
    search_nl(string, &i, &end);
    memset(string->color_map + i, 0, (end - i) * sizeof(int32_t));

    while (i < end) {
        if (string->buffer[i] == u'/' && string->buffer[i + 1] == u'/') {
            while (i < end && string->buffer[i + 1] != u'\n')
                string->color_map[i++] = 1;
            i++;
            continue;
        }

        uint64_t j = i;
        while (j < string->length && !is_word_boundary(string->buffer[j]))
            j++;
        uint64_t strlen = j - i;
        if (strlen > 0) {
            char16_t word[strlen + 1];
            memcpy(word, string->buffer + i, strlen * sizeof(char16_t));
            word[strlen] = u'\0';

            if (is_keyword(word)) {
                for (uint8_t j = 0; j < strlen; j++) {
                    string->color_map[i++] = keyword->last_match->color;
                }
                continue;
            }
            else {
                i += strlen;
                continue;
            }
        }
        i++;
    }
}

void color_map_comment(StringBuffer* restrict string) {
    for (uint64_t j = 0; j < string->length; j++) {
        if (string->buffer[j] == u'/' && string->buffer[j + 1] == u'*') {
            while (j < string->length && !(string->buffer[j - 2] == u'*' && string->buffer[j - 1] == u'/'))
                string->color_map[j++] = 1;
        }
    }
}

static _Bool ustr_greater(const char16_t* a, const char16_t* b) {
    while (*a && *b && *a == *b) { a++; b++; }
    return *a > *b;
}

static void keyword_map_sort() {
    for (int i = 1; i < keyword->len; ++i) {
        KeywordMapEntry entry = keyword->map[i];
        int j = i - 1;

        while (j >= 0 && ustr_greater(keyword->map[j].str, entry.str)) {
            keyword->map[j + 1] = keyword->map[j];
            j--;
        }
        keyword->map[j + 1] = entry;
    }
}

void keyword_map_add_arr(char16_t** arr, uint8_t color_val) {
    uint16_t alloc = keyword->len;
    for (uint16_t i = 0; arr[i] != NULL; i++)
        alloc++;

    keyword->map = realloc(keyword->map, alloc * sizeof(KeywordMapEntry)); if (!keyword->map) goto fail;

    for (uint16_t i = 0; arr[i] != NULL; i++) {
        KeywordMapEntry* entry = &keyword->map[keyword->len];
        char16_t* str   = arr[i];
        uint16_t strlen = u_strlen(str);

        entry->color  = color_val;
        entry->str    = malloc((strlen + 1) * sizeof(char16_t)); if (!entry->str) goto fail;
        memcpy(entry->str, str, strlen * sizeof(char16_t));
        entry->str[strlen] = u'\0';

        keyword->len++;
    }

    keyword_map_sort();

    return;

fail:
    fatal_error(u"memory allocation failed\ncolor_map.c");
    return;
}

void keyword_map_add(char16_t* str, uint8_t color) {
    keyword->map = realloc(keyword->map, (keyword->len + 1) * sizeof(KeywordMapEntry)); if (!keyword->map) goto fail;

    KeywordMapEntry* entry = &keyword->map[keyword->len];
    uint16_t strlen = u_strlen(str);
    entry->str      = malloc((strlen + 1) * sizeof(char16_t)); if (!entry->str) goto fail;
    entry->color    = color;
    memcpy(entry->str, str, strlen * sizeof(char16_t));

    entry->str[strlen] = u'\0';
    keyword->len++;
    return;

fail:
    fatal_error(u"memory allocation failed\ncolor_map.c");
    return;
}

void keyword_map_init(KeywordMap* restrict map) {
    char16_t* c_keyword[39] = 
        {u"if", u"while", u"for", u"switch", u"else", u"return", u"goto", u"true", u"false", u"#include", 
        u"#pragma", u"#ifdef", u"#ifndef", u"#if", u"#endif", u"#elif", u"#define", u"#error", u"#else",
        u"static", u"extern", u"inline", u"typedef", u"const", u"enum", u"struct", u"++", u"--", u"register",
        u"NULL", u"null", u"volatile", u"restrict", u"union", u"typeof", u"continue", u"break", u"case", NULL};
    char16_t* c_variable[24] = 
        {u"void", u"bool", u"_Bool", u"char", u"short", u"int", u"long", u"signed", u"unsigned", u"float", u"double",
        u"int8_t", u"int16_t", u"int32_t", u"int64_t", u"uint8_t", u"uint16_t", u"uint32_t", u"uint64_t", u"char8_t", 
        u"char16_t", u"char32_t", u"wchar_t", NULL};

    keyword = map;

    keyword_map_add_arr(c_keyword, 3);
    keyword_map_add_arr(c_variable, 2);

    return;
}

void keyword_map_destruct() {
    if (!keyword->map)
        return;
    for (uint16_t i = 0; i < keyword->len; i++)
        if (keyword->map[i].str)
            free(keyword->map[i].str);
    free(keyword->map);
}

static _Bool cmp_str(char16_t* restrict str, char16_t* restrict compared_to) {
    while (*str && *compared_to)
        if (*str++ != *compared_to++)
            return 0;
    return *compared_to == '\0' && (*str == '\0' || is_word_boundary(*str));
}

_Bool is_keyword(char16_t* restrict word) {
    int low = 0;
    int high = keyword->len - 1;
    while (low <= high) {
        int mid = low + (high - low) / 2;
        KeywordMapEntry* current = &keyword->map[mid];

        if (cmp_str(word, current->str)) {
            keyword->last_match = current;
            return 1;
        }
        if (ustr_greater(word, current->str))
            low = mid + 1;
        else
            high = mid - 1;
    }
    return 0;
}