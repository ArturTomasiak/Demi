#include "../platform_layer.h"

#ifdef demiwindows

#include <commdlg.h>

static Encoding get_encoding(const char16_t* file_path) {
    FILE* file = _wfopen(file_path, u"rb");
    int32_t BOM[4] = {0};
    fread(BOM, 1, 4, file);
    if (BOM[0] == 0x00 && BOM[1] == 0x00 && BOM[2] == 0xFE && BOM[3] == 0xFF)
        return UTF32BE;
    else if (BOM[0] == 0xFF && BOM[1] == 0xFE && BOM[2] == 0x00 && BOM[3] == 0x00)
        return UTF32LE;
    else if (BOM[0] == 0xFE && BOM[1] == 0xFF)
        return UTF16BE;
    else if (BOM[0] == 0xFF && BOM[1] == 0xFE)
        return UTF16LE;
    else if (BOM[0] == '%' && BOM[1] == 'P' && BOM[2] == 'D' && BOM[3] == 'F')
        return PDF;
    else
        return UTF8;
}

void file_open_explorer(Editor* restrict editor) {
    char16_t file_path[MAX_PATH] = u"";
    OPENFILENAME explorer = {0};
    explorer.lStructSize = sizeof(explorer);
    explorer.lpstrFilter = u"All Files\0*.*\0";
    explorer.lpstrFile = file_path;
    explorer.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;
    explorer.lpstrTitle = u"Select a file to open";

    if (GetOpenFileName(&explorer))
        file_open(editor, file_path);
}

void file_open(Editor* restrict editor, const char16_t* file_path) {
    editor->current_file = editor->files_opened;
    editor->files_opened++;
    editor->files = realloc(editor->files, editor->files_opened * sizeof(DemiFile));
    DemiFile* file = &editor->files[editor->current_file];
    uint32_t path_len = u_strlen(file_path);
    file->path = realloc(file->path, (path_len + 1) * sizeof(char16_t));
    memcpy(file->path, file_path, path_len * sizeof(char16_t));
    file->encoding = get_encoding(file_path);
    uint32_t pos = path_len;
    while (file_path[pos] != u'/' && file_path[pos] != u'\\') {
        pos--;
        if (pos == 0)
            break;
    }
    uint32_t name_len = path_len - pos;
    file->file_name = realloc(file->file_name, (name_len + 1) * sizeof(char16_t));
    memcpy(file->file_name, file_path + pos, name_len * sizeof(char16_t));
}

void file_close(Editor* restrict editor) {

}

void file_save(Editor* restrict editor) {

}

void file_save_as(Editor* restrict editor) {

}

#endif