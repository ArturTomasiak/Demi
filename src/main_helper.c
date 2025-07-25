#include "include.h"
#include "core/string.h"

extern inline void setup_lines(Editor* restrict editor) {
    StringBuffer* current = &editor->files[editor->current_file].string;
    uint32_t pos_since_nl = 0;
    uint64_t lines = 1;
    uint64_t alloc = 2;
    editor->data.current_line = 0;
    editor->data.since_nl     = 0;
    for (uint64_t i = 0; i <= current->length; i++) {
        if (i == current->position) {
            editor->data.current_line = lines;
            editor->data.since_nl     = pos_since_nl;
        }
        pos_since_nl++;
        if (current->buffer[i] == u'\n') {
            lines++;
            alloc += num_count(lines) + 1;
            pos_since_nl = 0;
        }
    }
    RenderData* data = &editor->data;
    data->lines     = realloc(data->lines, (alloc + 1) * sizeof(char16_t));
    data->color_map = realloc(data->color_map, alloc * sizeof(int32_t));
    for (uint64_t i = 0; i < alloc; i++)
        data->color_map[i] = 1;
    uint64_t index = 0;
    uint16_t temp_len;
    for (uint64_t i = 0; i < lines; i++) {
        char16_t* temp = num_to_ustr(i + 1, &temp_len);
        memcpy(data->lines + index, temp, temp_len * sizeof(char16_t));
        index += temp_len;
        free(temp);
    }
    data->lines_len = alloc;
    data->last_line = lines;
}