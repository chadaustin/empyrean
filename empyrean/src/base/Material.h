#ifndef PYR_MATERIAL_H
#define PYR_MATERIAL_H


#include <string>
#include "RefCounted.h"
#include "VecMath.h"


namespace pyr {

    class Material : public RefCounted {
    protected:
        ~Material() { }

    public:
        Vec3f diffuse;
        std::string texture;
    };
    typedef RefPtr<Material> MaterialPtr;

}


#endif
