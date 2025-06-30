#include "editor.h"

void editor_init(Editor* restrict editor) {
    glGetIntegerv(GL_MAX_VERTEX_UNIFORM_COMPONENTS, &editor->uniform_limit);

    if (editor->dpi < 30) // arbitrary impossible scenario
        editor->dpi = 96;
    editor->dpi_scale = (float)editor->dpi / 96.0f;

    editor->files_opened = 1;
    editor->files = malloc(sizeof(DemiFile));
    memset(&editor->files[0], 0, sizeof(DemiFile));
    if (!editor->files) {
        fatal_error(u"memory allocation failed");
        return;
    }
    buffer_init(&editor->files[editor->current_file].string);
}

void editor_resize(Editor* restrict editor, DemiFont* restrict font) {
    glViewport(0, 0, editor->width, editor->height);
    render_update_projection(editor->width, editor->height, font);
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

_Bool is_printable(const DemiFont* restrict font, char16_t ch) {
    int16_t val = (int16_t)ch;
    if (val < 20 || val > font->range[font->texture_count-1][1] || !font->character[ch].processed)
        return 0;
    return 1;
}