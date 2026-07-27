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
}

void Scene3D::SetFanSpeed(float d){
    fan->rotationSpeed = d;
}

void Scene3D::SetCameraDistance(float d){
    cameraDistance = d;
}

bool PickAt(float screenX, float screenY){
    // ??? TODO
    return false;
}

Scene3D::~Scene3D(){
    delete fan;
    delete ground;
}
