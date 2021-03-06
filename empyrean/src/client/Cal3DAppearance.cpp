#include "Cal3DAppearance.h"
#include "ModelRenderer.h"


namespace pyr {

    Cal3DAppearance::Cal3DAppearance(const std::string& resource) {
        _resource = resource;
        _model = new Model(_resource);
    }

    void Cal3DAppearance::sendCommand(const std::string& command) {
        if (command == "Face Left") {
            _direction = -90;
        } else if (command == "Face Right") {
            _direction = 90;
        }
    }

    void Cal3DAppearance::beginAnimation(const std::string& animation) {
        _model->executeAction(animation, 1.0f, 0.1f, 0.1f);
    }

    void Cal3DAppearance::beginAnimationCycle(const std::string& animation) {
        _model->blendCycle(animation, 1.0f, 0.1f);
    }

    void Cal3DAppearance::update(float dt, const Environment& env) {
        _model->update(dt);
    }

    void Cal3DAppearance::draw() {
        glPushMatrix();
        glEnable(GL_DEPTH_TEST);
        glRotatef(_direction, 0, 1, 0);
        ModelRenderer().draw(*_model);
        glDisable(GL_DEPTH_TEST);
        glPopMatrix();
    }

}
