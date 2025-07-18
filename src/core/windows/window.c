#include "../platform_layer.h"

#ifdef demiwindows

#include <dwmapi.h>
#include <glad/wgl.h>

// I am aware that win32's UNICODE mode expects wchar_t strings, 
// it can be used interchangably with char16_t on windows

typedef struct {
    HINSTANCE hinstance;
    WNDCLASSEX wc;
    HGLRC hglrc;
    HWND hwnd;
    HDC hdc;
} Windows;

static inline HGLRC create_temp_context(HDC hdc, HWND hwnd);
static inline HGLRC create_context(HDC hdc, HWND hwnd, _Bool* gl_version_fallback);
static inline _Bool create_window(WNDCLASSEX* wc, HINSTANCE hinstance, HWND* hwnd, const char16_t* app_name, int32_t width, int32_t height);

LRESULT CALLBACK process_message(HWND hwnd, UINT msg, WPARAM w_param, LPARAM l_param) { 
    Editor* editor = (Editor*)GetWindowLongPtr(hwnd, 0);
    DemiFont* font = (DemiFont*)GetWindowLongPtr(hwnd, sizeof(void*));
    switch(msg) {
        case WM_CLOSE: 
            PostQuitMessage(0);
        break;
        case WM_DPICHANGED:
            editor_dpi_change(editor, font, LOWORD(w_param));
        break;
        case WM_ACTIVATE:
            if (LOWORD(w_param) == WA_INACTIVE)
                editor->flags |= 0b10;
            else
                editor->flags &= ~0b10;
        break;
        case WM_SIZE:
            if (w_param == SIZE_MINIMIZED)
                editor->flags |= 0b10;
            else {
                editor->flags &= ~0b10;
                editor->flags |= 0b10000;

                RECT client_rect;
                GetClientRect(hwnd, &client_rect);
                editor->width = client_rect.right - client_rect.left;
                editor->height = client_rect.bottom - client_rect.top;
                editor_resize(editor);
            }
        break;
        case WM_COMMAND: 
            switch(LOWORD(w_param)) {
                case 1001: // open file
                    file_open_explorer(editor);
                break;
                case 1002: // save file
                    // TODO
                break;
                case 1003: // save file as
                    // TODO
                break;
                case 1004: // settings
                    editor->flags |= 0b100;
                break;
            }
        break;
        case WM_MOUSEWHEEL:
            editor_mouse_wheel(editor, (short)HIWORD(w_param));
        break;
        case WM_LBUTTONUP:
            editor_left_click(editor, font, (float)(short)LOWORD(l_param), (float)(short)HIWORD(l_param)); 
        break;
        case WM_CHAR:
            switch(w_param) {
                case 8:
                    editor_backspace(editor);
                break;
                case 9:
                    editor_tab(editor);
                break;
                case 13:
                    editor_enter(editor);
                break;
                default:
                    if (is_printable(font, w_param))
                        editor_input(editor, w_param);
                break;
            }
        break;
        case WM_KEYDOWN:
            switch(w_param) {
                case 'V':
                    if (GetKeyState(VK_CONTROL) & 0x8000) {
                        if (IsClipboardFormatAvailable(CF_UNICODETEXT)) {
                            if (OpenClipboard(hwnd)) {
                                HANDLE clipboard = GetClipboardData(CF_UNICODETEXT);
                                if (clipboard) {
                                    char16_t* text = GlobalLock(clipboard);
                                    if (text) {
                                        editor_paste(editor, text);
                                        GlobalUnlock(clipboard);
                                    }
                                }
                                CloseClipboard();
                            }
                        }
                    }
                break;
                case 'S':
                    if (GetKeyState(VK_CONTROL) & 0x8000) {
                        // TODO
                    }
                break;
                case VK_UP:
                    editor_up(editor);
                break;
                case VK_LEFT:
                    editor_left(editor);
                break;
                case VK_RIGHT:
                    editor_right(editor);
                break;
                case VK_DOWN:
                    editor_down(editor);
                break;
            }
        break;
    }
    return DefWindowProc(hwnd, msg, w_param, l_param);
}

