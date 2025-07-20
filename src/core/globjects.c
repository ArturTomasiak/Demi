#include "globjects.h"
#include <string.h>

////////////
// shader //
////////////

static uint32_t shader_compile(uint32_t type, const char* source) {
    uint32_t id = glCreateShader(type);
    glShaderSource(id, 1, &source, 0);
    glCompileShader(id);
    int32_t result;
    glGetShaderiv(id, GL_COMPILE_STATUS, &result);
    if (!result) {
        #ifdef demidebug
        int32_t len;
        glGetShaderiv(id, GL_INFO_LOG_LENGTH, &len);
        char message[len];
        glGetShaderInfoLog(id, len, &len, message);
        printf("%s\n", message);
        #endif
        glDeleteShader(id);
        fatal_error(u"shader compilation failed");
    }
    return id;
}

static char* shader_read(const char* const location, uint8_t texture_count, int32_t arr_limit, _Bool gl46) {
    char define_str[100];
    if (gl46)
        sprintf(define_str, "#version 460 core\n#define texture_count %d\n#define arr_limit %d\n", texture_count, arr_limit);
    else
        sprintf(define_str, "#version 330 core\n#define texture_count %d\n#define arr_limit %d\n", texture_count, arr_limit);
    uint32_t define_size = strlen(define_str);
    FILE* file_pointer = fopen(location, "rb");
    if (!file_pointer) {
        char error[50 + strlen(location)];
        sprintf(error, "failed to open file at: %s\n", location);
        uint16_t len = strlen(error);
        char16_t uerror[len+1];
        for (uint16_t i = 0; i < len; i++)
            uerror[i] = (char16_t)error[i];
        uerror[len] = '\0';
        fclose(file_pointer);
        fatal_error(uerror);
        return NULL;
    }
    fseek(file_pointer, 0, SEEK_END);
    uint32_t file_size = ftell(file_pointer);
    rewind(file_pointer);
    char* buffer = malloc(file_size + define_size + 1);
    if (!buffer) {
        fclose(file_pointer);
        fatal_error(u"memory allocation failed");
        return NULL;
    }
    strcpy(buffer, define_str);
    uint64_t read_size = fread(buffer + define_size, 1, file_size, file_pointer);
    buffer[define_size + read_size] = '\0';
    fclose(file_pointer);
    return buffer;
}

void shader_init(Shader* restrict shader, const char* const vertex_shader_path, const char* const fragment_shader_path, uint8_t texture_count, int32_t arr_limit, _Bool gl46) {
    char* vertex_shader   = shader_read(vertex_shader_path, texture_count, arr_limit, gl46);
    char* fragment_shader = shader_read(fragment_shader_path, texture_count, arr_limit, gl46);
    shader->renderer_id = glCreateProgram();
    uint32_t vs = shader_compile(GL_VERTEX_SHADER, vertex_shader);
    uint32_t fs = shader_compile(GL_FRAGMENT_SHADER, fragment_shader);
    if (!vs || !fs)
        return; // shader compile calls fatal_error
    glAttachShader(shader->renderer_id, vs);
    glAttachShader(shader->renderer_id, fs);
    glLinkProgram(shader->renderer_id);
    glDeleteShader(vs);
    glDeleteShader(fs);
    free(vertex_shader);
    free(fragment_shader);
}

void shader_destruct(Shader* restrict shader) {
    for (int i = 0; i < shader->cache_length; i++)
        free(shader->cache[i].name);
    if (shader->cache) {
        free(shader->cache);
        shader->cache = NULL;
        shader->cache_length = 0;
    }
    glDeleteProgram(shader->renderer_id);
    shader->renderer_id = 0;
}

void shader_bind(const Shader* restrict shader) {
    glUseProgram(shader->renderer_id);
}

extern inline void shader_unbind() {
    glUseProgram(0);
}

extern inline void shader_uniform_int32_t(Shader* restrict shader, const char* const name, int32_t value) {
    glUniform1i(get_uniform_location(shader, name), value);
}

extern inline void shader_uniform_int32_t_arr(Shader* restrict shader, const char* const name, int32_t* matrix, uint32_t length) {
    glUniform1iv(get_uniform_location(shader, name), length, matrix);
}

extern inline void shader_uniform_float(Shader* restrict shader, const char* const name, float value)  {
    glUniform1f(get_uniform_location(shader, name), value);
}

