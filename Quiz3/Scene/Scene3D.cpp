#include "Scene3D.h"

// it does the rendering n all now

Scene3D::Scene3D(){

#ifdef PLATFORM_ANDROID 
    fan = new Fan(assetMgr);
    ground = new Ground(assetMgr);
#else
    fan = new Fan();
    ground = new Ground();
#endif

}

void Scene3D::InitModel(){
    fan->InitModel();
    ground->InitModel();
}

void Scene3D::Render(){
    ground->Render();
    fan->Render();
}

void Scene3D::Resize(int w, int h){
    ground->Resize(w, h);
    fan->Resize(w, h);

    screenWidth = w;
    screenHeight = h;
}

void Scene3D::SetFanSpeed(float d){
    fan->rotationSpeed = d;
}

void Scene3D::SetCameraDistance(float d){
    cameraDistance = d;
    fan->cameraDistance = cameraDistance;
    ground->cameraDistance = cameraDistance;
}

bool Scene3D::hits(glm::mat4 const& worldMatrix, const glm::vec3& rayOrigin, const glm::vec3& rayDir, float radius, float& outT){
    
    glm::vec3 center {worldMatrix * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f)};
    glm::vec3 toCenter = center - rayOrigin;
    float t = glm::dot(toCenter, rayDir);
    if(t < 0.0f) return false; // behind the camera

    glm::vec3 closestPoint = rayOrigin + rayDir*t; // t is like a scale
    float dist = glm::length(center - closestPoint);
    if (dist > radius) return false;

    outT = t;
    return true;

}

bool Scene3D::hitsBlade(glm::mat4 const& worldMatrix, const glm::vec3& rayOrigin, const glm::vec3 rayDir, float radius, float halfLength, float& outT){
    const int SAMPLES = 4;
    float bestDist = FLT_MAX;
    float bestT = -1.0f;

    for(int i = 0 ; i < SAMPLES; ++i){
        float s = -halfLength + (2.0f * halfLength) * i / (SAMPLES - 1);
        glm::vec3 samplePoint{worldMatrix * glm::vec4(0.0f, s, 0.0f, 1.0f)};
        
        glm::vec3 toSample = samplePoint - rayOrigin;
        float t = glm::dot(toSample, rayDir);
        if(t < 0.0f) continue; // behind camera

        glm::vec3 closestPoint = rayOrigin + rayDir * t;
        float dist = glm::length(samplePoint - closestPoint);
        if(dist < bestDist) {bestDist = dist; bestT = t;}
    }

    if(bestT < 0.0f || bestDist > radius) return false;
    outT = bestT;
    return true;
}

static const float BASE_PICK_RADIUS = 1.6f;

static const float POLE_PICK_RADIUS = 0.25f;
static const float POLE_HALF_LENGTH = 2.53f;

static const float HUB_PICK_RADIUS = 0.6f;

static const float BLADE_PICK_RADIUS = 0.22f;
static const float BLADE_HALF_LENGTH = 0.8f;

bool Scene3D::PickAt(float screenX, float screenY){
    
    int viewport[4] = {0, 0, screenWidth, screenHeight};
    glm::mat4 viewMatrix = *fan->transform.TransformGetViewMatrix();
    glm::mat4 projMatrix = *fan->transform.TransformGetProjectionMatrix();

    float winY = (float)screenHeight - screenY; // origin top left

    float nx, ny, nz, fx, fy, fz;
    fan->transform.TransformUnproject(screenX, winY, 0.0f, &viewMatrix, &projMatrix, viewport, &nx, &ny, &nz);
    fan->transform.TransformUnproject(screenX, winY, 1.0f, &viewMatrix, &projMatrix, viewport, &fx, &fy, &fz);

    glm::vec3 rayOrigin(nx, ny, nz);
    glm::vec3 rayDir = glm::normalize(glm::vec3(fx, fy, fz) - rayOrigin);

    float bestT = FLT_MAX;
    enum class Hit{None, Base, Pole, Hub, Blade} best = Hit::None;
    int bestBladeIndex = -1;

    const float DEPTH_BIAS = 0.15f; // for the hub vs blades

    float t{};
    if(hits(fan->GetBaseWorldMatrix(), rayOrigin, rayDir, BASE_PICK_RADIUS, t) && t < bestT){bestT = t; best = Hit::Base;}
    
    if(hitsBlade(fan->GetPoleWorldMatrix(), rayOrigin, rayDir, POLE_PICK_RADIUS, POLE_HALF_LENGTH, t) && t < bestT){
        bestT = t; best = Hit::Pole;
    
    }
    
    if(hits(fan->GetHubWorldMatrix(), rayOrigin, rayDir, HUB_PICK_RADIUS, t) && t < bestT){bestT = t; best = Hit::Hub;}

    for(int i = 0 ; i < 4; ++i){
        if(hitsBlade(fan->GetBladesWorldMatrix(i), rayOrigin, rayDir, BLADE_PICK_RADIUS, BLADE_HALF_LENGTH, t) && (t < bestT-DEPTH_BIAS)){
            bestT = t; best = Hit::Blade; bestBladeIndex = i;
        }
    }
    
    switch(best){
        case Hit::Base:
            fan->ToggleBaseStatus();
            return true;
        case Hit::Pole:
            fan->TogglePoleStatus();
            return true;
        case Hit::Hub:
            fan->ToggleHubStatus();
            return true;
        case Hit::Blade:
            fan->ToggleBladesStatus(bestBladeIndex);
            return true;
        default:
            return false; // missed everything
    }

}

Scene3D::~Scene3D(){
    delete fan;
    delete ground;
}
