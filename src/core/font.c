#include "font.h"

void font_init(DemiFont* restrict font, int32_t uniform_limit, float dpi_scale, _Bool gl46) {
    int32_t texture_arr_limit;
    glGetIntegerv(GL_MAX_ARRAY_TEXTURE_LAYERS, &texture_arr_limit);
    font->line_spacing = 1.2;

    if (texture_arr_limit < 512) {
        fatal_error(u"gpu currently unsupported\n GL_MAX_ARRAY_TEXTURE_LAYERS < 512");
        return;
    }
    font->range[0][0] = 0;
    font->range[0][1] = 383;
    font->range[1][0] = 384;
    font->range[1][1] = 591;
    font->range[2][0] = 688;
    font->range[2][1] = 1023;
    font->range[3][0] = 1024;
    font->range[3][1] = 1423;
    font->range[4][0] = 3584;
    font->range[4][1] = 3711;
    font->range[5][0] = 4256;
    font->range[5][1] = 4351;
    font->range[6][0] = 7680;
    font->range[6][1] = 7935;
    font->range[7][0] = 8192;
    font->range[7][1] = 8527;
    font->range[8][0] = 8528;
    font->range[8][1] = 8959;
    font->texture_count = 9; // text_frag needs to be updated manually

    font->character = calloc(font->range[font->texture_count-1][1] + 1, sizeof(Character));
    if (!font->character) {
        fatal_error(u"memory allocation failed");
        return;
    }

    font->arr_limit = uniform_limit >> 4;   // because mat4 is 16 bytes
    font->arr_limit = font->arr_limit >> 2; // TODO actually count the needed bytes
    if (font->arr_limit < 10)               // arbitraty impossible scenario
        font->arr_limit = 10;

    font->transforms  = calloc(font->arr_limit * 16, sizeof(float));
    font->letter_map  = calloc(font->arr_limit, sizeof(int32_t));
    font->texture_map = calloc(font->arr_limit, sizeof(int32_t));

    if (FT_Init_FreeType(&font->ft_lib)) {
        fatal_error(u"FreeType initialization failed");
        return;
    }
    if (FT_New_Face(font->ft_lib, "../resources/fonts/Hack-regular.ttf", 0, &font->ft_face)) {
        fatal_error(u"failed to create face using resources/fonts/Hack-regular.ttf");
        return;
    }

    shader_init(&font->shader, "../resources/shaders/text_vert.glsl", "../resources/shaders/text_frag.glsl", font->arr_limit, gl46);

    vao_init(&font->vao);
    const float vertex[8] = {
        0.0f, 1.0f,
        0.0f, 0.0f,
        1.0f, 1.0f,
        1.0f, 0.0f
    };
    vbo_init(&font->vbo, vertex, 4 * sizeof(vertex));
    VertexBufferLayout layout = {0};
    vao_add_element(&layout, 2, GL_FLOAT, sizeof(float), 0);
    vao_add_buffer(&layout, &font->vao);
    vao_layout_destruct(&layout);

    font_rebuild(font, 18, dpi_scale);
}

void font_rebuild(DemiFont* restrict font, uint16_t new_font_size, float dpi_scale) {
    font->size = new_font_size * dpi_scale;

    if (font->texture[0]) {
        glDeleteTextures(font->texture_count, font->texture);
        memset(&font->texture, 0, sizeof(uint32_t) * font->texture_count);
    }

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    FT_Set_Pixel_Sizes(font->ft_face, font->size, font->size);

    FT_GlyphSlot glyph = font->ft_face->glyph;

    shader_bind(&font->shader);
    vao_bind(&font->vao);
    vbo_bind(&font->vbo);

    for (uint8_t i = 0; i < font->texture_count; i++) {
        glGenTextures(1, &font->texture[i]);
        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(GL_TEXTURE_2D_ARRAY, font->texture[i]);
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        uint16_t processed_chars = font->range[i][1] - font->range[i][0] + 1;

        uint8_t *empty = calloc(font->size * font->size * processed_chars, sizeof(uint8_t));
        if (!empty) {
            fatal_error(u"memory allocation failed");
            return;
        }

        glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_R8, font->size, font->size, processed_chars, 0, GL_RED, GL_UNSIGNED_BYTE, empty);
        free(empty);

        char name[11] = "textures[0]";
        name[9] = i + '0';
        shader_uniform_int32_t(&font->shader, name, i);

        for (uint32_t c = font->range[i][0]; c <= font->range[i][1]; c++) {
            Character* ch = &font->character[c];
            if (FT_Load_Char(font->ft_face, c, FT_LOAD_RENDER)) {
                #ifdef demidebug
                printf("%s %c\n", "failed to load glyph", (char)c);
                #endif
                continue;
            }
            glTexSubImage3D(
                GL_TEXTURE_2D_ARRAY,
                0, 0, 0, c - font->range[i][0],
                glyph->bitmap.width,
                glyph->bitmap.rows, 1,
                GL_RED,
                GL_UNSIGNED_BYTE,
                glyph->bitmap.buffer
            );
            ch->size.x    = glyph->bitmap.width;
            ch->size.y    = glyph->bitmap.rows;
            ch->bearing.x = glyph->bitmap_left;
            ch->bearing.y = glyph->bitmap_top;
            ch->advance   = glyph->advance.x >> 6;
            ch->processed = 1;
        }
    }
}

void font_destruct(DemiFont* restrict font) {
    if (font->texture[0]) {
        for (int i = 0; i < font->texture_count; i++) {
            glDeleteTextures(1, &font->texture[i]);
            font->texture[i] = 0;
        }
    } 
    shader_destruct(&font->shader);
    vao_destruct(&font->vao);
    vbo_destruct(&font->vbo);
    free(font->character);
    free(font->transforms);
    free(font->letter_map);
    free(font->texture_map);
}