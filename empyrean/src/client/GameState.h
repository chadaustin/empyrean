#ifndef PYR_GAME_STATE_H
#define PYR_GAME_STATE_H

#include <vector>
#include "InputManager.h"
#include "PlayerEntity.h"
#include "Scene.h"
#include "State.h"

namespace pyr {

    class Font;

    class GameState : public State {
    public:
        GameState();
        ~GameState();
        
        void draw(float fade);
        void update(float dt);
        
        void onKeyPress(SDLKey key, bool down);
        void onMousePress(Uint8 button, bool down, int x, int y);
        void onMouseMove(int x, int y);
        
    private:
        InputManager _im;
        Input* _inputMLeft;
        Input* _inputMRight;
	Input* _inputLeft;
	Input* _inputRight;
        Input* _inputSpace;
        Input* _inputQuit;
        
        Scene _scene;
        PlayerEntity* _player;
        
        ScopedPtr<Font> _font;
    };

}


#endif
