#include "../platform_layer.h"

#ifdef demiwindows

#include <commdlg.h>

static const uint8_t bom_utf8[3]    = {0xEF, 0xBB, 0xBF};
static const uint8_t bom_utf16le[2] = {0xFF, 0xFE};
static const uint8_t bom_utf16be[2] = {0xFE, 0xFF};

static inline uint16_t swap16(uint16_t x) {return (x << 8) | (x >> 8);}

static Encoding get_encoding(const char16_t* file_path, FILE* file) {
    uint8_t BOM[4] = {0};
    uint64_t len = fread(BOM, 1, 4, file);
    if (len >= 4 && memcmp(BOM, "\x00\x00\xFE\xFF", 4) == 0) return UTF32BE;
    if (len >= 4 && memcmp(BOM, "\xFF\xFE\x00\x00", 4) == 0) return UTF32LE;
    if (len >= 4 && memcmp(BOM, "%PDF", 4) == 0)             return PDF;
    if (len >= 2 && memcmp(BOM, "\xFE\xFF", 2) == 0)         return UTF16BE;
    if (len >= 2 && memcmp(BOM, "\xFF\xFE", 2) == 0)         return UTF16LE;
    if (len >= 3 && memcmp(BOM, "\xEF\xBB\xBF", 3) == 0)     return UTF8BOM;
    return UTF8;
}

static void set_file_path(DemiFile* restrict file, const char16_t* path) {
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
    if (name_len > 10)
        name_len = 10;
    file->file_name = realloc(file->file_name, (name_len + 1) * sizeof(char16_t));
    if (!file->file_name) {
        fatal_error(u"memory allocation failed\nfile.c");
        return;
    }
    memcpy(file->file_name, path + pos, name_len * sizeof(char16_t));
    file->file_name[name_len] = u'\0';
}

void file_open_explorer() {
    char16_t file_path[MAX_PATH] = u"";
    OPENFILENAME explorer = {0};
    explorer.lStructSize = sizeof(explorer);
    explorer.lpstrFilter = u"All Files\0*.*\0";
    explorer.lpstrFile = file_path;
    explorer.nMaxFile = MAX_PATH;
    explorer.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;
    explorer.lpstrTitle = u"Select a file to open";

    if (GetOpenFileName(&explorer))
        file_open(file_path);
}
void file_open(const char16_t* file_path) {
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

    fseek(raw, 0, SEEK_END);
    int64_t len = ftell(raw);
    rewind(raw);

    char* raw_data = malloc(len + 1);
    fread(raw_data, 1, len, raw);
    fclose(raw);

    char16_t* out;
    int64_t strlen = 0;

    if (encoding == UTF8) {
        strlen = MultiByteToWideChar(CP_UTF8, 0, raw_data, len, 0, 0);
        if (strlen <= 0) {
            error(u"could not read file", u"error");
            return;
        }
        out = malloc((strlen + 1) * sizeof(char16_t));
        if (!out) {
            error(u"memory allocation failed\nfile.c", u"error");  
            return;
        }
        MultiByteToWideChar(CP_UTF8, 0, raw_data, len, out, strlen);
        out[strlen] = u'\0';
    }
    else {
        char* cpy = raw_data;
        cpy  += 2;
        len  -= 2; 

        strlen = len / 2;
        out = malloc((strlen + 1) * sizeof(char16_t));
        if (!out) {
            error(u"memory allocation failed\nfile.c", u"error");  
            return;
        }
        memcpy(out, cpy, len);

        if (encoding == UTF16BE)
            for (int64_t i = 0; i < strlen; ++i)
                out[i] = swap16(out[i]);
        out[strlen] = u'\0';
    }
    free(raw_data);

    editor->current_file = editor->files_opened;
    editor->files_opened++;
    editor->files = realloc(editor->files, editor->files_opened * sizeof(DemiFile));
    DemiFile* current = &editor->files[editor->current_file];
    memset(current, 0, sizeof(DemiFile));
    memset(&current->string, 0, sizeof(StringBuffer));
    current->encoding = encoding;

    buffer_init_str(&current->string, strlen, out);
    free(out);

    set_file_path(current, file_path);
    render_content_projection(editor->width, editor->height, &editor->files[editor->current_file]);
}

void file_close(uint8_t file_index) {
    DemiFile* file = &editor->files[file_index];
    buffer_destruct(&file->string);
    free(file->path); 
    free(file->file_name); 
    uint32_t file_size = sizeof(DemiFile);
    memset(file, 0, sizeof(DemiFile));
    editor->files_opened--;
    if (editor->current_file == editor->files_opened)
        editor->current_file--;
    if(file_index == editor->files_opened)
        return;
    memcpy(file, &editor->files[editor->files_opened], file_size);
}

void file_save(uint8_t saved_file) {
    if (editor->files_opened == 0)
        return;
    DemiFile* current = &editor->files[saved_file];
    if (!current->path) {
        file_save_as();
        return;
    }
    FILE* file = _wfopen(current->path, u"wb");
    if (!file) {
        error(u"could not save file", u"error");
        return;
    }
    switch (current->encoding) {
        case UTF8:
            fwrite(current->string.buffer, sizeof(char16_t), current->string.length, file);
        break;
        case UTF8BOM:
            fwrite(bom_utf8, 1, 3, file);
            fwrite(current->string.buffer, sizeof(char16_t), current->string.length, file);
        break;
        case UTF16LE:
            fwrite(bom_utf16le, 1, 2, file);
            fwrite(current->string.buffer, sizeof(char16_t), current->string.length, file);
        break;
        case UTF16BE:
            fwrite(bom_utf16be, 1, 2, file);
            for (uint64_t i = 0; i < current->string.length; i++) {
                char16_t x = current->string.buffer[i];
                x = (x >> 8) | (x << 8);
                fwrite(&x, sizeof(char16_t), 1, file);
            }
        break;
        default:
        break;
    }
    fclose(file);
}

void file_save_as() {
    if (editor->files_opened == 0)
        return;
    char16_t file_path[MAX_PATH] = u"";
    OPENFILENAME explorer = {0};
    explorer.lStructSize = sizeof(explorer);
    explorer.lpstrFilter =
        L"UTF-8\0*.*\0"
        L"UTF-16 LE\0*.*\0"
        L"UTF-16 BE\0*.*\0";
    explorer.lpstrFile = file_path;
    explorer.nMaxFile = MAX_PATH;
    explorer.Flags = OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST;
    explorer.lpstrTitle = u"Save File As";
    if (GetSaveFileName(&explorer)) {
        DemiFile* current = &editor->files[editor->current_file];
        set_file_path(current, file_path);
        switch (explorer.nFilterIndex) {
            case 2:
                current->encoding = UTF16LE;
            break;
            case 3:
                current->encoding = UTF16BE;
            break;
            default:
                current->encoding = UTF8;
            break;
        }
        file_save(editor->current_file);
    }
}

void file_save_all() {
    if (editor->files_opened == 0)
        return;
    for (uint8_t i = 0; i < editor->files_opened; i++)
        file_save(i);
}

#endif