#include "render.h"

static float camera_projection[16] = {0};
static float static_projection[16] = {0};

void render_init(int32_t width, int32_t height, DemiFont* restrict font) {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_MULTISAMPLE);
    render_update_projection(width, height, font);
}

void render_update_projection(int32_t width, int32_t height, DemiFont* restrict font) {
    math_orthographic_f4x4(camera_projection, font->camera_x, width + font->camera_x, font->camera_y, height + font->camera_y, -1.0f, 1.0f);
    math_orthographic_f4x4(static_projection, 0, width, 0, height, -1.0f, 1.0f);
}

void render_text_bind(DemiFont* restrict font, _Bool gui) {
    shader_bind(&font->shader);
    vao_bind(&font->vao);
    vbo_bind(&font->vbo);
    for (int i = 0; i < font->texture_count; i++) {
        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(GL_TEXTURE_2D_ARRAY, font->texture[i]);
    }
    if (gui)
        shader_uniform_mat4(&font->shader, "projection", static_projection, 1);
    else
        shader_uniform_mat4(&font->shader, "projection", camera_projection, 1);
}

static void render_text_call(Shader* restrict shader, float* transforms, int32_t* letter_map, int32_t* texture_map, uint32_t length) {
    shader_uniform_float_3(shader, "text_color", 1.0f, 1.0f, 1.0f);
    shader_uniform_mat4(shader, "transforms", transforms, length);
    shader_uniform_int32_t_arr(shader, "letter_map", letter_map, length);
    shader_uniform_int32_t_arr(shader, "texture_map", texture_map, length);
    glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, length);
}   

void render_text(DemiFont* restrict font, StringBuffer* restrict string, float start_x, float start_y, float dpi_scale) {
    float translate[16] = {0}, scale_matrix[16] = {0};
    float x = start_x;
    float y = start_y;
    uint32_t working_index = 0;
    for (uint64_t i = 0; i < string->length; i++) {
        int32_t ch_int = (int32_t)string->buffer[i];
        Character* character = &font->character[ch_int];
        if (string->buffer[i] == u'\n') {
            y -= font->character[u'\n'].size.y * font->line_spacing;
            x = start_x;
        }
        else if (string->buffer[i] == u' ')
            x += character->advance;
        else if (!character->processed || ch_int > font->range[font->texture_count-1][1])
            continue;
        else {
            float xpos = x + character->bearing.x;
            float ypos = y + character->bearing.y - font->size;
            memset(translate, 0.0f, 16 * sizeof(float));
            memset(scale_matrix, 0.0f, 16 * sizeof(float));
            math_identity_f4x4(scale_matrix, 1.0f);
            
            for (uint8_t texture = 0; texture < font->texture_count; texture++) {
                if (ch_int <= font->range[texture][1]) {
                    font->texture_map[working_index] = texture;
                    font->letter_map[working_index] = ch_int - font->range[texture][0];
                    break;
                }
            }

            math_translate_f4x4(translate, xpos, ypos, 0.9f);
            math_scale_f4x4(scale_matrix, font->size, font->size, 0);
            math_multiply_f4x4(font->transforms + (working_index * 16), translate, scale_matrix);
            x += character->advance;
            working_index++;
            if (working_index == font->arr_limit) {
                render_text_call(&font->shader, font->transforms, font->letter_map, font->texture_map, working_index);
                working_index = 0;
            }
        }
    }
    if (working_index != 0)
        render_text_call(&font->shader, font->transforms, font->letter_map, font->texture_map, working_index);;
}