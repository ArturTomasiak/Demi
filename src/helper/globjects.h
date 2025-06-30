#pragma once
#include "../include.h"

////////////
// shader //
////////////

void shader_init(Shader* restrict shader, const char* vertex_shader_path, const char* fragment_shader_path, int32_t arr_limit, _Bool gl46);
void shader_destruct(Shader* restrict shader);
void shader_bind(const Shader* restrict shader);
void shader_unbind();
void shader_uniform_int32_t(Shader* restrict shader, const char* name, int32_t value);
void shader_uniform_int32_t_arr(Shader* restrict shader, const char* name, int32_t* matrix, uint32_t length);
void shader_uniform_float(Shader* restrict shader, const char* name, float value);
void shader_uniform_float_3(Shader* restrict shader, const char* name, float v0, float v1, float v2);
void shader_uniform_float_4(Shader* restrict shader, const char* name, float v0, float v1, float v2, float v3);
void shader_uniform_mat4(Shader* restrict shader, const char* name, float* matrix, uint32_t length);
int32_t get_uniform_location(Shader* restrict shader, const char* name);

/////////
// vbo //
/////////

void vbo_init(VBO* restrict vbo, const void* data, uint32_t size);
void vbo_destruct(VBO* restrict vbo);
void vbo_bind(const VBO* restrict vbo);
void vbo_unbind();

/////////
// vao //
/////////

void vao_init(VAO* restrict vao);
void vao_destruct(VAO* restrict vao);
void vao_bind(const VAO* restrict vao);
void vao_unbind();

void vao_layout_destruct(VertexBufferLayout* restrict layout);
void vao_add_element(VertexBufferLayout* restrict layout, uint32_t count, uint32_t type, uint32_t type_size, _Bool normalized);
void vao_add_buffer(VertexBufferLayout* restrict layout, const VAO* restrict vao);