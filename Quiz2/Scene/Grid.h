#pragma once

/**
 * Grid.h – 4-VertexArrayObject-Modern
 *
 * Renders a flat XZ grid of white lines using a VAO.
 * The grid geometry is generated procedurally in createGrid() at construction.
 * The VAO captures the VBO vertex layout + IBO binding at construction time.
 * Render() only needs glBindVertexArray() – demonstrates the same VAO benefit
 * as Cube.h but for a line-based mesh.
 *
 * Visual: a 400×400 world-unit ground grid, tilted 20° on X, spinning on Y.
 */

#include "Model.h"
#include "Platform.h"
#include "Math3D.h"

#ifdef PLATFORM_ANDROID
#include <android/asset_manager.h>
#endif

class Grid : public Model {
public:
#ifdef PLATFORM_ANDROID
    explicit Grid(AAssetManager* assetMgr);
#else
    Grid();
#endif
    ~Grid() override;

    void InitModel()          override;
    void Render()             override;
    void Resize(int w, int h) override;
    void TouchEventDown(float x, float y) override {}

private:
    void createGrid(float xDim, float zDim, int xDiv, int zDiv);

#ifdef PLATFORM_ANDROID
    AAssetManager* mgr = nullptr;
#endif

    GLuint program    = 0;
    GLuint vao        = 0;
    GLuint vbo        = 0;
    GLuint ibo        = 0;
    GLint  uMVP       = -1;
    int    indexCount = 0;

    float  rotDeg  = 0.0f;
    mat4   projMatrix;
};
