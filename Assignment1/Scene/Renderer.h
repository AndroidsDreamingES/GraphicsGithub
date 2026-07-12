#pragma once
#include "Model.h"
#include <vector>

#ifdef PLATFORM_ANDROID
#include <android/asset_manager.h>
#endif

class Renderer{
public:
    static Renderer& Instance();

    // singleton!!
    Renderer(const Renderer&) = delete;
    Renderer& operator=(const Renderer&) = delete;

#ifdef PLATFORM_ANDROID
    void setAssetManager(AAssetManager* mgr);
#endif

    bool InitializeRenderer();
    void Resize(int w, int h);
    void Render();

private:
    Renderer() = default;
    ~Renderer();

    void createModels();
    void initializeModels();
    void clearModels();

#ifdef PLATFORM_ANDROID
    AAssetManager* assetMgr = nullptr;
#endif

    //Model* model = nullptr; // exactly one model for now - Task 3 changes this to a collection

    std::vector<Model*> models; // replaces the single Model* model

};