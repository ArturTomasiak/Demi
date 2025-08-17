#include "demifile.h"

void file_create_empty() {
    editor->current_file = editor->files_opened;
    editor->files_opened++;
    editor->files = realloc(editor->files, editor->files_opened * sizeof(DemiFile));
    if (!editor->files) {
        fatal_error(u"memory allocation failed\neditor.c");
        return;
    }

    DemiFile* file = &editor->files[editor->files_opened - 1];
    memset(file, 0, sizeof(DemiFile));
    memset(&file->string, 0, sizeof(StringBuffer));
    char16_t* unnamed = u"unnamed";
    uint8_t len = u_strlen(unnamed);
    file->file_name = malloc((len + 1) * sizeof(char16_t));
    file->file_name[len] = u'\0';
    memcpy(file->file_name, unnamed, len * sizeof(char16_t));
    buffer_init(&file->string);

    render_content_projection(editor->width, editor->height, file);
}

void file_create(char16_t* content, int64_t len, const char16_t* file_path, Encoding encoding) {
    editor->current_file = editor->files_opened;
    editor->files_opened++;
    editor->files = realloc(editor->files, editor->files_opened * sizeof(DemiFile));
    DemiFile* file = &editor->files[editor->current_file];
    memset(file, 0, sizeof(DemiFile));
    memset(&file->string, 0, sizeof(StringBuffer));
    file->encoding = encoding;

    buffer_init_str(&file->string, len, content);
    free(content);

    set_file_path(file, file_path);
    render_content_projection(editor->width, editor->height, file);
}

void set_file_path(DemiFile* restrict file, const char16_t* path) {
    uint32_t path_len = u_strlen(path);
    file->path = realloc(file->path, (path_len + 1) * sizeof(char16_t));
    if (!file->path) {
        fatal_error(u"memory allocation failed\nfile.c");
        return;
    }
    memcpy(file->path, path, path_len * sizeof(char16_t));
    file->path[path_len] = u'\0';
    uint32_t pos = path_len;
    while (path[pos - 1] != u'/' && path[pos - 1] != u'\\') {
        pos--;
        if (pos - 1 == 0)
            break;
    }
    uint32_t name_len = path_len - pos;
    if (name_len > 12)
        name_len = 12;
    file->file_name = realloc(file->file_name, (name_len + 1) * sizeof(char16_t));
    if (!file->file_name) {
        fatal_error(u"memory allocation failed\nfile.c");
        return;
    }
    memcpy(file->file_name, path + pos, name_len * sizeof(char16_t));
    file->file_name[name_len] = u'\0';
}