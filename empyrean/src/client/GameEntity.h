#ifndef PYR_GAME_ENTITY_H
#define PYR_GAME_ENTITY_H

#include <map>
#include "ClientEntity.h"
#include "Collider.h"
#include "Utility.h"
#include "Types.h"

namespace pyr {

    class Model;
    class Renderer;
    class InputManager;
    class Input;
    class Weapon;
    class Armor;

    /** Represents a local player.
     *
     * I'm not all that happy that I have to pass a renderer
     * here.  It feels sloppy.  Maybe it's just me.
     *
     * On the plus side, this means that entities have individual
     * rendering settings implicitly.  Good juju.  I'll have to
     * tweak the renderer a bit if we go with this, though, since
     * every CellShadeRenderer has its own shade texture, as well
     * as other miscellaneous weirdness. (as is customary for any
     * code written by yours truly)
     */
    class GameEntity : public ClientEntity {
    public:

        enum Animation {
            ATTACKING,
            JUMPING,
            JUMPSTART,
            STANDING,
            WALKING     
        };

        GameEntity(Model* model, Renderer* renderer);
        ~GameEntity();

        void draw();
        void update(float dt, const Map* terrain);
        
        typedef std::map<u16, ClientEntity*> EntityMap;
        void collideWithOthers(EntityMap entities);

        void getVitalityUpdate(int& current, int& max);
        void getEtherUpdate(int& current, int& max);
        void decrVitality(int decr);
        void incrVitality(int incr);
        void decrEther(int decr);
        void incrEther(int incr);
 
        float& getHeight()                    { return _height; }
        const float& getHeight()     const    { return _height; }

        float& getWidth()                    { return _width; }
        const float& getWidth()     const    { return _width; }
        
        u16& getJumping()                    { return _jumping; }
        const u16& getJumping()     const    { return _jumping; }

        bool jump();
        bool attack();
        
    private:
        typedef void (GameEntity::*StateHandler)(float dt);
        StateHandler _state;

        void changeState(StateHandler* newstate);

        void phaseOutAnimation(Animation name);
        void correctDirection(float xvel);

        void startAttackState();
        void updateAttackState(float dt);

        void startJumpState();
        void updateJumpState(float dt);

        void startJumpStartState();
        void updateJumpStartState(float dt);

        void startStandState();
        void updateStandState(float dt);

        void startWalkState();
        void updateWalkState(float dt);

        // These are aggregates.  We don't clean up.
        Model* _model;
        Renderer* _renderer;
        
        /// Data from last collision detection test.  Used for debug drawing.
        CollisionData _lastCD;

        // Jumping animation information
        bool _jumpStart;
        float _jumpStartTime;

        // Attacking animation information
        bool _attackStart;
        float _attackingStartTime;
        float _origDirection;

        // Character Information
        float _direction;
        Zeroed<u16> _jumping;
        int _maxVitality;
        int _currentVitality;
        int _maxEther;
        int _currentEther;
        Weapon* _meleeWeapon;
        Weapon* _rangeWeapon;
        Armor* _armor;
        
        // Height and Width of Entity used for collision
        Zeroed<float> _height; 
        Zeroed<float> _width; 

        
        
    };

}

#endif
