#include "core/platform_layer.h"
#include "core/editor.h"
#include "core/font.h"
#include "core/gui.h"
#include "main_helper.c"

const float light_violet[3] = {149.0f / 255, 106.0f / 255, 1.0f};
const float violet[3]       = {149.0f / 255, 106.0f / 255, 1.0f};
const float orange[3]       = {240.0f / 255, 128.0f / 255, 0.0f};
const float gray[3]         = {129.0f / 255, 133.0f / 255, 137.0f / 255};
const float dark[3]         = {19.0f / 255, 19.0f / 255, 19.0f / 255};
uint8_t text_start_x = 10;
const uint8_t gui_margin_y = 5;
GUI*      restrict gui;
Editor*   restrict editor;
DemiFont* restrict font;

static _Bool running = 1;

#ifdef demiwindows
static HANDLE run_prepare;
static HANDLE prepare_complete;
static HANDLE thread;
static DWORD  thread_id;
DWORD WINAPI prepare(void* arg) {
    while (1) {
        WaitForSingleObject(run_prepare, INFINITE);
        if (!running) break;
        setup_lines(editor);
        SetEvent(prepare_complete);
    }
    return 0;
}
#elif defined(demilinux)
void* prepare(void* arg) {
    // TODO
}
#endif

void multithreading_init();
void multithreading_destruct();

#if defined(demidebug) || defined(demilinux)
int32_t main() {
#else
#include <windows.h>
int32_t CALLBACK WinMain(
    HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    LPSTR lpCmdLine,
    int32_t nCmdShow 
) {
#endif
    GUI      gui_original      = {0};
    Editor   editor_original   = {0};
    DemiFont font_original     = {0};
    KeywordMap keyword_map     = {0};
    gui =    &gui_original;
    editor = &editor_original;
    font   = &font_original;
    Platform platform = {0};

    platform_init(&platform, u"DemiEditor");
    editor_init();
    keyword_map_init(&keyword_map);
    gui_init(editor->flags & FLAGS_GL46);
    font_init(editor->uniform_limit, editor->flags & FLAGS_GL46); 
    font_rebuild(16, editor->dpi_scale);
    render_init(editor->width, editor->height, &editor->files[editor->current_file]);
    multithreading_init();

    RenderData* data = &editor->data;
    Character*  num  = &font->character[u'9'];
    float text_x;
    float text_y;
    float buffer_text_x;
    float cursor_text_x;
    float cursor_text_y;
    float nl_height;
    DemiFile* current;
    wchar_t cursor[1]    = u"|";
    int32_t cursor_color_map[1] = {1};
    
    platform_show_window(&platform);
    while (running) {
        while (editor->flags & FLAGS_MINIMIZED) {
            platform_sleep(100);
            platform_msg(&running);
        }

        if (editor->flags & FLAGS_VSYNC_OFF)
            platform_sleep(5);

        if (editor->flags & FLAGS_FONT_RESIZED) {
            nl_height = (font->character[u'\n'].size.y * font->line_spacing);
            render_update_nl_height(nl_height);

            editor->flags &= ~FLAGS_FONT_RESIZED;
        }
            
        glClearColor(dark[0], dark[1], dark[2], 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        #ifdef demidebug
        check_gl_errors();
        #endif

        if (editor->files_opened) {
            #ifdef demiwindows
            SetEvent(run_prepare);
            #elif defined(demilinux)
            // TODO
            #endif

            text_x = text_start_x;
            text_y = editor->height - font->size - gui->size.y - gui_margin_y;
            current = &editor->files[editor->current_file];
        }

        render_gui();

        if (editor->files_opened) {
            #ifdef demiwindows
            WaitForSingleObject(prepare_complete, INFINITE);
            #elif defined(demilinux)
            // TODO
            #endif

            buffer_text_x = (text_x * 2) + (num->advance * num_count(data->last_line));
            cursor_text_x = buffer_text_x + (num->advance * data->since_nl) - (num->advance >> 1);
            cursor_text_y = text_y - (nl_height * (data->current_line - 1)); 
            
            if (editor->flags & FLAGS_ADJUST_CAMERA_TO_CURSOR) {
                editor_camera_to_cursor(cursor_text_x, cursor_text_y, num->advance, nl_height, buffer_text_x);
                editor->flags &= ~FLAGS_ADJUST_CAMERA_TO_CURSOR;
            }

            render_text_bind(0);
            render_text(data->lines, data->color_map, data->lines_len, text_x, text_y, 0);
            render_text(current->string.buffer, current->string.color_map, current->string.length, buffer_text_x, text_y, 0);
            render_text(cursor, cursor_color_map, 1, cursor_text_x, cursor_text_y, 0);
        }

        platform_msg(&running);
        platform_swap_buffers(&platform);
    }
    platform_hide_window(&platform);

    for (uint8_t i = 0; i < editor->files_opened; i++)
        file_close(i);
    multithreading_destruct();
    font_destruct();
    editor_destruct();
    platform_destruct(&platform);
    gui_destruct();
    keyword_map_destruct();
}

void multithreading_init() {
    #ifdef demiwindows
    run_prepare = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (!run_prepare)
        fatal_error(u"event creation failed");

    prepare_complete = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (!run_prepare)
        fatal_error(u"event creation failed");

    thread = CreateThread(NULL, 0, prepare, 0, 0, &thread_id);
    if (!thread)
        fatal_error(u"thread creation failed");
    #elif defined(demilinux)
    // TODO
    #endif
}

void multithreading_destruct() {
    #ifdef demiwindows
        SetEvent(run_prepare);
        WaitForSingleObject(thread, INFINITE);
        CloseHandle(thread);
        CloseHandle(run_prepare);
    #elif defined(demilinux)
    // TODO
    #endif
}