#include "../platform_layer.h"

#ifdef demiwindows

#include <commdlg.h>

static Encoding get_encoding(const char16_t* file_path, FILE* file) {
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
    explorer.nMaxFile = MAX_PATH;
    explorer.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;
    explorer.lpstrTitle = u"Select a file to open";

    if (GetOpenFileName(&explorer))
        file_open(editor, file_path);
}

void file_open(Editor* restrict editor, const char16_t* file_path) {
    FILE* raw = _wfopen(file_path, L"rb");
    if (!raw) {  
        error(u"could not open file", u"error");  
        return;
    }
    Encoding encoding = get_encoding(file_path, raw);
    if (encoding != UTF8 && encoding != UTF16LE && encoding != UTF16BE) {
        error(u"unsuported file encoding", u"error");
        return;
    }
    editor->current_file = editor->files_opened;
    editor->files_opened++;
    editor->files = realloc(editor->files, editor->files_opened * sizeof(DemiFile));
    DemiFile* file = &editor->files[editor->current_file];
    memset(file, 0, sizeof(DemiFile));
    file->encoding = encoding;
    uint32_t path_len = u_strlen(file_path);
    file->path = realloc(file->path, (path_len + 1) * sizeof(char16_t));
    memcpy(file->path, file_path, path_len * sizeof(char16_t));
    uint32_t pos = path_len;
    while (file_path[pos - 1] != u'/' && file_path[pos - 1] != u'\\') {
        pos--;
        if (pos-1 == 0)
            break;
    }
    uint32_t name_len = path_len - pos;
    if (name_len > 10)
        name_len = 10;
    file->file_name = realloc(file->file_name, (name_len + 1) * sizeof(char16_t));
    memcpy(file->file_name, file_path + pos, name_len * sizeof(char16_t));
    file->file_name[name_len] = u'\0';

    fseek(raw, 0, SEEK_END);
    long len = ftell(raw);
    rewind(raw);

    char* raw_data = malloc(len + 1);
    char* original = raw_data;
    fread(raw_data, 1, len, raw);
    fclose(raw);
    raw_data[len] = '\0';

    if (encoding == UTF8) {
        if ((uint8_t)raw_data[0] == 0xEF && (uint8_t)raw_data[1] == 0xBB && (uint8_t)raw_data[2] == 0xBF) {
            raw_data += 3;
            len      -= 3;
        }
        uint64_t final_len = MultiByteToWideChar(CP_UTF8, 0, raw_data, -1, NULL, 0);
        char16_t wide_str[final_len+1];
        MultiByteToWideChar(CP_UTF8, 0, raw_data, -1, wide_str, final_len);
        wide_str[final_len] = u'\0';
        buffer_init_str(&file->string, final_len, wide_str);
    }
    else if (encoding == UTF16LE) {
        raw_data += 2;
        len      -= 2;
        // TODO
    }
    else {
        raw_data += 2;
        len      -= 2;
        // TODO
    }
    free(original);
}

void file_close(Editor* restrict editor, uint8_t file_index) {
    DemiFile* file = &editor->files[file_index];
    buffer_destruct(&file->string);
    free(file->path); 
    free(file->file_name); 
    uint32_t file_size = sizeof(DemiFile);
    memset(file, 0, file_size);
    editor->files_opened--;
    if(file_index == editor->files_opened)
        return;
    memcpy(file, &editor->files[editor->files_opened], file_size);
}

void file_save(Editor* restrict editor) {

}

void file_save_as(Editor* restrict editor) {

}

#endif