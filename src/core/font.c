#include "font.h"

void font_init(int32_t uniform_limit, _Bool gl46) {
    int32_t texture_arr_limit;
    glGetIntegerv(GL_MAX_ARRAY_TEXTURE_LAYERS, &texture_arr_limit);
    font->line_spacing = 1.3;

    if (texture_arr_limit < 256) {
        fatal_error(u"gpu currently unsupported\n GL_MAX_ARRAY_TEXTURE_LAYERS < 256");
        return;
    }
    else if (texture_arr_limit < 2000) {
        font->range[0][0] = 0;
        font->range[0][1] = 255;
        font->range[1][0] = 256;
        font->range[1][1] = 500;
        font->range[2][0] = 501;
        font->range[2][1] = 539;
        font->range[3][0] = 900;
        font->range[3][1] = 1150;
        font->range[4][0] = 1151;
        font->range[4][1] = 1300;
        font->range[5][0] = 1301;
        font->range[5][1] = 1418;
        font->range[6][0] = 4300;
        font->range[6][1] = 4351;
        font->range[7][0] = 8194;
        font->range[7][1] = 8300;
        font->range[7][0] = 8301;
        font->range[7][1] = 8487;
        font->texture_count = 9;  // adjust DemiFont struct when changed
    }
    else {
        font->range[0][0] = 0;
        font->range[0][1] = 539;
        font->range[1][0] = 900;
        font->range[1][1] = 1418;
        font->range[2][0] = 4300;
        font->range[2][1] = 4351;
        font->range[3][0] = 8194;
        font->range[3][1] = 8487;
        font->texture_count = 4;
    }

    font->character = calloc(font->range[font->texture_count - 1][1] + 1, sizeof(Character));
    if (!font->character) {
        fatal_error(u"memory allocation failed\nfont.c");
        return;
    }

    font->arr_limit =  uniform_limit >> 4; // mat4 is 16 bytes
    font->arr_limit -= 20;                 // text_vert's bytes used except for arr

    font->transforms  = calloc(font->arr_limit * 16, sizeof(float));
    font->letter_map  = calloc(font->arr_limit, sizeof(int32_t));
    font->texture_map = calloc(font->arr_limit, sizeof(int32_t));

    if (FT_Init_FreeType(&font->ft_lib)) {
        fatal_error(u"FreeType initialization failed");
        return;
    }
    if (FT_Library_SetLcdFilter(font->ft_lib, FT_LCD_FILTER_LIGHT)) {
        #ifdef demidebug
        printf("FT_LCD_FILTER_LIGHT failed\n");
        #endif
        if (FT_Library_SetLcdFilter(font->ft_lib, FT_LCD_FILTER_DEFAULT)) {
            #ifdef demidebug
            printf("FT_LCD_FILTER_DEFAULT failed\n");
            #endif
        }
    }
    if (FT_New_Face(font->ft_lib, "../resources/fonts/Hack-Regular.ttf", 0, &font->ft_face)) {
        fatal_error(u"failed to create face using /resources/fonts/Hack-Regular.ttf");
        return;
    }
    if (FT_Select_Charmap(font->ft_face, FT_ENCODING_UNICODE)) {
        #ifdef demidebug
        printf("%s\n", "FT_Select_Charmap failed\n");
        #endif
    }

    shader_init(&font->shader, "../resources/shaders/text_vert.glsl", "../resources/shaders/text_frag.glsl", font->texture_count, font->arr_limit, gl46);
    shader_bind(&font->shader);

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

    shader_uniform_float_3(&font->shader, "color[0]", 1.0f, 1.0f, 1.0f);
    shader_uniform_float_3(&font->shader, "color[1]", gray[0], gray[1], gray[2]);         // comments
    shader_uniform_float_3(&font->shader, "color[2]", orange[0], orange[1], orange[2]);   // variables
    shader_uniform_float_3(&font->shader, "color[3]", violet[0], violet[1], violet[2]);   // keywords

    glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &font->max_anisotropy);
}

