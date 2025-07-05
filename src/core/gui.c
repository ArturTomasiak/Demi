#include "gui.h"

void gui_size(GUI* restrict gui, uint32_t width, uint32_t height, float dpi_scale) {
    gui->size.x = width * dpi_scale;
    gui->size.y = height * dpi_scale;
}

void gui_init(GUI* restrict gui, _Bool gl46) {
    shader_init(&gui->shader, "../resources/shaders/gui_vert.glsl", "../resources/shaders/gui_frag.glsl", 0, 0, gl46);
    
    gui->rectangle_indices[0] = 0;
    gui->rectangle_indices[1] = 1;
    gui->rectangle_indices[2] = 2;
    gui->rectangle_indices[3] = 2;
    gui->rectangle_indices[4] = 3;
    gui->rectangle_indices[5] = 0;
    
    vao_init(&gui->vao[0]);
    vao_init(&gui->vao[1]);

    const float rect_vertices[8] = {
        0.0f, 0.0f,
        1.0f, 0.0f,
        1.0f, 1.0f,
        0.0f, 1.0f
    };
    
    vbo_init(&gui->vbo[0], rect_vertices, sizeof(rect_vertices));
    vao_bind(&gui->vao[0]);
    vbo_bind(&gui->vbo[0]);
    VertexBufferLayout rect_layout = {0};
    vao_add_element(&rect_layout, 2, GL_FLOAT, sizeof(float), 0);
    vao_add_buffer(&rect_layout, &gui->vao[0]);
    vao_layout_destruct(&rect_layout);
    
    const float close_vertices[8] = {
        0.2f, 0.2f,
        0.8f, 0.8f,
        0.8f, 0.2f,
        0.2f, 0.8f
    };
    
    vbo_init(&gui->vbo[1], close_vertices, sizeof(close_vertices));
    vao_bind(&gui->vao[1]);
    vbo_bind(&gui->vbo[1]);
    VertexBufferLayout close_layout = {0};
    vao_add_element(&close_layout, 2, GL_FLOAT, sizeof(float), 0);
    vao_add_buffer(&close_layout, &gui->vao[1]);
    vao_layout_destruct(&close_layout);
}   

void gui_destruct(GUI* restrict gui) {
    shader_destruct(&gui->shader);
    vao_destruct(&gui->vao[0]);
    vao_destruct(&gui->vao[1]);
    vbo_destruct(&gui->vbo[0]);
    vbo_destruct(&gui->vbo[1]);
}