
// Scene/SceneHUD.h - four buttons (+SPD, -SPD, +ZOOM, -ZOOM) and a 20-block
// speed bar, all drawn in one orthographic pass.
// owns no 3D state itself - it reads / writes shared speed & zoom values the
// Renderer passes to Scene3D

#pragma once

#include "Model.h"
#include "Transform.h"
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

class SceneHUD : public Model{
    public:

    #ifdef PLATFORM_ANDROID
    explicit SceneHUD(AAssetManager *assetMgr);
    #else
    SceneHUD();
    #endif

    ~SceneHUD() override;

    void InitModel() override;
    void Render() override;
    void Resize(int w, int h) override;

    bool TouchDown(float x, float y); // HUD hit-testing
    void TouchRelease();

    float CurrentSpeed() const {return speed;}

    GLuint program = 0;
    GLuint vao_speed = 0;
    GLuint vbo_speed = 0;

    GLuint vao_zoom = 0;
    GLuint vbo_zoom = 0;

    Transform transform;

    GLint uProjection = -1;
    GLint uColor = -1;

    private:

    float speedRects[2][12] = {};
    float zoomRects[2][12] = {};

    float speed = 8.0f; // 0..20 (??) maybe the range
    int activeButton = -1; // which button (if any) is held

    int screenW = 800, screenH = 600;

    void buildGeometry(int w, int h);
};