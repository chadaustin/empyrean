#include "CutSceneState.h"
#include "GameState.h"
#include "GLUtility.h"
#include "IncludePython.h"
#include "PythonInterpreter.h"
#include "Texture.h"


namespace pyr {

    CutSceneState::CutSceneState() {
        using namespace boost::python;

        // initialize Python
        the<PythonInterpreter>();

        std::string file =
            "def prepare(scene):\n"
            "    scene.addImage('cutscene/warrior1.jpg');\n"
            "    scene.addImage('cutscene/ninja1.jpg');\n";
        
        handle<> main_module(borrowed( PyImport_AddModule("__main__") ));
        handle<> main_namespace(borrowed( PyModule_GetDict(main_module.get()) ));

        (handle<>( PyRun_String(
            file.c_str(),
            Py_file_input,
            main_namespace.get(),
            main_namespace.get()) ));

        _cutScene = new CutScene;
        object prepare = object(main_module).attr("prepare");
        prepare(ptr(_cutScene.raw_ptr()));
        
/*
        static const std::string images[] = {
            "warrior1.jpg",
            "Caphriel3.JPG",
            "ninja1.jpg",
            "Robot3.jpg",
            "levelOneColor.jpg",
        };

        
        for (size_t i = 0; i < sizeof(images) / sizeof(*images); ++i) {
            _cutScene->addImage("cutscene/" + images[i]);
        }
*/
    }

    void CutSceneState::update(float dt) {
        _fade = std::min(1.0f, _fade + dt);
    }

    void CutSceneState::draw(float fade) {
        setOrthoProjection(1, 1);

        glDisable(GL_DEPTH_TEST);
        glEnable(GL_TEXTURE_2D);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        if (_current < _cutScene->getImageCount()) {
            glColor4f(1, 1, 1, 1);
            if (_current > 0) {
                _cutScene->getImage(_current - 1)->drawRectangle(0, 0, 1, 1);
            } else {
                glClear(GL_COLOR_BUFFER_BIT);
            }
            glColor4f(1, 1, 1, _fade);
            _cutScene->getImage(_current)->drawRectangle(0, 0, 1, 1);
        }
    }

    void CutSceneState::onKeyPress(SDLKey key, bool down) {
        if (key == SDLK_ESCAPE) {
            invokeTransition<GameState>();
            return;
        }
        if (down) {
            next();
        }
    }

    void CutSceneState::onMousePress(Uint8 button, bool down, int x, int y) {
        if (down) {
            next();
        }
    }

    void CutSceneState::onJoyPress(Uint8 button, bool down) {
        if (down) {
            next();
        }
    }

    void CutSceneState::next() {
        _fade = 0;
        if (++_current >= _cutScene->getImageCount()) {
            invokeTransition<GameState>();
        }
    }

}
