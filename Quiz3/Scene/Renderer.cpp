#define LOG_TAG "Renderer"
#include "Renderer.h"
#include "Fan.h"
#include "Scene3D.h"

Renderer::~Renderer() { clearModels(); }

#ifdef PLATFORM_ANDROID
void Renderer::setAssetManager(AAssetManager* mgr) { assetMgr = mgr; }
#endif

bool Renderer::initializeRenderer()
{
    LOGI("Renderer::initializeRenderer");
    createModels();
    initializeModels();
    return true;
}

void Renderer::createModels()
{
    clearModels();
#ifdef PLATFORM_ANDROID
    scene3D = new Scene3D(assetMgr);
    sceneHUD = new SceneHUD(assetMgr);
#else
    // creates one Scene3D
    scene3D = new Scene3D;
    sceneHUD = new SceneHUD;
    
#endif
}

void Renderer::initializeModels()  {scene3D->InitModel(); sceneHUD->InitModel();}
void Renderer::clearModels()       { delete scene3D; delete sceneHUD;}

void Renderer::resize(int w, int h)
{
    screenWidth = w; screenHeight = h;
    glViewport(0, 0, w, h);
    // for (Model* m : models) m->Resize(w, h);
    scene3D->Resize(w, h);
    sceneHUD->Resize(w, h);
    LOGI("Renderer::resize %d x %d", w, h);
}

void Renderer::render()
{
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    //for (Model* m : models) m->Render();
    scene3D->Render();
    sceneHUD->Render();
}

void Renderer::TouchEventDown(float x, float y)    { }//for (Model* m : models) m->TouchEventDown(x,y); }
void Renderer::TouchEventMove(float x, float y)    { }//for (Model* m : models) m->TouchEventMove(x,y); }
void Renderer::TouchEventRelease(float x, float y) { }//for (Model* m : models) m->TouchEventRelease(x,y); }
