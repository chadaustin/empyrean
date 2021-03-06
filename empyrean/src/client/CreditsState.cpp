#include <stdexcept>
#include "CreditsState.h"
#include "GLUtility.h"
#include "OpenGL.h"


namespace pyr {

    class MenuState;

    PYR_REGISTER_STATE_FACTORY(CreditsState)

    CreditsState::CreditsState() {
        _renderer = gltext::CreateRenderer(
            gltext::TEXTURE,
            gltext::OpenFont("fonts/Vera.ttf", 24));
        if (!_renderer) {
            throw std::runtime_error("Creating font renderer failed");
        }
    }

    void CreditsState::draw() {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        setOrthoProjection(640, 480);

        glClear(GL_COLOR_BUFFER_BIT);
        _renderer->render("Credits to be determined.");
    }


    void CreditsState::onKeyPress(SDLKey key, bool down) {
        if (down) {
            invokeTransition<MenuState>();
        }
    }

    void CreditsState::onMousePress(Uint8 button, bool down, int x, int y) {
        if (down) {
            invokeTransition<MenuState>();
        }
    }

}
