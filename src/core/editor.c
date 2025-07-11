#include "editor.h"

void editor_init(Editor* restrict editor) {
    glGetIntegerv(GL_MAX_VERTEX_UNIFORM_COMPONENTS, &editor->uniform_limit);

    editor->dpi_scale = (float)editor->dpi / 96.0f;

    editor->scroll_speed = 1.1;

    editor->files_opened = 1;
    editor->files = malloc(sizeof(DemiFile));
    if (!editor->files) {
        fatal_error(u"memory allocation failed");
        return;
    }
    DemiFile* file = &editor->files[0];
    memset(file, 0, sizeof(DemiFile));
    char16_t* unnamed = u"unnamed";
    uint8_t len = u_strlen(unnamed);
    file->file_name = malloc((len + 1) * sizeof(char16_t));
    file->file_name[len] = u'\0';
    memcpy(file->file_name, unnamed, len * sizeof(char16_t));
    buffer_init(&file->string);
}

void editor_resize(Editor* restrict editor) {
    glViewport(0, 0, editor->width, editor->height);
    render_gui_projection(editor->width, editor->height);
    render_content_projection(editor->width, editor->height, &editor->files[editor->current_file]);
}

void editor_dpi_change(Editor* restrict editor, DemiFont* restrict font, uint32_t dpi) {
    editor->dpi       = dpi;
    editor->dpi_scale = (float)editor->dpi / 96.0f;
    font_rebuild(font, editor, 15, editor->dpi_scale); // TODO replace magic number with setting
}

void editor_destruct(Editor* restrict editor) {
    for (uint8_t i = 0; i < editor->files_opened; i++) {
        buffer_destruct(&editor->files[i].string);
        free(editor->files[i].file_name);
        free(editor->files[i].path);
    }
    free(editor->files);
}

void editor_backspace(Editor* restrict editor) {
    buffer_rem_char(&editor->files[editor->current_file].string);
    editor->flags |= 0b100000;
}

void editor_tab(Editor* restrict editor) {
    buffer_add_string(&editor->files[editor->current_file].string, 4, u"    ");
    editor->flags |= 0b100000;
}

void editor_enter(Editor* restrict editor) {
    buffer_add_char(&editor->files[editor->current_file].string, u'\n');
    editor->flags |= 0b100000;
}

void editor_input(Editor* restrict editor, char16_t ch) {
    buffer_add_char(&editor->files[editor->current_file].string, ch);
    editor->flags |= 0b100000;
}

void editor_paste(Editor* restrict editor, char16_t* str) {
    buffer_add_string(&editor->files[editor->current_file].string, u_strlen(str), str);
    editor->flags |= 0b100000;
}

void editor_mouse_wheel(Editor* restrict editor, int32_t delta) {
    delta *= editor->scroll_speed;
    DemiFile* file = &editor->files[editor->current_file];
    file->camera_y += delta;
    if (file->camera_y > 0)
        file->camera_y = 0;
    render_content_projection(editor->width, editor->height, file);
}

void editor_left_click(Editor* restrict editor, DemiFont* restrict font, float x, float y) {
    y = editor->height - y;
    if (y > editor->height - editor->gui.size.y) {
        int32_t current_x = 0;
        int32_t size_x    = editor->gui.size.x;
        for (uint8_t i = 0; i < editor->files_opened; i++) {
            if (x > current_x && x < current_x + size_x) {
                if (x > current_x + editor->gui.size.x - font->size
                    && y > editor->height - font->size - (editor->gui.size.y >> 2)
                    && y < editor->height - (editor->gui.size.y >> 2)) {
                    file_close(editor, i);
                }
                editor->current_file = i;
                render_content_projection(editor->width, editor->height, &editor->files[i]);
                return;
            }
            current_x += size_x;
        }
    }
}

void editor_up(Editor* restrict editor) {
    StringBuffer* string = &editor->files[editor->current_file].string;
    if (string->position == 0)
        return;
    _Bool repeat = 1;
    uint64_t pos = 0;

reach_nl:
    while (string->position > 0 && string->buffer[string->position - 1] != u'\n') {
        string->position--;
        if (repeat == 1)
            pos++;
    }
    if (repeat == 1 && string->position > 0) {
        string->position--;
        repeat = 0;
        goto reach_nl;
    }
    while (pos != 0 && string->buffer[string->position] != u'\n') {
        string->position++;
        pos--;
    }
    editor->flags |= 0b100000;
}

void editor_left(Editor* restrict editor) {
    StringBuffer* string = &editor->files[editor->current_file].string;
    if (string->position > 0)
        string->position--;
    editor->flags |= 0b100000;
}

void editor_right(Editor* restrict editor) {
    StringBuffer* string = &editor->files[editor->current_file].string;
    if (string->position < string->length)
        string->position++;
    editor->flags |= 0b100000;
}

void editor_down(Editor* restrict editor) {
    StringBuffer* string = &editor->files[editor->current_file].string;
    uint64_t pos = 0;
    if (string->position == string->length) {
        return;
    }
    while (string->position > 0 && string->buffer[string->position - 1] != u'\n') {
        string->position--;
        pos++;
    }
    while (string->position < string->length && string->buffer[string->position] != u'\n')
        string->position++;  
    string->position++;
    while (pos != 0 && string->position < string->length && string->buffer[string->position] != u'\n') {
        string->position++;
        pos--;
    }   
    editor->flags |= 0b100000;
}

void editor_camera_to_cursor(Editor* restrict editor, float x, float y, float advance, float nl_height, float min_x) {
    DemiFile* file = &editor->files[editor->current_file];
    while (y < file->camera_y)
        file->camera_y -= nl_height;
    while (y > file->camera_y + editor->height - editor->gui.size.y - nl_height)
        file->camera_y += nl_height;
    while (x - file->camera_x > editor->width - advance)
        file->camera_x += advance;
    while (x < file->camera_x)
        file->camera_x -= advance;
    if (file->camera_y > 0)
        file->camera_y = 0;
    if (file->camera_x < min_x + advance)
        file->camera_x = 0;
    render_content_projection(editor->width, editor->height, file);
}