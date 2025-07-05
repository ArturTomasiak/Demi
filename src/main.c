#include "core/platform_layer.h"
#include "core/editor.h"
#include "core/font.h"
#include "core/gui.h"

const float dark_gray[3] = {19.0f / 255, 19.0f / 255, 19.0f / 255};
const float gray[3] = {129.0f / 255, 133.0f / 255, 137.0f / 255};
const float orange[3] = {240.0f / 255, 128.0f / 255, 0.0f};
const float violet[3] = {127.0f / 255, 0.0f, 1.0f};
const float light_violet[3] = {149.0f / 255, 106.0f / 255, 1.0f};
static const char16_t* const name = u"DemiEditor";

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
    platform_init(&platform, &editor, &font, name);
    editor_init(&editor);
    font_init(&font, editor.uniform_limit, editor.dpi_scale, editor.flags & 1); 
    render_init(editor.width, editor.height, &editor.files[editor.current_file], &font);
    gui_size(&editor.gui, 100, 30, editor.dpi_scale);
    gui_init(&editor.gui, editor.flags & 1);
    multithreading_init(&editor);

    RenderData* data = &editor.data;
    Character*  num  = &font.character[u'9'];
    
    platform_show_window(&platform);
    while (running) {
        glClearColor(dark_gray[0], dark_gray[1], dark_gray[2], 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        #ifdef demidebug
        check_gl_errors();
        #endif

        float text_x = 10;
        float text_y = editor.height - font.size - editor.gui.size.y -  5;

        #ifdef demiwindows
        SetEvent(run_prepare);
        #elif defined(demilinux)
        // TODO
        #endif

        render_gui(&editor);

        #ifdef demiwindows
        WaitForSingleObject(prepare_complete, INFINITE);
        #elif defined(demilinux)
        // TODO
        #endif

        DemiFile* current = &editor.files[editor.current_file];
        render_text_bind(0);
        render_text(data->lines, data->color_map, data->lines_len, text_x, text_y, current->camera_y, editor.height, editor.gui.size.y);
        text_x = text_x * 2 + num->advance * num_count(data->last_line);
        render_text(current->string.buffer, current->string.color_map, current->string.length, text_x, text_y, current->camera_y, editor.height, editor.gui.size.y);

        platform_msg(&running);
        platform_swap_buffers(&platform);
    }

    #ifdef demiwindows
        SetEvent(run_prepare);
        WaitForSingleObject(thread, INFINITE);
        CloseHandle(thread);
        CloseHandle(run_prepare);
    #elif defined(demilinux)
    // TODO
    #endif

    font_destruct(&font);
    editor_destruct(&editor);
    platform_destruct(&platform);
    gui_destruct(&editor.gui);
}