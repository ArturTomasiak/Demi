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
    gui_size(&editor->gui, 100, 30, editor->dpi_scale);
    font_rebuild(font, 15, editor->dpi_scale); // TODO replace magic number with setting
}

void editor_destruct(Editor* restrict editor) {
    for (uint8_t i = 0; i < editor->files_opened; i++)
        buffer_destruct(&editor->files[i].string);
}

void editor_backspace(Editor* restrict editor) {
    buffer_rem_char(&editor->files[editor->current_file].string);
}

void editor_tab(Editor* restrict editor) {
    buffer_add_string(&editor->files[editor->current_file].string, 4, u"    ");
}

void editor_enter(Editor* restrict editor) {
    buffer_add_char(&editor->files[editor->current_file].string, u'\n');
}

void editor_input(Editor* restrict editor, char16_t ch) {
    buffer_add_char(&editor->files[editor->current_file].string, ch);
}

void editor_paste(Editor* restrict editor, char16_t* str) {
    buffer_add_string(&editor->files[editor->current_file].string, u_strlen(str), str);
}

void editor_mouse_wheel(Editor* restrict editor, int32_t delta) {
    delta *= editor->scroll_speed;
    DemiFile* file = &editor->files[editor->current_file];
    file->camera_y += delta;
    if (file->camera_y > 0)
        file->camera_y = 0;
    render_content_projection(editor->width, editor->height, file);
}

_Bool is_printable(const DemiFont* restrict font, char16_t ch) {
    if (ch == u' ')
        return 1;
    int16_t val = (int16_t)ch;
    if (val < 20 || val > font->range[font->texture_count-1][1] || !font->character[ch].processed)
        return 0;
    return 1;
}