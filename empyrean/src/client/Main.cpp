#include <memory>
#include <stdexcept>
#include <string>
#include <stdlib.h>
#include <time.h>
#include <SDL.h>

#ifdef PYR_USE_EXTGL
#include "extgl.h"
#endif

#include "Application.h"
#include "Configuration.h"
#include "Error.h"
#include "InputManager.h"
#include "Profiler.h"
#include "SDLUtility.h"
#include "NSPRUtility.h"

namespace pyr {

    void runClient() {
        PYR_PROFILE_BLOCK("main");

        // initialize the random number generator in case we ever
        // decide to use rand()
        srand(time(0));
        
        initializeSDL(SDL_INIT_NOPARACHUTE | SDL_INIT_VIDEO | SDL_INIT_TIMER);

        const SDL_VideoInfo* info = SDL_GetVideoInfo();
        if (!info) {
            throwSDLError("Retrieving video information failed");
        }
        
        // define our minimum requirements for the GL window
        SDL_GL_SetAttribute(SDL_GL_RED_SIZE,     5);
        SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE,   5);
        SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE,    5);
        SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE,   16);
        SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

        int mode = SDL_OPENGL;
        if (Configuration::instance().isFullscreen()) {
            mode |= SDL_FULLSCREEN;
        }
        
        const int width  = Configuration::instance().getScreenWidth();
        const int height = Configuration::instance().getScreenHeight();
        const int bpp    = info->vfmt->BitsPerPixel;
        if (!SDL_SetVideoMode(width, height, bpp, mode)) {
            throwSDLError("Setting video mode failed");
        }

#ifdef PYR_USE_EXTGL
        if (extgl_Initialize() != 0) {
            throw std::runtime_error("extgl_Initialize() failed");
        }
#endif

        SDL_WM_SetCaption("Empyrean", 0);
        SDL_ShowCursor(SDL_DISABLE);
        
        pyr::Application& app = pyr::Application::instance();
        
        // notify the app and the input manager of the window size
        app.resize(width, height);
        
        float last_time = getNow();
        while (!app.shouldQuit()) {
            bool should_quit = false;

            SDL_Event event;
            int result = SDL_PollEvent(&event);
            while (result == 1) {
                switch (event.type) {
                    case SDL_VIDEORESIZE:
                        app.resize(event.resize.w, event.resize.h);
                        break;

                    case SDL_KEYDOWN:
                    case SDL_KEYUP:
                        app.onKeyPress(
                            event.key.keysym.sym,
                            event.key.state == SDL_PRESSED);
                        break;

                    case SDL_MOUSEBUTTONDOWN:
                    case SDL_MOUSEBUTTONUP:
                        app.onMousePress(
                            event.button.button,
                            event.button.state == SDL_PRESSED,
                            event.button.x,
                            event.button.y);
                        break;

                    case SDL_MOUSEMOTION:
                        app.onMouseMove(event.motion.x, event.motion.y);
                        break;

                    case SDL_QUIT:
                        should_quit = true;
                        break;
                }

                result = SDL_PollEvent(&event);
            }

            // error or SDL_QUIT message
            if (result < 0 || should_quit) {
                break;
            }

            float now = getNow();

            // ignore wraparound
            if (now >= last_time) {
                float dt = now - last_time;

                app.update(dt);
                app.draw();
                {
                    PYR_PROFILE_BLOCK("PageFlip");
                    SDL_GL_SwapBuffers();
                }
            }
            last_time = now;
        }
    }

}


#if defined(WIN32)

    #include <windows.h>

    inline void setStartDirectory() {
        // set the current path to where the executable resides
        char filename[MAX_PATH];
        GetModuleFileName(GetModuleHandle(0), filename, MAX_PATH);

        // remove the basename
        char* backslash = strrchr(filename, '\\');
        if (backslash) {
            *backslash = 0;
            SetCurrentDirectory(filename);
        }
    }
    
#endif


/// main() needs to be defined with argc and argv so SDL works right.
int main(int /*argc*/, char* /*argv*/[]) {

#ifdef WIN32
    setStartDirectory();
#endif

    PYR_BEGIN_EXCEPTION_TRAP()

        pyr::runClient();
        pyr::Profiler::dump();
        return EXIT_SUCCESS;
    
    PYR_END_EXCEPTION_TRAP()
    
    pyr::Profiler::dump();
    return EXIT_FAILURE;
}


#if defined(WIN32) && !defined(_CONSOLE)

    int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
        #ifdef CYGWIN
            int __argc = 1;
            char* __argv[] = {"client"};
        #endif
        return main(__argc, __argv);
    }
    
#endif
