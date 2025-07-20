#include "color_map.h"

static ColorMapData* data;

void color_map_init() {
    data = calloc(1, sizeof(ColorMapData));
    if (!data) {
        fatal_error(u"memory allocation failed");
        return;
    }
    const char16_t* keyword[18] = {u"if", u"while", u"for", u"switch", u"else", u"return", u"#include", 
                                u"#pragma", u"#ifdef", u"#if", u"#endif", u"#elif", u"#define", u"#error",
                                u"static", u"extern", u"typedef", u"const"};
    data->keyword_len = 18;
    data->keyword = malloc(data->keyword_len * sizeof(char16_t*));
    if (!data->keyword) {
        data->keyword_len = 0;
        fatal_error(u"memory allocation failed");
        return;
    }
    for (uint16_t i = 0; i < data->keyword_len; i++) {
        uint16_t len = u_strlen(keyword[i]);
        data->keyword[i] = malloc((len + 1) * sizeof(char16_t));
        if (!data->keyword[i]) {
            data->keyword_len = i;
            fatal_error(u"memory allocation failed");
            return;
        }
        memcpy(data->keyword[i], keyword[i], len * sizeof(char16_t));
        data->keyword[i][len] = u'\0';
    }
    const char16_t* variable[19] = {u"void", u"bool", u"_Bool", u"char", u"short", u"int", u"long", u"signed", u"unsigned", u"float", u"double",
                                u"int8_t", u"int16_t", u"int32_t", u"int64_t", u"uint8_t", u"uint16_t", u"uint32_t", u"uint64_t"};
    data->variable_len = 19;
    data->variable = malloc(data->variable_len * sizeof(char16_t*));
    if (!data->variable) {
        data->variable_len = 0;
        fatal_error(u"memory allocation failed");
        return;
    }
    for (uint16_t i = 0; i < data->variable_len; i++) {
        uint16_t len = u_strlen(variable[i]);
        data->variable[i] = malloc((len + 1) * sizeof(char16_t));
        if (!data->variable[i]) {
            data->variable_len = i;
            fatal_error(u"memory allocation failed");
            return;
        }
        memcpy(data->variable[i], variable[i], (len + 1) * sizeof(char16_t));
        data->variable[i][len] = u'\0';
    }
}

static _Bool is_word_boundary(char16_t ch) {
    return ch == u' ' || ch == u'\t' || ch == u'\n' || ch == u'\r' || 
           ch == u'(' || ch == u')' || ch == u'{' || ch == u'}' || 
           ch == u'[' || ch == u']' || ch == u';' || ch == u',' ||
           ch == u'.' || ch == u':' || ch == u'*' || ch == u'\0';
}

static _Bool cmp_str(char16_t* str, char16_t* compared_to) {
    while (*str && *compared_to)
        if (*str++ != *compared_to++)
            return 0;
    return *compared_to == '\0' && is_word_boundary(*str);
}

void color_map_update(StringBuffer* restrict string) {
    uint64_t i = 0;
    uint64_t end = string->length; 

    while (i < end) {
        string->color_map[i] = 0;

        if (i < end && string->buffer[i] == u'/' && string->buffer[i + 1] == u'/') {
            while (i < end && string->buffer[i + 1] != u'\n')
                string->color_map[i++] = 1;
            goto end;
        }

        if (i != 0 && string->color_map[i - 1] != 3 && !is_word_boundary(string->buffer[i - 1]))
            goto end;

        for (uint8_t j = 0; j < data->keyword_len; j++) {
            char16_t* str = data->keyword[j];
            if (cmp_str(string->buffer + i, str)) {
                while (*str++)
                    string->color_map[i++] = 3;
                i--;
                goto end;
            }
        }

        for (uint8_t j = 0; j < data->variable_len; j++) {
            char16_t* str = data->variable[j];
            if (cmp_str(string->buffer + i, str)) {
                while (*str++)
                    string->color_map[i++] = 2;
                i--;
                goto end;
            }
        }

        for (uint8_t j = 0; j < data->name_len; j++) {
            char16_t* str = data->name[j];
            if (cmp_str(string->buffer + i, str)) {
                while (*str++)
                    string->color_map[i++] = 2;
                i--;
                goto end;
            }
        }

end:
        i++;
    }
}

void color_map_destruct() {
    for (uint16_t i = 0; i < data->keyword_len; i++)
        free(data->keyword[i]);
    free(data->keyword);
    for (uint16_t i = 0; i < data->variable_len; i++)
        free(data->variable[i]);
    free(data->variable);
    free(data);    
}
