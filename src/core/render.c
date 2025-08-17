#include "render.h"

static float content_projection[16] = {0};
static float gui_projection[16] = {0};
static float nl_height;

void render_init(int32_t width, int32_t height, DemiFile* restrict file) {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    render_gui_projection(width, height);
    render_content_projection(width, height, file);
}

void render_update_nl_height(float new_height) {
    nl_height = new_height;
}

extern inline void render_gui_projection(int32_t width, int32_t height) {
    math_orthographic_f4x4(gui_projection, gui->camera_x, gui->camera_x + width, 0, height, -1.0f, 1.0f);
}

extern inline void render_content_projection(int32_t width, int32_t height, DemiFile* restrict file) {
    math_orthographic_f4x4(content_projection, file->camera_x, file->camera_x + width, file->camera_y, file->camera_y + height, -1.0f, 1.0f);
}

void render_text_bind(_Bool gui) {
    shader_bind(&font->shader);
    vao_bind(&font->vao);
    vbo_bind(&font->vbo);
    for (int i = 0; i < font->texture_count; i++) {
        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(GL_TEXTURE_2D_ARRAY, font->texture[i]);
    }
    if (gui)
        shader_uniform_mat4(&font->shader, "projection", gui_projection, 1);
    else
        shader_uniform_mat4(&font->shader, "projection", content_projection, 1);
}

static void render_text_call(Shader* restrict shader, float* transforms, int32_t* letter_map, int32_t* restrict color_map, int32_t* texture_map, uint32_t length) {
    shader_uniform_mat4(shader, "transforms", transforms, length);
    shader_uniform_int32_t_arr(shader, "letter_map", letter_map, length);
    shader_uniform_int32_t_arr(shader, "texture_map", texture_map, length);
    shader_uniform_int32_t_arr(shader, "color_map", color_map, length);

    glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, length);
}   

void render_text(char16_t* restrict buffer, int32_t* restrict color_map, uint64_t len, float start_x, float start_y, _Bool gui_text) {
    DemiFile* file;
    float y_limit[2];
    if (!gui_text) {
        file = &editor->files[editor->current_file];
        y_limit[0] = file->camera_y + editor->height - (gui->size.y + gui_margin_y) - font->size;
        y_limit[1] = file->camera_y - font->size;
    }

    int32_t advance   = font->character['a'].advance; // safe with monospaced fonts
    float translate[16] = {0}, scale_matrix[16] = {0};
    int32_t working_color_map[font->arr_limit];
    float x = start_x;
    float y = start_y;
    uint32_t working_index = 0;
    for (uint64_t i = 0; i < len; i++) {
        int32_t ch_int = (int32_t)buffer[i];
        Character* character = &font->character[ch_int];
        if (buffer[i] == u'\n') {
            y -= nl_height;
            x = start_x;
        }
        else if (buffer[i] == u' ')
            x += advance;
        else if (!character->processed || ch_int > font->range[font->texture_count - 1][1])
            continue;
        else {
            float xpos = x + character->bearing.x;
            float ypos = y + character->bearing.y - font->size;
            if (gui_text)
                ;
            else if (y > y_limit[0] || y < y_limit[1])
                continue;
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

            working_color_map[working_index] = color_map[i];

            math_translate_f4x4(translate, xpos, ypos, 1.0f);
            math_scale_f4x4(scale_matrix, font->size, font->size, 0);
            math_multiply_f4x4(font->transforms + (working_index * 16), translate, scale_matrix);
            x += advance;
            working_index++;
            if (working_index == font->arr_limit) {
                render_text_call(&font->shader, font->transforms, font->letter_map, working_color_map, font->texture_map, working_index);
                working_index = 0;
            }
        }
    }
    if (working_index != 0)
        render_text_call(&font->shader, font->transforms, font->letter_map, working_color_map, font->texture_map, working_index);
}

void render_gui() {
    float current_x = 0;
    static float rect_width = 0, rect_height = 0, close_size = 0, rect_y = 0, close_y = 0, text_ypos = 0;

    if (editor->flags & FLAGS_RENGER_GUI_UPDATE) {
        rect_width  = gui->size.x;
        rect_height = gui->size.y >> 4;
        close_size = font->size;
        rect_y = editor->height - gui->size.y;
        close_y = editor->height - close_size - (gui->size.y >> 2);
        text_ypos = editor->height - font->size - (font->size / 3);
        editor->flags &= ~FLAGS_RENGER_GUI_UPDATE;
    }
    
    for (uint8_t i = 0; i < editor->files_opened; i++) {
        float model[16] = {0}, mvp[16] = {0};
        float rect_x = current_x;
        
        shader_bind(&gui->shader);
        vao_bind(&gui->vao[0]);
        
        memset(model, 0, sizeof(model));
        math_identity_f4x4(model, 1.0f);
        math_translate_f4x4(model, rect_x, rect_y, 0.0f);
        math_scale_f4x4(model, rect_width, rect_height, 1.0f);
    
        math_multiply_f4x4(mvp, gui_projection, model);
        
        shader_uniform_mat4(&gui->shader, "mvp", mvp, 1);
        if (i == editor->current_file) 
            shader_uniform_float_4(&gui->shader, "color", cold_purple[0], cold_purple[1], cold_purple[2], 1.0f);
        else
            shader_uniform_float_4(&gui->shader, "color", gray[0], gray[1], gray[2], 1.0f);
        
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, gui->rectangle_indices);
        
        vao_bind(&gui->vao[1]);
        
        float close_x = current_x + gui->size.x - close_size;
        
        memset(model, 0, sizeof(model));
        math_identity_f4x4(model, 1.0f);
        math_translate_f4x4(model, close_x, close_y, 0.0f);
        math_scale_f4x4(model, close_size, close_size, 1.0f);
        
        math_multiply_f4x4(mvp, gui_projection, model);
        
        shader_uniform_mat4(&gui->shader, "mvp", mvp, 1);
        shader_uniform_float_4(&gui->shader, "color", white[0], white[1], white[2], 1.0f); 
        
        glLineWidth(2.0f * editor->dpi_scale);
        glDrawArrays(GL_LINES, 0, 2);
        glDrawArrays(GL_LINES, 2, 2);

        char16_t* name    = editor->files[i].file_name;
        uint32_t name_len = u_strlen(name);
        int32_t color_map[name_len];
        memset(color_map, 0, name_len * sizeof(int32_t));

        float text_xpos = current_x + text_start_x;

        render_text_bind(1);
        render_text(name, color_map, name_len, text_xpos, text_ypos, 1);
        current_x += gui->size.x;
    }
}