void font_rebuild(uint16_t new_font_size, float dpi_scale) {
    font->size = new_font_size * dpi_scale;

    if (font->texture[0]) {
        glDeleteTextures(font->texture_count, font->texture);
        memset(&font->texture, 0, sizeof(uint32_t) * font->texture_count);
    }

    FT_Set_Pixel_Sizes(font->ft_face, font->size - 1, font->size - 1);

    FT_GlyphSlot glyph = font->ft_face->glyph;

    shader_bind(&font->shader);
    vao_bind(&font->vao);
    vbo_bind(&font->vbo);

    float gamma = 1.8f + (0.7f * fmin(1.0f, font->size / 24.0f));
    shader_uniform_float(&font->shader, "gamma", gamma);
    shader_uniform_float(&font->shader, "font_size", font->size);

    float border_color[4] = {0};

    for (uint8_t i = 0; i < font->texture_count; i++) {
        glGenTextures(1, &font->texture[i]);
        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(GL_TEXTURE_2D_ARRAY, font->texture[i]);

        glTexParameterfv(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_BORDER_COLOR, border_color);
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        if (font->max_anisotropy > 1.0f) {
            glTexParameterf(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAX_ANISOTROPY_EXT, font->max_anisotropy);
        }

        uint16_t processed_chars = font->range[i][1] - font->range[i][0] + 1;

        uint8_t *empty = calloc(font->size * font->size * processed_chars * 3, sizeof(uint8_t));
        if (!empty) {
            fatal_error(u"memory allocation failed\nfont.c");
            return;
        }

        glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGB8, font->size, font->size, processed_chars, 0, GL_RGB, GL_UNSIGNED_BYTE, empty);
        free(empty);

        char name[11] = "textures[0]";
        name[9] = i + '0';
        shader_uniform_int32_t(&font->shader, name, i);

        for (uint32_t c = font->range[i][0]; c <= font->range[i][1]; c++) {
            Character* ch = &font->character[c];
            if (FT_Load_Char(font->ft_face, c, FT_LOAD_RENDER | FT_LOAD_TARGET_LCD)) {
                #ifdef demidebug
                printf("%s %lc\n", "failed to load glyph", (char16_t)c);
                #endif
                ch->processed = 0;
                continue;
            }
            else if (glyph->bitmap.width == 0 
                || glyph->bitmap.rows == 0 
                || glyph->bitmap.width > font->size * 3
                || glyph->bitmap.rows > font->size) {
                ch->processed = 0;
                #ifdef demidebug
                printf("%s %lc\n", "glyph's size is 0 or exceeds limit: ", (char16_t)c);
                #endif
                continue;
            }

            uint32_t lcd_width = glyph->bitmap.width / 3;
            
            uint8_t* rgb_buffer = malloc(lcd_width * glyph->bitmap.rows * 3);
            if (!rgb_buffer) {
                fatal_error(u"memory allocation failed\nfont.c");
                continue;
            }

            memcpy(rgb_buffer, glyph->bitmap.buffer, glyph->bitmap.width * glyph->bitmap.rows);

            glTexSubImage3D(
                GL_TEXTURE_2D_ARRAY,
                0, 0, 0, c - font->range[i][0],
                lcd_width,
                glyph->bitmap.rows, 1,
                GL_RGB,
                GL_UNSIGNED_BYTE,
                glyph->bitmap.buffer
            );

            free(rgb_buffer);

            ch->size.x    = lcd_width;
            ch->size.y    = glyph->bitmap.rows;
            ch->bearing.x = glyph->bitmap_left;
            ch->bearing.y = glyph->bitmap_top;
            ch->advance   = glyph->advance.x >> 6;
            ch->processed = 1;
        }
    }
    gui_size(font->size);
    editor->flags |= FLAGS_FONT_RESIZED;
}

void font_destruct() {
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