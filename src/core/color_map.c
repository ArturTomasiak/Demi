#include "string.h"

static KeywordMap* restrict keyword;

static _Bool is_word_boundary(char16_t ch) {
    return ch == u' ' || ch == u'+' || ch == u'-' || ch == u'\n' ||
           ch == u'(' || ch == u')' || ch == u'{' || ch == u'}' || 
           ch == u'[' || ch == u']' || ch == u';' || ch == u',' ||
           ch == u'.' || ch == u':' || ch == u'*' || ch == u'/' ||
           ch == u'\0';
}

static inline void search_nl(StringBuffer* restrict string, uint64_t* prev, uint64_t* next) {
    while (*prev > 0 && string->buffer[(*prev)--] != u'\n') ;
    while (*next < string->length && string->buffer[(*next)++] != u'\n') ;
}

void color_map_update(StringBuffer* restrict string) {
    uint64_t i   = string->last_change[0];
    uint64_t end = string->last_change[1];
    search_nl(string, &i, &end);

    while (i < end) {
        string->color_map[i] = 0;

        if (i < end && string->buffer[i] == u'/' && string->buffer[i + 1] == u'/') {
            while (i < end && string->buffer[i + 1] != u'\n')
                string->color_map[i++] = 1;
            goto end;
        }

        if (i != 0 && string->color_map[i - 1] != 3 && !is_word_boundary(string->buffer[i - 1]))
            goto end;

        if (is_keyword(string->buffer + i)) {
            for (uint8_t j = 0; j < keyword->last_len; j++) {
                string->color_map[i++] = keyword->last_color;
            }
            i--;
            goto end;
        }

end:
        i++;
    }
}

static void keyword_map_add(const char16_t** arr, uint8_t color_val) {
    uint8_t i = 0;
    while (arr[i] != NULL) {
        uint32_t ch = (uint32_t)arr[i][0];
        if (ch >= keyword->map_len)
            goto while_end;
        uint16_t idx_len = ++keyword->alloc_map[ch];
        uint16_t str_len = u_strlen(arr[i]) + 1;

        keyword->map[ch]   = realloc(keyword->map[ch], idx_len * sizeof(char16_t*));         if (!keyword->map[ch])       goto fail;
        keyword->color_map[ch] = realloc(keyword->color_map[ch], idx_len * sizeof(uint8_t)); if (!keyword->color_map[ch]) goto fail;
        idx_len--;
        keyword->color_map[ch][idx_len] = color_val;
        keyword->map[ch][idx_len]   = malloc(str_len * sizeof(char16_t)); 
        memcpy(keyword->map[ch][idx_len], arr[i], str_len * sizeof(char16_t));   
while_end:
        i++;
    }

    return;

fail:
    fatal_error(u"memory allocation failed");
    return;
}

void keyword_map_init(KeywordMap* restrict map) {
    const char16_t* c_keyword[27] = 
        {u"if", u"while", u"for", u"switch", u"else", u"return", u"goto", u"#include", 
        u"#pragma", u"#ifdef", u"#ifndef", u"#if", u"#endif", u"#elif", u"#define", u"#error", u"#else",
        u"static", u"extern", u"inline", u"typedef", u"const", u"enum", u"struct", u"++", u"--", NULL};
    const char16_t* c_variable[20] = 
        {u"void", u"bool", u"_Bool", u"char", u"short", u"int", u"long", u"signed", u"unsigned", u"float", u"double",
        u"int8_t", u"int16_t", u"int32_t", u"int64_t", u"uint8_t", u"uint16_t", u"uint32_t", u"uint64_t", NULL};

    keyword = map;

    keyword->map_len   = 123;
    keyword->map       = calloc(keyword->map_len, sizeof(char16_t**)); if (!keyword->map)       goto map_fail;
    keyword->color_map = calloc(keyword->map_len, sizeof(uint8_t*));   if (!keyword->color_map) goto color_map_fail;
    keyword->alloc_map = calloc(keyword->map_len, sizeof(uint16_t));   if (!keyword->alloc_map) goto alloc_map_fail;

    keyword_map_add(c_keyword, 3);
    keyword_map_add(c_variable, 2);

    return;

alloc_map_fail:
            free(keyword->color_map);
color_map_fail:
            free(keyword->map);
map_fail:
            fatal_error(u"memory allocation failed");
            return;
}

void keyword_map_destruct() {
    for (uint8_t i = 0; i < keyword->map_len; i++) {

        if (keyword->alloc_map[i]) {
            for (uint16_t j = 0; j < keyword->alloc_map[i]; j++) {
                free(keyword->map[i][j]);
            }
            free(keyword->map[i]);
            free(keyword->color_map[i]);
        }

    }
    free(keyword->map);
    free(keyword->color_map);
    free(keyword->alloc_map);
}

static _Bool cmp_str(char16_t* restrict str, char16_t* restrict compared_to) {
    while (*str && *compared_to)
        if (*str++ != *compared_to++)
            return 0;
    return *compared_to == '\0' && is_word_boundary(*str);
}

_Bool is_keyword(char16_t* restrict word) {
    uint32_t ch = (uint32_t)word[0];
    if (ch >= keyword->map_len)
        return 0;
    char16_t** map   = keyword->map[ch];
    uint16_t map_len = keyword->alloc_map[ch];
    
    for (uint16_t i = 0; i < map_len; i++) {
        if (cmp_str(word, map[i])) {
            keyword->last_color = keyword->color_map[ch][i];
            keyword->last_len   = u_strlen(map[i]);
            return 1;
        }
    }

    return 0;
}