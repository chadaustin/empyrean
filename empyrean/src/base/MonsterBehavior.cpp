#include "Collider.h"
#include "Constants.h"
#include "Entity.h"
#include "MonsterBehavior.h"


namespace pyr {

    float distance(const Entity* a, const Entity* b) {
        return length(a->getPos() - b->getPos());
    }

    void MonsterBehavior::update(Entity* entity, float dt, const Environment& env) {
        // Customized behavior.
        const Entity* closest = 0;
        for (size_t i = 0; i < env.entities.size(); ++i) {
            if (env.entities[i] != entity) {
                if (closest) {
                    if (distance(env.entities[i], entity) < distance(closest, entity)) {
                        closest = env.entities[i];
                    }
                } else {
                    closest = env.entities[i];
                }
            }
        }

        if (closest) {
            float xvel = 0;
            if (closest->getPos()[0] > entity->getPos()[0] + 1.0) {
                xvel = 1;
            }
			else if (closest->getPos()[0] > entity->getPos()[0]) {
				xvel = -1;
			}
            else if (closest->getPos()[0] < entity->getPos()[0] - 1.0) {
                xvel = -1;
            }
			else if (closest->getPos()[0] < entity->getPos()[0]) {
				xvel = 1;
			}
            entity->getVel()[0] = xvel;
        }

        // Default physical behavior.
        Vec2f& pos = entity->getPos();
        Vec2f& vel = entity->getVel();
        const Vec2f origPos = pos;

        pos += vel * dt;
        vel[1] += -constants::GRAVITY * dt;           // gravity

        const float mu = 0.8f;
        const float acc = -mu * constants::GRAVITY;

        if(vel[0] > 0) {
            vel[0] += acc * dt;
            if(vel[0] < 0) {
                vel[0] = 0;
            }
        }
        
        if(vel[0] < 0) {
            vel[0] -= acc * dt;
            if(vel[0] > 0) {
                vel[0] = 0;
            }
        }                        
       
        if (vel[1] < constants::TERMINAL_VELOCITY) { // terminal velocity
            vel[1] = constants::TERMINAL_VELOCITY;
        }

    }

}
