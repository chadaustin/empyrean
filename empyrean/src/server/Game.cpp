#include "Connection.h"
#include "Game.h"
#include "PacketTypes.h"
#include "ServerEntity.h"


namespace pyr {

    Game::Game(const std::string& name, const std::string& password) {
        _name = name;
        _password = password;
    }
    
    Game::~Game() {
        clearConnections();
    
        for (size_t i = 0; i < _entities.size(); ++i) {
            delete _entities[i];
        }
        _entities.clear();
    }
    
    const std::string& Game::getName() const {
        return _name;
    }
    
    const std::string& Game::getPassword() const {
        return _password;
    }

    void Game::update(float dt) {
        ConnectionHolder::update();

        for (size_t i = 0; i < _entities.size(); ++i) {
            _entities[i]->update(dt);
        }

        for (size_t i = 0; i < _entities.size(); ++i) {
            ServerEntity* e = _entities[i];
            sendAll(new EntityUpdatedPacket(
                        e->getID(), e->getPos(), e->getVel()));
        }
    }

    Game::ConnectionData* Game::getData(Connection* c) {
        return static_cast<ConnectionData*>(c->getOpaque());
    }
    
    void Game::addEntity(ServerEntity* entity) {
        _entities.push_back(entity);
    }
    
    void Game::removeEntity(ServerEntity* entity) {
        for (size_t i = 0; i < _entities.size(); ++i) {
            if (_entities[i] == entity) {
                _entities.erase(_entities.begin() + i);
                return;
            }
        }
    }
    
    void Game::connectionAdded(Connection* connection) {
        ServerEntity* entity = new ServerEntity(_idGenerator.reserve());
        sendAll(new EntityAddedPacket(
                    entity->getID(),
                    entity->getAppearance()));
                    
        // set connection-specific data
        ConnectionData* cd = new ConnectionData;
        cd->playerEntity = entity;
        connection->setOpaque(cd);

        // add packet handlers
        connection->definePacketHandler(this, &Game::handleSetVelocity);
        
        addEntity(entity);
        
        // send all existing entities to the new connection
        for (size_t i = 0; i < _entities.size(); ++i) {
            connection->sendPacket(new EntityAddedPacket(
                _entities[i]->getID(),
                _entities[i]->getAppearance()));
        }

        connection->sendPacket(new SetPlayerPacket(entity->getID()));
    }
    
    void Game::connectionRemoved(Connection* connection) {
        ConnectionData* cd = getData(connection);
        PYR_ASSERT(cd, "connectionRemoved() called before connectionAdded()");

        sendAll(new EntityRemovedPacket(cd->playerEntity->getID()));
        connection->sendPacket(new EntityRemovedPacket(
                                   cd->playerEntity->getID()));

        _idGenerator.release(cd->playerEntity->getID());
        removeEntity(cd->playerEntity);
        delete cd->playerEntity;
        delete cd;

        connection->clearHandlers();
        connection->setOpaque(0);
    }

    void Game::handleSetVelocity(Connection* c, SetVelocityPacket* p) {
        ConnectionData* cd = getData(c);
        cd->playerEntity->setVel(p->vel());
    }
    
}
