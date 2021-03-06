#ifndef PYR_GL_UTILITY_H
#define PYR_GL_UTILITY_H


#include "OpenGL.h"
#include "VecMath.h"


namespace pyr {

    void setOrthoProjection(float width, float height, bool cartesian = false);
    const char* getErrorString(GLenum error);
    void loadShader(GLhandleARB shader, const char* filename);

    void checkOpenGLErrors();

    inline void glTranslate(const Vec2f& vec) {
        glTranslatef(vec[0], vec[1], 0);
    }

    inline void glVertex(const Vec2f& vec) {
        glVertex2f(vec[0], vec[1]);
    }

    inline void glVertex(const Vec3f& vec) {
        glVertex3f(vec[0], vec[1], vec[2]);
    }
    
    inline void glNormal(const Vec3f& vec) {
        glNormal3f(vec[0], vec[1], vec[2]);
    }
    
    inline void glTexCoord(const Vec2f& vec) {
        glTexCoord2f(vec[0], vec[1]);
    }

    inline void glColor(const Vec3f& vec) {
        glColor3f(vec[0], vec[1], vec[2]);
    }

    inline void glColor(const Vec4f& vec) {
        glColor4f(vec[0], vec[1], vec[2], vec[3]);
    }

}


#endif
