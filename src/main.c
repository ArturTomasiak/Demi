#include "core/platform_layer.h"
#include "core/editor.h"
#include "core/font.h"

static const char16_t* const name = u"DemiEditor";
static const double background[3] = {19.0f / 255, 19.0f / 255, 19.0f / 255};

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
    render_init(editor.width, editor.height, &font);

    platform_show_window(&platform);
    _Bool running = 1;
    while (running) {
        if (editor.flags & 0b10)
            editor_resize(&editor, &font);
        glClearColor(background[0], background[1], background[2], 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        render_text_bind(&font, 0);
        render_text(&font, &editor.files[editor.current_file].string, 0, editor.height - font.size, editor.dpi_scale);

        platform_msg(&running);
        platform_swap_buffers(&platform);
    }

    font_destruct(&font);
    editor_destruct(&editor);
    platform_destruct(&platform);
}