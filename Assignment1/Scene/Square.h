#pragma once

/**
 * Square.h
 THIS IS A SQUARE. if you dont know what a square is go back to kindergarten !!!!!!!!!
 this is a yellow square. . .

 this declares the square functions (child of model)
 */

#include "Platform.h"
#include "Model.h"
#include "ShaderHelper.h"

class Square : public Model{
public:

#ifdef PLATFORM_ANDROID
    explicit Square(AAssetManager* assetMgr);
#else

    Square();
#endif

    ~Square() override;

    void InitModel() override;
    void Render() override;
    void Resize(int w, int h) override;

private:
#ifdef PLATFORM_ANDROID
    AAssetManager* mgr = nullptr;
#endif

    GLuint programID    = 0;
    GLuint vao          = 0;
    GLuint vboPos       = 0;
    GLuint vboColor     = 0;
    GLint  uRadianAngle = -1;
    float  degree       = 0.0f;
};