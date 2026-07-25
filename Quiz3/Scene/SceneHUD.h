
// Scene/SceneHUD.h - four buttons (+SPD, -SPD, +ZOOM, -ZOOM) and a 20-block
// speed bar, all drawn in one orthographic pass.
// owns no 3D state itself - it reads / writes shared speed & zoom values the
// Renderer passes to Scene3D

#pragma once

#include "Model.h"

class SceneHUD : public Model{
    public:
    void InitModel() override;
    void Render() override;
    void Resize(int w, int h) override;

    bool TouchDown(float x, float y); // HUD hit-testing
    void TouchRelease();

    float CurrentSpeed() const {return speed;}

    private:
    float speed = 8.0f; // 0..20 (??) maybe the range
    int activeButton = -1; // which button (if any) is held
};