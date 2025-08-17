#include "../include.h"
#include "render.h"

void file_create_empty();
void file_create(char16_t* content, int64_t len, const char16_t* file_path, Encoding encoding);
void set_file_path(DemiFile* restrict file, const char16_t* path);