#ifndef PYR_SERVER_ENTITY_H
#define PYR_SERVER_ENTITY_H

#include <algorithm>
#include <string>
#include "ScopedPtr.h"
#include "Types.h"
#include "Utility.h"
#include "VecMath.h"

namespace pyr {

    class Map;
    class ServerEntity;

    class Behavior {
    public:
        virtual void update(ServerEntity* entity, float dt) = 0;
    };

    /// Placeholder entity behavior class, for testing only.
    class DumbBehavior : public Behavior {
    public:
        void update(ServerEntity* entity, float dt);

    private:
        Zeroed<float> angle;
    };

    class ServerEntity {
    public:
        ServerEntity(u16 id, const std::string& appearance, Behavior* behavior = 0) {
            _id = id;
            _appearance = appearance;
            _behavior = behavior;
            _height = 1.9f;
            _width = 0.3f;

        }

        u16 getID() const {
            return _id;
        }

        std::string getAppearance() const {
            return _appearance;
        }

        void setPos(const gmtl::Vec2f& pos) { _pos = pos; }
        gmtl::Vec2f& getPos()               { return _pos; }
        const gmtl::Vec2f& getPos() const   { return _pos; }

        void setVel(const gmtl::Vec2f& vel) { _vel = vel; }
        gmtl::Vec2f& getVel()               { return _vel; }
        const gmtl::Vec2f& getVel() const   { return _vel; }

        float& getHeight()                    { return _height; }
        const float& getHeight()     const    { return _height; }

        float& getWidth()                    { return _width; }
        const float& getWidth()     const    { return _width; }

        void update(float dt, Map* terrain);
        void collideWithOthers(std::vector<ServerEntity*>& entities);

    private:
        u16 _id;
        std::string _appearance;
        ScopedPtr<Behavior> _behavior;

        Vec2f _pos;
        Vec2f _vel;

        float _height;
        float _width;
        
    };
    
}

#endif
