#include <memory>
#include <stdexcept>
#include <string>
#include <SDL.h>
#include "Application.h"
#include "Configuration.h"
#include "Error.h"
#include "InputManager.h"
#include "Log.h"
#include "OpenGL.h"
#include "Platform.h"
#include "Profiler.h"
#include "SDLUtility.h"
#include "NSPRUtility.h"

namespace pyr {

    void runClient(int argc, char* argv[]) {
        // Why bother?
        //PYR_PROFILE_BLOCK("main");

        try {
            the<Log>().open(getStartDirectory(argc, argv) + "/client.log");
        }
        catch (const LogFileOpenFailure&) {
            // Could not open log file.  That's okay, defer any problems until later.
        }

        setStartDirectory(argc, argv);

        try {
            the<Configuration>().load();
        }
        catch (const ConfigurationError& e) {
            PYR_LOG() << "Could not load client configuration: " << e.what()
                      << "  That's okay, using defaults.";
        }

        PYR_LOG() << "Initializing SDL...";
        initializeSDL(SDL_INIT_NOPARACHUTE | SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_JOYSTICK);

        // define our minimum requirements for the GL window
        SDL_GL_SetAttribute(SDL_GL_RED_SIZE,     5);
        SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE,   5);
        SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE,    5);
        SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE,   16);
        SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

        int mode = SDL_OPENGL | SDL_ANYFORMAT;
        if (the<Configuration>().fullscreen) {
            mode |= SDL_FULLSCREEN;
        }

        /**
         * @note SDL does silly things when the created window is larger
         * than the desktop.
         */
        const int width  = the<Configuration>().screenWidth;
        const int height = the<Configuration>().screenHeight;
        PYR_LOG() << "Setting video mode...";
        if (!SDL_SetVideoMode(width, height, 32, mode)) {
            throwSDLError("Setting video mode failed");
        }

#ifdef PYR_USE_EXTGL
        if (extgl_Initialize() != 0) {
            throw std::runtime_error("extgl_Initialize() failed");
        }
#endif

        PYR_LOG() << "GL_VENDOR: " << glGetString(GL_VENDOR);
        PYR_LOG() << "GL_RENDERER: " << glGetString(GL_RENDERER);
        PYR_LOG() << "GL_VERSION: " << glGetString(GL_VERSION);
        PYR_LOG() << "GL_EXTENSIONS: " << glGetString(GL_EXTENSIONS);

        SDL_WM_SetCaption("Empyrean", 0);
        SDL_ShowCursor(SDL_DISABLE);

        SDL_Joystick *joystick=NULL;
        if(SDL_NumJoysticks() > 0) {
            
            SDL_JoystickEventState(SDL_ENABLE);
            joystick = SDL_JoystickOpen(0);

            PYR_LOG() << "Creating Joystick with name of " << SDL_JoystickName(0);
            PYR_LOG() << "Num Axes " << SDL_JoystickNumAxes(joystick);
            PYR_LOG() << "Num Buttons " << SDL_JoystickNumButtons(joystick);
            PYR_LOG() << "Num Hats " << SDL_JoystickNumHats(joystick);
        }

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

                    case SDL_JOYAXISMOTION:
                        switch(event.jaxis.axis) {
                            case 0:
                                // X Axis left stick (Left = negative, Right = positive)
                                app.onJoyMove(0,event.jaxis.value);
                                break;
                            case 1:
                                // Y Axis left stick (Forward = negative, Back = positive)
                                app.onJoyMove(1,event.jaxis.value);
                                break;
                            case 2:
                                // Throttle (low = positive, high = negative)
                                break;
                            case 3:
                                // X Axis right stick (Left = negative, Right = positive)
                                break;
                            case 4:
                                // Y Axis right stick (Forward = negative, Back = positive)
                                break;
                        }
                        break;

                    case SDL_JOYHATMOTION:
                        if(event.jhat.value & SDL_HAT_CENTERED) {
                            // Doesn't work
                        }

                        if(event.jhat.value & SDL_HAT_RIGHT) {
                            // Right
                        }
                        
                        if(event.jhat.value & SDL_HAT_LEFT ) {
                            // Left
                        }

                        break;

                    case SDL_JOYBUTTONDOWN:
                        switch(event.jbutton.button) {
                            case 0:
                                // A
                                app.onJoyPress(0,true);
                                break;
                            case 1:
                                // B
                                
                                break;
                            case 2:
                                // C
                                
                                break;
                            case 3:
                                // X
                                
                                break;
                            case 4:
                                // Y
                                
                                break;
                            case 5:
                                // Z
                                
                                break;
                            case 6:
                                // L1
                                
                                break;
                            case 7:
                                // R1
                                
                                break;
                            case 8:
                                // Start
                                app.onJoyPress(8,true);
                                break;
                            case 9:
                                // L2
                                
                                break;
                            case 10:
                                // R2
                                
                                break;
                        }
                        break;

                    case SDL_JOYBUTTONUP:
                        switch(event.jbutton.button) {
                            case 0:
                                // A
                                app.onJoyPress(0,false);
                                break;
                            case 1:
                                // B
                                
                                break;
                            case 2:
                                // C
                                
                                break;
                            case 3:
                                // X
                                
                                break;
                            case 4:
                                // Y
                                
                                break;
                            case 5:
                                // Z
                                
                                break;
                            case 6:
                                // L1
                                
                                break;
                            case 7:
                                // R1
                                
                                break;
                            case 8:
                                // Start
                                app.onJoyPress(8,false);
                                break;
                            case 9:
                                // L2
                                
                                break;
                            case 10:
                                // R2
                                
                                break;
                        }
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
                    PYR_PROFILE_BLOCK("glSwapBuffers");
                    SDL_GL_SwapBuffers();
                }
            }
            last_time = now;
            
            the<Profiler>().nextFrame();
        }

        try {
            // Perhaps this should be saved right after changes, in case
            // the program crashes.
            the<Configuration>().save();
        }
        catch (const ConfigurationError& e) {
            // Display a warning.
            warning("Saving client configuration failed: " + std::string(e.what()));
        }
    }

}


/// main() needs to be defined with argc and argv so SDL works right.
int main(int argc, char* argv[]) {
    PYR_BEGIN_EXCEPTION_TRAP()

        pyr::runClient(argc, argv);
        return EXIT_SUCCESS;

    PYR_END_EXCEPTION_TRAP()
    return EXIT_FAILURE;
}


#if defined(WIN32) && !defined(_CONSOLE)

    #ifdef __CYGWIN__
    extern "C" int __argc;
    extern "C" char* __argv[];
    #endif

    int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
        return main(__argc, __argv);
    }

#endif
