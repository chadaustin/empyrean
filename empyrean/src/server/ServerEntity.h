#ifndef PYR_SERVER_ENTITY_H
#define PYR_SERVER_ENTITY_H

#include <algorithm>
#include <string>
#include "Database.h"
#include "Entity.h"
#include "ScopedPtr.h"
#include "ServerAppearance.h"
#include "Types.h"
#include "Utility.h"
#include "VecMath.h"
#include "GameStatistics.h"

namespace pyr {

    class Appearance;
    class Behavior;
    class Map;

    class ServerEntity : public Entity {
    protected:
        ~ServerEntity() { }

    public:
        ServerEntity(u16 id, BehaviorPtr behavior, ServerAppearance* appearance,
                     GameStatisticsPtr gameStats)
        : Entity(behavior, appearance) {
            _id = id;
            _gameStats = gameStats;
        }

        u16 getID() const {
            return _id;
        }

        ServerAppearancePtr getServerAppearance() const {
            // Since we created the base class, we know what
            // type of appearance it has.
            return checked_cast<ServerAppearance*>(getAppearance().get());
        }

        GameStatisticsPtr getGameStats() const {
            return _gameStats;
        }

    private:
        u16 _id;
        GameStatisticsPtr _gameStats;
    };
    PYR_REF_PTR(ServerEntity);
    
}

#endif
