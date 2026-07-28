#pragma once

#include "Transform.h"
#include "Model.h"
#include "Platform.h"
#include "Math3D.h"
#include <chrono>
#include <glm/gtc/type_ptr.hpp>
#include <glm/glm.hpp>


#ifdef PLATFORM_ANDROID
#include <android/asset_manager.h>
#endif

class Fan : public Model {

public: 
Transform transform; // provided matrix stack (model/view/projection)
float spinAngle = 0.0f; // current blade angle (degrees)
float rotationSpeed= 8.0f;
float cameraDistance = 8.0f; 

#ifdef PLATFORM_ANDROID
    explicit Fan(AAssetManager* assetMgr);
#else 
    Fan();
#endif
    ~Fan() override;

    void InitModel() override;
    void Render() override;
    void Resize (int w, int h) override;

    void TouchEventDown(float x, float y) override;
    void TouchEventMove(float x, float y) override;
    void TouchEventRelease(float x, float y) override;

    const glm::mat4& GetBaseWorldMatrix(){return baseWorldMatrix;};
    const glm::mat4& GetPoleWorldMatrix(){return poleWorldMatrix;};
    const glm::mat4& GetHubWorldMatrix(){return hubWorldMatrix;};
    const glm::mat4& GetBladesWorldMatrix(int index){return bladesWorldMatrix[index];};

    void ToggleAllFalse();
    void ToggleBaseStatus();
    void TogglePoleStatus();
    void ToggleHubStatus();
    void ToggleBladesStatus(int index);

private:
    void draw(glm::vec3 color);

    void InitNormals();

#ifdef PLATFORM_ANDROID
    AAssetManager* mgr = nullptr;
#endif

    // to look at ...
    GLuint program = 0;
    GLuint vao = 0;
    GLuint vbo = 0;
    GLuint ibo = 0;
    GLint uMVP = -1;
    GLint uPartColor = -1;
    GLint uNormalMatrix = -1;
    GLint uMV = -1;
    GLint uMatDiff = -1;

    glm::mat4 baseWorldMatrix;
    glm::mat4 poleWorldMatrix;
    glm::mat4 hubWorldMatrix;
    std::vector<glm::mat4> bladesWorldMatrix;


    bool baseStatus{false}; // if true means selected
    bool poleStatus{false};
    bool hubStatus{false};
    std::vector<bool> bladesStatus{false, false, false, false};

    // VBO sub-region sizes
    GLsizeiptr posSize = 8*3*sizeof(GLfloat); // 8 vertices of 3 GLfloat (96 bytes)
    GLsizeiptr norSize = 8*3*sizeof(GLfloat); 


};