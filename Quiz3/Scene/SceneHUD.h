
// Scene/SceneHUD.h - four buttons (+SPD, -SPD, +ZOOM, -ZOOM) and a 20-block
// speed bar, all drawn in one orthographic pass.
// owns no 3D state itself - it reads / writes shared speed & zoom values the
// Renderer passes to Scene3D

#pragma once

#include "Model.h"
#include "Transform.h"
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#ifdef PLATFORM_ANDROID
#include <android/asset_manager.h>
#endif

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
    float CurrentCamDist() const {return cameraDist;}
    float ActiveButton() const {return activeButton;}

    void IncreaseSpeed() {if(speed < 20){++speed;}}
    void DecreaaseSpeed() {if(speed >0){--speed;}}

    void ZoomIn(){if(cameraDist>3){--cameraDist;}}
    void ZoomOut(){if(cameraDist<20){++cameraDist;}}

    GLuint program = 0;
    GLuint vao = 0;
    GLuint vbo = 0;

    GLuint vao_indicator = 0;
    GLuint vbo_indicator = 0;

    Transform transform;

    GLint uProjection = -1;
    GLint uColor = -1;

    private:

    float buttons[4][12] = {};
    float indicatorHUD[20][12] = {};

    float speed = 8.0f; // 0..20 (??) maybe the range oh yep
    float cameraDist = 8.0f;
    int activeButton = -1; // which button (if any) is held

    int screenW = 800, screenH = 600;

    void buildGeometry(int w, int h);

    bool pointInTriangle(float px, float py, const float* t);

    #ifdef PLATFORM_ANDROID
    AAssetManager* mgr = nullptr;
    #endif
};