#include "MapRenderer.h"
#include "OBJAppearance.h"
#include "OBJLoader.h"
#include "OpenGL.h"


namespace pyr {

    OBJAppearance::OBJAppearance(const string& resource) {
        _resource = resource;
        _map = loadOBJFile(resource);
    }
    
    void OBJAppearance::sendCommand(const std::string& command) {
    }
    
    void OBJAppearance::beginAnimation(const std::string& animation) {
    }
    
    void OBJAppearance::beginAnimationCycle(const std::string& animation) {
    }

    void OBJAppearance::update(float dt) {
        _time += dt;
    }
    
    void OBJAppearance::draw() {
        glPushMatrix();
        glRotatef(_time * 43, 1, 0, 0);
        glRotatef(_time * 47, 0, 1, 0);
        glRotatef(_time * 53, 0, 0, 1);
    
        MapRenderer m;
        _map->handleVisitor(m);
        
        glPopMatrix();
    }

}