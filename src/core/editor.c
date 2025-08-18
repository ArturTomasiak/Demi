#include "editor.h"

static inline void unselect() {
    editor->selected[0] = 0;
    editor->selected[1] = 0;
}

void editor_init() {
    editor->flags |= FLAGS_RUNNING;
    glGetIntegerv(GL_MAX_VERTEX_UNIFORM_COMPONENTS, &editor->uniform_limit);
    editor->dpi_scale    = (float)editor->dpi / 96.0f;
    editor->scroll_speed = 1.1;

    unselect();
    editor_update_tab(4);
    file_create_empty();
}

void editor_resize() {
    glViewport(0, 0, editor->width, editor->height);
    render_gui_projection(editor->width, editor->height);
    render_content_projection(editor->width, editor->height, &editor->files[editor->current_file]);
}

void editor_dpi_change(uint32_t dpi) {
    editor->dpi       = dpi;
    editor->dpi_scale = (float)editor->dpi / 96.0f;
}

void editor_destruct() {
    if (!editor->files)
        return;
    for (uint8_t i = 0; i < editor->files_opened; i++) {
        DemiFile* file = &editor->files[i];
        buffer_destruct(&file->string);
        if (file->file_name)
            free(file->file_name);
        if (file->path)
            free(file->path);
    }
    free(editor->files);
    free(editor->tab);
}

void editor_backspace() {
    DemiFile* file = &editor->files[editor->current_file];
    if (editor->selected[0] || editor->selected[1]) {
        editor_delete_selected(&file->string);
        unselect();
        return;
    }
    unselect();
    buffer_rem_char(&file->string, 0);
    editor->flags |= FLAGS_ADJUST_CAMERA_TO_CURSOR;
}

extern inline void editor_tab() {
    editor_paste(editor->tab);
}

void editor_enter() {
    DemiFile* file = &editor->files[editor->current_file];
    if (editor->selected[0] || editor->selected[1])
        editor_delete_selected(&file->string);
    unselect();
    buffer_add_char(&file->string, u'\n', 0);
    editor->flags |= FLAGS_ADJUST_CAMERA_TO_CURSOR;
}

void editor_input(char16_t ch) {
    DemiFile* file = &editor->files[editor->current_file];
    if (editor->selected[0] || editor->selected[1])
        editor_delete_selected(&file->string);
    unselect();
    buffer_add_char(&file->string, ch, 0);
    editor->flags |= FLAGS_ADJUST_CAMERA_TO_CURSOR;
    if (ch == u'/' || ch == u'*')
        editor->flags |= FLAGS_SCAN_COMMENTS;
}

void editor_paste(char16_t* str) {
    DemiFile* file = &editor->files[editor->current_file];
    if (editor->selected[0] || editor->selected[1])
        editor_delete_selected(&file->string);
    unselect();
    buffer_add_string(&file->string, u_strlen(str), str, 0);
    editor->flags |= FLAGS_ADJUST_CAMERA_TO_CURSOR;
}

void editor_undo() {
    DemiFile* file = &editor->files[editor->current_file];
    undo_pop(&file->string);
    editor->flags |= FLAGS_ADJUST_CAMERA_TO_CURSOR;

    unselect();
}

void editor_redo() {
    DemiFile* file = &editor->files[editor->current_file];
    redo_pop(&file->string);
    editor->flags |= FLAGS_ADJUST_CAMERA_TO_CURSOR;

    unselect();
}

void editor_jump_top() {
    DemiFile* file = &editor->files[editor->current_file];
    file->string.position = 0;
    editor->flags |= FLAGS_ADJUST_CAMERA_TO_CURSOR;
}

void editor_jump_bottom() {
    DemiFile* file = &editor->files[editor->current_file];
    file->string.position = file->string.length;
    editor->flags |= FLAGS_ADJUST_CAMERA_TO_CURSOR;
}

void editor_mouse_wheel(int32_t delta, _Bool horizontal, void* handle) {
    delta *= editor->scroll_speed;
    DemiFile* file = &editor->files[editor->current_file];
    if (horizontal) {
        delta = delta >> 1;
        uint64_t y = editor->height - platform_mouse_y(handle);
        if (y > editor->height - gui->size.y) {
            gui->camera_x += delta;
            if (gui->camera_x < 0)
                gui->camera_x = 0;
            render_gui_projection(editor->width, editor->height);
            return;
        }
        else {
            file->camera_x += delta;
            if (file->camera_x < 0)
                file->camera_x = 0;
        }
    }
    else {
        file->camera_y += delta;
        if (file->camera_y > 0)
            file->camera_y = 0;
    }
    render_content_projection(editor->width, editor->height, file);
}

void editor_left_click(float x, float y) {
    unselect();
    y = editor->height - y;
    if (y > editor->height - gui->size.y) {
        int32_t current_x = 0;
        int32_t size_x    = gui->size.x;
        for (uint8_t i = 0; i < editor->files_opened; i++) {
            if (x > current_x && x < current_x + size_x) {
                if (x > current_x + gui->size.x - font->size
                    && y > editor->height - font->size - (gui->size.y >> 2)
                    && y < editor->height - (gui->size.y >> 2)) {
                    file_close(i);
                }
                else
                    editor->current_file = i;
                render_content_projection(editor->width, editor->height, &editor->files[i]);
                return;
            }
            current_x += size_x;
        }
    }
    else {

    }
}

void editor_up() {
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
    editor->flags |= FLAGS_ADJUST_CAMERA_TO_CURSOR;

    unselect();
}

void editor_left() {
    StringBuffer* string = &editor->files[editor->current_file].string;
    if (string->position > 0)
        string->position--;
    editor->flags |= FLAGS_ADJUST_CAMERA_TO_CURSOR;

    unselect();
}

void editor_right() {
    StringBuffer* string = &editor->files[editor->current_file].string;
    if (string->position < string->length)
        string->position++;
    editor->flags |= FLAGS_ADJUST_CAMERA_TO_CURSOR;

    unselect();
}

void editor_down() {
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
    editor->flags |= FLAGS_ADJUST_CAMERA_TO_CURSOR;

    unselect();
}

void editor_select_all() {
    StringBuffer* string = &editor->files[editor->current_file].string;
    editor->selected[0] = 0;
    editor->selected[1] = string->length;
    string->position = editor->selected[1];
}

void editor_key_select(_Bool right) {
    StringBuffer* string = &editor->files[editor->current_file].string;
    if (editor->selected[0] == 0 && editor->selected[1] == 0) {
        editor->selected[0] = string->position;
        editor->selected[1] = string->position;
    }
    if (right && editor->selected[1] < string->length)
        editor->selected[1]++;
    else if (editor->selected[0] > 0)
        editor->selected[0]--;

    string->position = editor->selected[1];
    editor->flags |= FLAGS_ADJUST_CAMERA_TO_CURSOR;
}

extern inline void editor_delete_selected(StringBuffer* string) {
    buffer_rem_len(string, editor->selected[1] - editor->selected[0], 0);
}

void editor_update_tab(uint8_t len) {
    editor->tab_len = len;
    editor->tab     = realloc(editor->tab, (len + 1) * sizeof(char16_t));
    for (uint8_t i = 0; i < len; i++)
        editor->tab[i] = u' ';
    editor->tab[len] = u'\0';
}

void editor_camera_to_cursor(float x, float y, float advance, float nl_height, float min_x) {
    DemiFile* file = &editor->files[editor->current_file];
    while (y < file->camera_y)
        file->camera_y -= nl_height;
    while (y > file->camera_y + editor->height - gui->size.y - nl_height)
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