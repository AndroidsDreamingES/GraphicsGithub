// Scene/Ground.h - a big quad, Phong-lit, checkerboard computed in the fragment
// shader from world-space position. no texture, no vbo beyond one quad

#pragma once

#include "Model.h"
#include "Transform.h"
#include <glm/gtc/type_ptr.hpp>
#include <glm/glm.hpp>
#ifdef PLATFORM_ANDROID
#include <android/asset_manager.h>
#endif

class Ground : public Model
{
public:
    Transform transform;

#ifdef PLATFORM_ANDROID
    explicit Ground(AAssetManager *assetMgr);
#else
    Ground();
#endif

    ~Ground() override;

    void InitModel() override;
    void Render() override;
    void Resize(int w, int h) override;

    void BuildGround();

    GLuint program = 0;
    GLuint vao = 0;
    GLuint vbo = 0;

    GLint uGroundMVP = -1;
    GLint uGroundMV = -1;
    GLint uNormalMatrix = -1;

    glm::mat4 projMatrix{1.0f};
    float cameraDistance = 8.0f;

    float groundY = -3.6f;

    #ifdef PLATFORM_ANDROID
    AAssetManager* mgr = nullptr;
    #endif

};