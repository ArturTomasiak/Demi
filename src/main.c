#include "core/platform_layer.h"
#include "core/editor.h"
#include "core/font.h"
#include "core/gui.h"
#include "main_helper.c"

const float light_violet[3] = {149.0f / 255, 106.0f / 255, 1.0f};
const float violet[3]       = {127.0f / 255, 0.0f, 1.0f};
const float orange[3]       = {240.0f / 255, 128.0f / 255, 0.0f};
const float gray[3]         = {129.0f / 255, 133.0f / 255, 137.0f / 255};
const float dark[3]         = {19.0f / 255, 19.0f / 255, 19.0f / 255};
uint8_t text_start_x = 10;
const uint8_t gui_margin_y = 5;

static _Bool running = 1;

#ifdef demiwindows
static HANDLE run_prepare;
static HANDLE prepare_complete;
static HANDLE thread;
static DWORD  thread_id;
DWORD WINAPI prepare(void* arg) {
    Editor* editor = (Editor*)arg;
    while (1) {
        WaitForSingleObject(run_prepare, INFINITE);
        if (!running) break;
        setup_lines_rendering(editor);
        SetEvent(prepare_complete);
    }
    return 0;
}
#elif defined(demilinux)
void* prepare(void* arg) {
    // TODO
}
#endif

static inline void multithreading_init(Editor* restrict editor);
static inline void multithreading_destruct();

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
    Editor editor = {0};
    DemiFont font = {0};
    Platform platform = {0};
    platform_init(&platform, &editor, &font, u"DemiEditor");
    editor_init(&editor);
    color_map_init();
    font_init(&font, editor.uniform_limit, editor.flags & FLAGS_GL46); 
    font_rebuild(&font, &editor, 15, editor.dpi_scale);
    render_init(editor.width, editor.height, &editor.files[editor.current_file], &font);
    gui_init(&editor.gui, editor.flags & FLAGS_GL46);
    multithreading_init(&editor);

    RenderData* data = &editor.data;
    Character*  num  = &font.character[u'9'];
    float text_x;
    float text_y;
    DemiFile* current;
    wchar_t cursor[1]    = u"|";
    int32_t color_map[1] = {1};
    
    platform_show_window(&platform);
    while (running) {
        while (editor.flags & FLAGS_MINIMIZED) {
            platform_sleep(100);
            platform_msg(&running);
        }
        if (editor.flags & FLAGS_VSYNC_OFF)
            platform_sleep(5);
            
        glClearColor(dark[0], dark[1], dark[2], 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        #ifdef demidebug
        check_gl_errors();
        #endif

        if (editor.files_opened) {
            #ifdef demiwindows
            SetEvent(run_prepare);
            #elif defined(demilinux)
            // TODO
            #endif

            text_x = text_start_x;
            text_y = editor.height - font.size - editor.gui.size.y - gui_margin_y;
            current = &editor.files[editor.current_file];
        }

        render_gui(&editor);

        if (editor.files_opened) {
            #ifdef demiwindows
            WaitForSingleObject(prepare_complete, INFINITE);
            #elif defined(demilinux)
            // TODO
            #endif

            float nl_height = (font.character[u'\n'].size.y * font.line_spacing);
            float buffer_text_x = (text_x * 2) + (num->advance * num_count(data->last_line));
            float cursor_text_x = buffer_text_x + (num->advance * data->since_nl) - (num->advance >> 1);
            float cursor_text_y = text_y - (nl_height * (data->current_line - 1)); 
            if (editor.flags & FLAGS_ADJUST_CAMERA_TO_CURSOR) {
                editor_camera_to_cursor(&editor, cursor_text_x, cursor_text_y, num->advance, nl_height, buffer_text_x);
                editor.flags &= ~FLAGS_ADJUST_CAMERA_TO_CURSOR;
            }

            render_text_bind(0);
            render_text(data->lines, data->color_map, data->lines_len, text_x, text_y, &editor);
            render_text(current->string.buffer, current->string.color_map, current->string.length, buffer_text_x, text_y, &editor);
            render_text(cursor, color_map, 1, cursor_text_x, cursor_text_y, &editor);
        }

        platform_msg(&running);
        platform_swap_buffers(&platform);
    }
    platform_hide_window(&platform);

    for (uint8_t i = 0; i < editor.files_opened; i++)
        file_close(&editor, i);
    color_map_destruct();
    multithreading_destruct();
    font_destruct(&font);
    editor_destruct(&editor);
    platform_destruct(&platform);
    gui_destruct(&editor.gui);
}

static inline void multithreading_init(Editor* restrict editor) {
    #ifdef demiwindows
    run_prepare = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (!run_prepare)
        fatal_error(u"event creation failed");

    prepare_complete = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (!run_prepare)
        fatal_error(u"event creation failed");

    thread = CreateThread(NULL, 0, prepare, editor, 0, &thread_id);
    if (!thread)
        fatal_error(u"thread creation failed");
    #elif defined(demilinux)
    // TODO
    #endif
}

static inline void multithreading_destruct() {
    #ifdef demiwindows
        SetEvent(run_prepare);
        WaitForSingleObject(thread, INFINITE);
        CloseHandle(thread);
        CloseHandle(run_prepare);
    #elif defined(demilinux)
    // TODO
    #endif
}