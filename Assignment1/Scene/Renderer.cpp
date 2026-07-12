
// renderer.cpp
// defines the functions needed to render the models

#include "Triangle.h" // ?
#include "Square.h"
#include "Renderer.h"

Renderer &Renderer::Instance() {
    static Renderer instance;
    return instance;
}


Renderer::~Renderer(){};

void Renderer::createModels(){
    clearModels();
#ifdef PLATFORM_ANDROID
    models.push_back(new Triangle(assetMgr));
    models.push_back(new Square(assetMgr));
#else 
    models.push_back(new Triangle());
    models.push_back(new Square());
#endif
}
void Renderer::initializeModels(){
    for(Model * model : models){
        model->InitModel();
    }
}
    
void Renderer::clearModels(){
    for(Model* model : models){
        delete model;
    }
    models.resize(0);
}

#ifdef PLATFORM_ANDROID
void Renderer::setAssetManager(AAssetManager *mgr) {
    assetMgr = mgr;
}
#endif

void Renderer::Resize(int w, int h) {
    
    for(Model * model : models){
         model->Resize(w, h);
    }
   
}

void Renderer::Render() {

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    for(Model * model : models){
        model->Render();
    }
}

bool Renderer::InitializeRenderer(){
#ifdef PLATFORM_ANDROID
    //model = new Triangle(assetMgr);
    createModels();
    initializeModels();
#else
    //model = new Triangle();
    createModels();
    initializeModels();
#endif
    //model->InitModel();
    return true;
}