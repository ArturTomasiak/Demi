#include "../../include.h"

#ifdef demiwindows

#include <windows.h>

void error(char16_t* err, char16_t* title) {
    MessageBox(NULL, err, title, MB_OK);
}

void fatal_error(char16_t* msg) {
    MessageBox(NULL, msg, u"Error", MB_ICONERROR | MB_OK);
    PostQuitMessage(-1);
}

#ifdef demidebug
static const DWORD orange = FOREGROUND_RED | FOREGROUND_GREEN;
static const DWORD reset  = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;

static char* translate_gl_error(GLenum error) {
    switch (error) {
        case GL_INVALID_ENUM:
            return "invalid enumeration parameter";
        case GL_INVALID_VALUE:
            return "invalid value parameter";
        case GL_INVALID_OPERATION:
            return "invalid operation";
        case GL_STACK_OVERFLOW:
            return "stack overflow";
        case GL_STACK_UNDERFLOW:
            return "stack underflow";
        case GL_OUT_OF_MEMORY:
            return "memory allocation failed";
        case GL_INVALID_FRAMEBUFFER_OPERATION:
            return "writing to incomplete framebuffer";
        default:
            return "unknown code";
    }
}

void check_gl_errors() {
    GLenum error;
    HANDLE console_output = GetStdHandle(STD_OUTPUT_HANDLE);
    while ((error = glGetError())) {
        SetConsoleTextAttribute(console_output, orange);
        printf("[glGetError] ");
        SetConsoleTextAttribute(console_output, reset);
        printf("%s", translate_gl_error(error));
        printf("%s%d%s\n", " (", error, ")");
    }
}
#endif

#endif