extern inline void shader_uniform_float_3(Shader* restrict shader, const char* const name, float v0, float v1, float v2) {
    glUniform3f(get_uniform_location(shader, name), v0, v1, v2);
}

extern inline void shader_uniform_float_4(Shader* restrict shader, const char* const name, float v0, float v1, float v2, float v3) {
    glUniform4f(get_uniform_location(shader, name), v0, v1, v2, v3);
}

extern inline void shader_uniform_mat4(Shader* restrict shader, const char* const name, float* matrix, uint32_t length) {
    glUniformMatrix4fv(get_uniform_location(shader, name), length, GL_FALSE, matrix);
}

int32_t get_uniform_location(Shader* restrict shader, const char* const name) {
    for (uint32_t i = 0; i < shader->cache_length; i++)
        if (strcmp(shader->cache[i].name, name) == 0)
            return shader->cache[i].location;

    int32_t location = glGetUniformLocation(shader->renderer_id, name);
    #ifdef demidebug
    if (location == -1)
       printf("uniform %s does not exist\n", name);
    #endif
    shader->cache_length++;
    shader->cache = realloc(shader->cache, sizeof(shader_uniform_cache) * shader->cache_length);
    if (!shader->cache) {
        fatal_error(u"memory_allocation_failed");
        return location;
    }
    uint16_t len = strlen(name);
    shader->cache[shader->cache_length - 1].name = malloc(len+1);
    if (!shader->cache[shader->cache_length - 1].name) {
        fatal_error(u"memory_allocation_failed");
        return location;
    }
    memcpy(shader->cache[shader->cache_length - 1].name, name, len);
    shader->cache[shader->cache_length - 1].location = location;
    return location;
}

/////////
// vbo //
/////////

void vbo_init(VBO* restrict vbo, const void* data, uint32_t size) {
    glGenBuffers(1, &vbo->renderer_id);
    glBindBuffer(GL_ARRAY_BUFFER, vbo->renderer_id);
    glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW);
}

void vbo_destruct(VBO* restrict vbo) {
    glDeleteBuffers(1, &vbo->renderer_id);
    vbo->renderer_id = 0;
}

extern inline void vbo_bind(const VBO* restrict vbo) {
    glBindBuffer(GL_ARRAY_BUFFER, vbo->renderer_id);
}

extern inline void vbo_unbind() {
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

/////////
// vao //
/////////

extern inline void vao_init(VAO* restrict vao) {
    glGenVertexArrays(1, &vao->renderer_id);
}

void vao_destruct(VAO* restrict vao) {
    glDeleteVertexArrays(1, &vao->renderer_id);
    vao->renderer_id = 0;
}

extern inline void vao_bind(const VAO* restrict vao) {
    glBindVertexArray(vao->renderer_id);
}

extern inline void vao_unbind() {
    glBindVertexArray(0);
}

void vao_layout_destruct(VertexBufferLayout* restrict layout) {
    if (layout->elements) {
        free(layout->elements);
        layout->elements = NULL;
    }
    layout->element_count = 0;
    layout->stride = 0;
}

void vao_add_element(VertexBufferLayout* restrict layout, uint32_t count, uint32_t type, uint32_t type_size, _Bool normalized) {
    layout->element_count++;
    layout->stride += count * type_size;
    layout->elements = realloc(layout->elements, sizeof(LayoutElement) * layout->element_count);
    if (!layout->elements)
        fatal_error(u"memory allocation failed");
    layout->elements[layout->element_count - 1].count = count;
    layout->elements[layout->element_count - 1].type = type;
    layout->elements[layout->element_count - 1].type_size = type_size;
    layout->elements[layout->element_count - 1].normalized = normalized;
}

void vao_add_buffer(VertexBufferLayout* restrict layout, const VAO* restrict vao) {
    vao_bind(vao);
    uint64_t offset = 0;
    for (uint32_t i = 0; i < layout->element_count; i++) {
        glEnableVertexAttribArray(i);
        glVertexAttribPointer(i, 
            layout->elements[i].count, 
            layout->elements[i].type, 
            layout->elements[i].normalized, 
            layout->stride, 
            (const void *) offset
        );
        offset += layout->elements[i].type_size * layout->elements[i].count;
    }
}