void platform_msg(_Bool* running) {
    static MSG msg;
    while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
        if (msg.message == WM_QUIT)
            *running = 0;
        else {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }
}

void platform_swap_buffers(Platform* restrict platform) {
    Windows* win = platform->window_data;
    SwapBuffers(win->hdc);
}

void platform_destruct(Platform* restrict platform) {
    free(platform->window_data);
}

_Bool platform_init(Platform* restrict platform, Editor* restrict editor, DemiFont* restrict font, const char16_t* app_name) {
    Windows* win = malloc(sizeof(Windows));
    platform->window_data = win;
    SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
    win->hinstance = GetModuleHandle(NULL);

    editor->width  = 960;
    editor->height = 540;

    if (!create_window(&win->wc, win->hinstance, &win->hwnd, app_name, editor->width, editor->height))
        return 0;

    SetWindowLongPtr(win->hwnd, 0, (LONG_PTR)editor);
    SetWindowLongPtr(win->hwnd, sizeof(void*), (LONG_PTR)font);

    win->hdc = GetDC(win->hwnd);
    HGLRC temp_context = create_temp_context(win->hdc, win->hwnd);

    if (!gladLoadWGL(win->hdc, (GLADloadfunc)wglGetProcAddress)) {
        fatal_error(u"failed to load WGL");
        ReleaseDC(win->hwnd, win->hdc);
        DestroyWindow(win->hwnd);
        return 0;
    }

    _Bool gl_fallback_version;
    win->hglrc = create_context(win->hdc, win->hwnd, &gl_fallback_version);
    if (gl_fallback_version)
        editor->flags = 0;
    else
        editor->flags = 1;
    wglDeleteContext(temp_context);

    if (!gladLoaderLoadGL()) {
        wglDeleteContext(temp_context);
        fatal_error(u"failed to load openGL");
        ReleaseDC(win->hwnd, win->hdc);
        DestroyWindow(win->hwnd);
        return 0;
    }

    if (wglSwapIntervalEXT)
        wglSwapIntervalEXT(1);
    #ifdef demidebug
    else 
        printf("%s\n", "vsync not supported");
    #endif

    editor->dpi = GetDpiForWindow(win->hwnd);

    return 1;
}

void platform_show_window(Platform* restrict platform) {
    Windows* win = platform->window_data;
    ShowWindow(win->hwnd, SW_SHOW);
    UpdateWindow(win->hwnd);
}

static inline HGLRC create_temp_context(HDC hdc, HWND hwnd) {
    PIXELFORMATDESCRIPTOR pfd = {
        sizeof(PIXELFORMATDESCRIPTOR),
        1,
        PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER, 
        PFD_TYPE_RGBA,
        32,
        0, 0, 0, 0, 0, 0,
        0,
        0,
        0,
        0, 0, 0, 0,
        24,
        8,
        0,
        PFD_MAIN_PLANE,
        0, 0, 0, 0
    };
    int32_t pixel_format = ChoosePixelFormat(hdc, &pfd);
    SetPixelFormat(hdc, pixel_format, &pfd);
    HGLRC temp_context = wglCreateContext(hdc);
    wglMakeCurrent(hdc, temp_context);
    return temp_context;
}

