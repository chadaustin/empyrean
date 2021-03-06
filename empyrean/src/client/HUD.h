#ifndef PYR_HUD_H
#define PYR_HUD_H

#include <gltext.h>
#include "ClientEntity.h"
#include "VecMath.h"

namespace pyr {
    class ClientEntity;
    class Texture;

    class HUD {
    public:
        HUD();
        ~HUD(){}

        void draw(gltext::FontRendererPtr rend, ClientEntityPtr entity);
        void update(float dt);

    private:
        void drawVitalityBar(float vitPerc);
        void drawEtherBar(float ethPerc);
        void calcBar();
        void drawBar(float perc);

        std::vector<Vec2f> _vertsLeft;
        std::vector<Vec2f> _vertsRight;
        Vec2f _vitCenter;
        Vec2f _ethCenter;
        Texture* _thumbnail;
        float _thumbWidth;
        float _thumbHeight;
        float _barRadius;
        float _barLongStraight;
        float _barShortStraight;
        float _barHeight;
        int _numSegsInCurves;
        int _barBufferX;
        int _barBufferY;
        int _barBuffer;
        float _barPathLength;
        bool _vitUseRed;
        float _timeSinceLastVitChange;
    };
};
#endif
