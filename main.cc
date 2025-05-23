#include "demo.h"

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <emscripten/html5.h>
#endif
#include <string.h> // For strcmp
#include <SDL/SDL.h>
#include <GL/glew.h>

#include <memory>
#include <cstdio>
#include <cstdlib>

namespace GlobalGameParams
{
extern int ShapeCount;
extern int Columns;
extern int CanvasWidth;
extern int CanvasHeight;
}

// Declare the C++ functions that can be called (defined in demo.cc)
extern "C" {
void set_shape_count(int count);
void set_columns(int num);
void set_canvas_dimensions(int width, int height);
}

namespace
{
inline void panic(const char *fmt)
{
    std::printf("%s", fmt);
    std::abort();
}

template<typename... Args>
inline void panic(const char *fmt, const Args &...args)
{
    std::printf(fmt, args...);
    std::abort();
}

std::unique_ptr<Demo> demo;
}

static bool processEvents()
{
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        switch (event.type)
        {
        case SDL_KEYDOWN: {
            const auto &keysym = event.key.keysym;
            switch (keysym.sym)
            {
            case SDLK_SPACE:
                demo->handleKeyPress(Key::Space);
                break;
            default:
                break;
            }
            break;
        }
        case SDL_MOUSEBUTTONDOWN: {
            demo->handleMouseButton(event.button.x, event.button.y);
            break;
        }
        case SDL_QUIT:
            return false;
        }
    }
    return true;
}

int main(int argc, char *argv[])
{
    // Start with default values which might be updated by JS via Module.arguments
    int current_width = GlobalGameParams::CanvasWidth;
    int current_height = GlobalGameParams::CanvasHeight;
    int current_shape_count = GlobalGameParams::ShapeCount;
    int current_column_num = GlobalGameParams::Columns;
    printf("C++ main: argc=%d\n", argc);
    for (int i = 0; i < argc; ++i)
    {
        printf("C++ main: argv[%d]=%s\n", i, argv[i]);
    }

    for (int i = 1; i < argc; ++i)
    {
        if (strcmp(argv[i], "--width") == 0 && i + 1 < argc)
        {
            current_width = atoi(argv[++i]);
        }
        else if (strcmp(argv[i], "--height") == 0 && i + 1 < argc)
        {
            current_height = atoi(argv[++i]);
        }
        else if (strcmp(argv[i], "--shapes") == 0 && i + 1 < argc)
        {
            current_shape_count = atoi(argv[++i]);
        }
        else if (strcmp(argv[i], "--columns") == 0 && i + 1 < argc)
        {
            current_column_num = atoi(argv[++i]);
        }
    }
    printf("C++ main: Effective initial values: width=%d, height=%d, shapes=%d\n", current_width, current_height,
           current_shape_count);

    // Apply parsed/defaulted values by calling the C++ setters
    // This ensures the HTML canvas is resized by emscripten_set_canvas_element_size
    set_canvas_dimensions(current_width, current_height);
    set_shape_count(current_shape_count);
    set_columns(current_column_num);

    // SDL Initialization using the (potentially modified) global parameters
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        panic("Video initialization failed: %s", SDL_GetError());
        return 1;
    }
    const SDL_VideoInfo *info = SDL_GetVideoInfo();
    if (!info)
    {
        panic("Video query failed: %s\n", SDL_GetError());
        return 1;
    }

    const int final_width = GlobalGameParams::CanvasWidth; // Read from globals again after set_canvas_dimensions
    const int final_height = GlobalGameParams::CanvasHeight;
    const int bpp = info->vfmt->BitsPerPixel;
    const Uint32 flags = SDL_OPENGL;

    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 5);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 5);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 5);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    if (!SDL_SetVideoMode(final_width, final_height, bpp, flags))
    {
        panic("Video mode set failed: %s\n", SDL_GetError());
        return 1;
    }

    glewInit();
    demo.reset(new Demo(final_width, final_height)); // Demo constructor will use these dimensions
                                                     // and implicitly the global shape params

#ifdef __EMSCRIPTEN__
    emscripten_request_animation_frame_loop(
        [](double now, void *) -> EM_BOOL {
            static double last_time = 0;
            const auto elapsed = (last_time != 0) ? (now - last_time) / 1000.0 : 0.0;
            last_time = now;
            if (!processEvents())
                return EM_FALSE;
            if (demo)
                demo->renderAndStep(elapsed); // Check if demo is valid
            return EM_TRUE;
        },
        nullptr);
#else
    while (processEvents())
    {
        const auto elapsed = [] {
            static Uint32 last = 0;
            const Uint32 now_ticks = SDL_GetTicks();
            const Uint32 elapsed_ticks = last != 0 ? now_ticks - last : 0;
            last = now_ticks;
            return static_cast<double>(elapsed_ticks) / 1000.0;
        }();
        if (demo)
            demo->renderAndStep(elapsed); // Check if demo is valid
        SDL_GL_SwapBuffers();
    }
    demo.reset();
    SDL_Quit();
#endif
    return 0;
}