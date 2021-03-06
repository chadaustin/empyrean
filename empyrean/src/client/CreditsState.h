#ifndef PYR_CREDITS_STATE_H
#define PYR_CREDITS_STATE_H


#include <gltext.h>
#include "State.h"


namespace pyr {

    class CreditsState : public State {
    public:
        CreditsState();

	const char* getName() const {
            return "CreditsState";
	}
    
        void draw();
        void onKeyPress(SDLKey key, bool down);
        void onMousePress(Uint8 button, bool down, int x, int y);

    private:
        gltext::FontRendererPtr _renderer;
    };

}


#endif
