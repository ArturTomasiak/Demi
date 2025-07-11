#pragma once

////////////////////////
// includes & defines //
////////////////////////

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <uchar.h>

#include <glad/gl.h>  
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_LCD_FILTER_H

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__)
#define demiwindows
#define UNICODE
#define _UNICODE
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#elif defined(__linux__) || defined(__gnu_linux__)
#define demilinux
#include <pthread.h>
#endif

#if defined(_MSC_VER) && !defined(__clang__)
  #define inline __inline
  #define restrict __restrict
  #undef extern
  #define extern
#endif

//////////////////
// declarations //
//////////////////

extern const float light_violet[3];
extern const float violet[3];
extern const float orange[3];
extern const float gray[3];
extern const float dark[3];
extern uint8_t text_start_x;
extern const uint8_t gui_margin_y;

//////////////
// typedefs //
//////////////

#pragma pack(push, 1)  // remove padding, similar to __attribute__ ((packed))

typedef struct {
    int32_t location;
    char* name;    
} shader_uniform_cache;

typedef struct {
    uint32_t renderer_id;
    uint32_t cache_length;
    shader_uniform_cache* cache;
} Shader;

typedef struct {
    uint32_t renderer_id;
} VBO;

typedef struct {
    uint32_t renderer_id;
} VAO;

typedef struct {
    uint32_t count;
    uint32_t type;
    uint32_t type_size;
    _Bool normalized;
} LayoutElement;

typedef struct {
    LayoutElement* elements;
    uint32_t element_count;
    uint32_t stride;
} VertexBufferLayout;

typedef struct {
    void* window_data;
} Platform;

typedef struct {
    int32_t x;
    int32_t y;
} Size;

typedef enum {
    UTF8,
    UTF16BE,
    UTF16LE,
    UTF32BE,
    UTF32LE,
    PDF
} Encoding;

typedef struct {
    char16_t* buffer;
    int32_t*  color_map;
    uint64_t  length;
    uint64_t  allocated_memory;
    uint64_t  position;
    uint64_t  color_map_change[2];
} StringBuffer;

typedef struct {
    Encoding  encoding;
    char16_t* path;
    char16_t* file_name;
    _Bool     file;
    _Bool     saved_progress;

    float camera_x; 
    float camera_y;

    StringBuffer string;
} DemiFile;

typedef struct {
    Shader shader;
    VAO vao[2];
    VBO vbo[2];
    Size size;
    uint32_t rectangle_indices[6];
} GUI;

typedef struct {
    char16_t* lines;
    int32_t*  color_map;
    uint64_t  lines_len;
    uint64_t  last_line;
    uint64_t  current_line;
    uint32_t  since_nl;
} RenderData;

typedef struct {
    float      dpi_scale;
    float      scroll_speed;
    uint8_t    flags; // flags description at the bottom of the file
    uint8_t    files_opened;
    uint8_t    current_file;
    uint32_t   dpi;
    int32_t    uniform_limit;
    DemiFile*  files;
    RenderData data;
    GUI        gui;

    int32_t width;
    int32_t height; 
} Editor;

typedef struct {
    Size size;
    Size bearing;
    uint32_t advance;
    _Bool processed;
} Character;

typedef struct {
    uint8_t texture_count;
    uint16_t range[9][2];
    uint32_t texture[9];
    Character* character;

    float line_spacing;
    float* transforms;
    int32_t* texture_map;
    int32_t* letter_map;

    uint16_t size;
    Shader shader;
    VAO vao;
    VBO vbo;

    int32_t arr_limit;

    FT_Library ft_lib;
    FT_Face ft_face;
} DemiFont;

#pragma pack(pop)

/////////////
// error.c //
/////////////

void error(char16_t* err, char16_t* title);
void fatal_error (char16_t* msg);

#ifdef demidebug
void check_gl_errors();
#endif

///////////
// flags //
///////////

// flags & 0b1 -> 1 = opengl46, 0 = opengl33
// flags & 0b10 -> 1 = minimized/not focused
// flags & 0b100 -> 1 = open settings
// flags & 0b1000 -> 1 = vsync off
// flags & 0b10000 -> 1 = render_gui update
// flags & 0b100000 -> 1 = adjust camera to cursor
// flags & 0b1000000 -> 1 = unused