#include "Entity.h"
#include "GLUtility.h"
#include "Scene.h"
#include "Renderer.h"
#include "Texture.h"


namespace pyr {

    PYR_DEFINE_SINGLETON(Scene)

    Scene::Scene() {
        _focus = 0;
        _backdrop = Texture::create("images/stars.tga");
    }
    
    Scene::~Scene() {
        EntityMap::iterator itr = _entities.begin();
        for (; itr != _entities.end(); ++itr) {
            delete itr->second;
        }
    }
    
    void Scene::draw() {
        // Nominal viewport is 12 meters wide and 9 meters high.
        const float width = 12.0f;
        const float height = 9.0f;
        setOrthoProjection(width, height, true);
        
        float focusX = 0;
        float focusY = 0;
        if (_focus) {
            focusX = _focus->getPos()[0];
            focusY = _focus->getPos()[1];
        }

        glDisable(GL_DEPTH_TEST);
        glDisable(GL_BLEND);

        glClear(GL_DEPTH_BUFFER_BIT);

        // draw background
        glEnable(GL_TEXTURE_2D);
        _backdrop->bind();
        glBegin(GL_TRIANGLE_FAN);
        glTexCoord2f(0 + focusX / width, 0 + focusY / height); glVertex2f(0,     0);
        glTexCoord2f(0 + focusX / width, 1 + focusY / height); glVertex2f(0,     height);
        glTexCoord2f(1 + focusX / width, 1 + focusY / height); glVertex2f(width, height);
        glTexCoord2f(1 + focusX / width, 0 + focusY / height); glVertex2f(width, 0);
        glEnd();
        
        glTranslatef(width / 2 - focusX, height / 2 - focusY, 0);

        EntityMap::iterator itr = _entities.begin();
        for (; itr != _entities.end(); ++itr) {
            Entity* e = itr->second;
            glPushMatrix();
            glTranslate(e->getPos());
            e->draw();
            glPopMatrix();
        }
    }
    
    void Scene::update(float dt) {
        EntityMap::iterator itr = _entities.begin();
        for (; itr != _entities.end(); ++itr) {
            itr->second->update(dt);
        }
    }
    
    void Scene::addEntity(u16 id, Entity* entity) {
        PYR_ASSERT(_entities.count(id) == 0, "Two entities have same ID");
        _entities[id] = entity;
    }
    
    void Scene::removeEntity(u16 id) {
        if (_focus && getEntity(id) == _focus) {
            _focus = 0;
        }
        _entities.erase(id);
    }
    
    Entity* Scene::getEntity(u16 id) const {
        EntityMap::const_iterator i = _entities.find(id);
        return (i == _entities.end() ? 0 : i->second);
    }
    
    void Scene::setFocus(u16 id) {
        _focus = getEntity(id);
    }
    
    Entity* Scene::getFocus() const {
        return _focus;
    }
    
}
