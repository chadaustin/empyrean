#ifndef PYR_INTRO_STATE_H
#define PYR_INTRO_STATE_H


#include "State.h"


namespace pyr {

    class Texture;

    class IntroState : public State {
    public:
        IntroState();

	const char* getName() const {
            return "IntroState";
	}
        
        void draw();
        
        void onKeyPress(SDLKey key, bool down);
        void onMousePress(Uint8 button, bool down, int x, int y);
        void onJoyPress(Uint8 button, bool down);

    private:
        Texture* _image;  
    };
    
}


#endif