static inline HGLRC create_context(HDC hdc, HWND hwnd, _Bool* gl_version_fallback) {
    const int attrib_list[] =
    {
        WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
        WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
        WGL_DOUBLE_BUFFER_ARB, GL_TRUE,
        WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB,
        WGL_COLOR_BITS_ARB, 32,
        WGL_DEPTH_BITS_ARB, 24,
        WGL_STENCIL_BITS_ARB, 8,
        WGL_SAMPLE_BUFFERS_ARB, 1,
        WGL_SAMPLES_ARB, 4,
        0,
    };
    int pixel_format;
    UINT num_format;
    if (!wglChoosePixelFormatARB(hdc, attrib_list, NULL, 1, &pixel_format, &num_format))
        goto fail;
    const int32_t attribs[] = {
        WGL_CONTEXT_MAJOR_VERSION_ARB, 4,
        WGL_CONTEXT_MINOR_VERSION_ARB, 6,
        WGL_CONTEXT_PROFILE_MASK_ARB,  WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
        0
    };
    HGLRC hglrc = wglCreateContextAttribsARB(hdc, 0, attribs);
    *gl_version_fallback = 0;
    if (!hglrc) {
            const int32_t attribs_fallback[] = {
                WGL_CONTEXT_MAJOR_VERSION_ARB, 3,
                WGL_CONTEXT_MINOR_VERSION_ARB, 3,
                WGL_CONTEXT_PROFILE_MASK_ARB,  WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
                0
            };
            HGLRC fallback = wglCreateContextAttribsARB(hdc, 0, attribs_fallback);
            *gl_version_fallback = 1;
            if (!fallback)
                goto fail;
            return fallback;
    }
    wglMakeCurrent(hdc, hglrc);
    return hglrc;

fail:
    fatal_error(u"failed to create openGL context");
    ReleaseDC(hwnd, hdc);
    DestroyWindow(hwnd);
    ExitProcess(0);
}

static inline _Bool create_window(WNDCLASSEX* wc, HINSTANCE hinstance, HWND* hwnd, const char16_t* app_name, int32_t width, int32_t height) {
    const char16_t* application_icon = u"..\\resources\\icons\\icon.ico";
    uint32_t attributes = GetFileAttributes(application_icon);
    if (attributes == INVALID_FILE_ATTRIBUTES || attributes & FILE_ATTRIBUTE_DIRECTORY) {
        fatal_error(u"could not find resources/icons/icon.ico");
        return 0;
    }

    HMENU menu_bar  = CreateMenu();
    HMENU file_menu = CreatePopupMenu();
    AppendMenuW(file_menu, MF_STRING, 1001, L"Open File");
    AppendMenuW(file_menu, MF_STRING, 1002, L"Save File");
    AppendMenuW(file_menu, MF_STRING, 1003, L"Save File As");
    AppendMenuW(file_menu, MF_SEPARATOR, 0, NULL);
    AppendMenu(file_menu, MF_STRING, 1004, L"Settings");
    AppendMenuW(menu_bar, MF_POPUP, (UINT_PTR)file_menu, L"File");
 
    ZeroMemory(wc, sizeof *wc);
    wc->cbSize = sizeof(*wc);
    wc->style = CS_OWNDC;
    wc->lpfnWndProc = process_message;
    wc->hInstance = hinstance;
    wc->lpszClassName = app_name;
    wc->hIcon = (HICON)LoadImage(hinstance, application_icon, IMAGE_ICON, 32, 32, LR_LOADFROMFILE);
    wc->hIconSm = (HICON)LoadImage(hinstance, application_icon, IMAGE_ICON, 16, 16, LR_LOADFROMFILE);
    wc->hCursor = LoadCursor(NULL, IDC_ARROW);
    wc->cbWndExtra = sizeof(void*) * 2;
    RegisterClassEx(wc);
    *hwnd = CreateWindowEx(
        0,
        app_name,
        app_name,
        WS_CAPTION | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_SIZEBOX | WS_SYSMENU,
        200, 200,
        width, height, 
        0,
        menu_bar,
        hinstance, 
        0
    );
    if (!*hwnd) {
        fatal_error(u"window creation failed");
        return 0;
    }
    COLORREF caption_color = RGB(255, 255, 255); 
    COLORREF border_color  = RGB(149, 106, 255);
    DwmSetWindowAttribute(*hwnd, DWMWA_CAPTION_COLOR, &caption_color, sizeof(caption_color));
    DwmSetWindowAttribute(*hwnd, DWMWA_BORDER_COLOR, &border_color, sizeof(border_color));
    DWM_WINDOW_CORNER_PREFERENCE preference = DWMWCP_DONOTROUND;
    DwmSetWindowAttribute(*hwnd, DWMWA_WINDOW_CORNER_PREFERENCE, &preference, sizeof(preference));
    return 1;
}

extern inline void platform_sleep(uint32_t millis) {
    Sleep(millis);
}

#endif