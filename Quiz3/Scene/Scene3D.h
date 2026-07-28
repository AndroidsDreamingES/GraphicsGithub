// Scene/Scene3D.h - owns your android programming quiz 2 fan plus the new Ground
// forwards resize() to both, applies the shared Phong light + adjusteable camera distance
// before drawing either one, and owns PickAt

#pragma once

#include "Model.h"
#include "Fan.h"
#include "Ground.h"

class Scene3D : public Model {
    public:

    Scene3D();
    ~Scene3D();

    void InitModel() override;
    void Render() override;
    void Resize(int w, int h) override;

    void SetCameraDistance(float d); // called by SceneHUD's zoom buttons
    void SetFanSpeed(float d); 

    bool hits(glm::mat4 const& worldMatrix, const glm::vec3& rayOrigin, const glm::vec3& rayDir,
              float radius, float& outT);
    bool hitsBlade(glm::mat4 const& worldMatrix, const glm::vec3& rayOrigin, const glm::vec3 rayDir,
              float radius, float halfLength, float& outT);

    bool PickAt(float screenX, float screenY); // 3d picking

    private:
    Fan* fan = nullptr;
    Ground* ground = nullptr;
    float cameraDistance = 8.0f;

    int screenWidth = 800, screenHeight = 600;
};