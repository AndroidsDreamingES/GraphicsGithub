#pragma once

/**
 * Cube.h – 4-VertexArrayObject-Modern
 *
 * Renders a single rotating coloured cube using a VAO.
 * The VAO captures the VBO attribute layout (positions + colours in one
 * sub-buffer) and the IBO binding at InitModel() time.
 * renderCube() only needs glBindVertexArray() before glDrawElements.
 *
 * Demonstrates:
 *   • VAO (glGenVertexArrays) capturing VBO attribute pointers + IBO in one object
 *   • VBO with two sub-regions: positions (offset 0) and colours (offset posSize)
 *   • glDrawElements(GL_TRIANGLES) – solid shaded cube faces
 *
 * Touch pauses / resumes rotation.
 */

#include "Model.h"
#include "Platform.h"
#include "Math3D.h"

#ifdef PLATFORM_ANDROID
#include <android/asset_manager.h>
#endif

class Cube : public Model {
public:
#ifdef PLATFORM_ANDROID
    explicit Cube(AAssetManager* assetMgr);
#else
    Cube();
#endif
    ~Cube() override;

    void InitModel()          override;
    void Render()             override;
    void Resize(int w, int h) override;

    void TouchEventDown(float x, float y) override;

private:
    void renderCube();

#ifdef PLATFORM_ANDROID
    AAssetManager* mgr = nullptr;
#endif

    GLuint program = 0;
    GLuint vao     = 0;
    GLuint vbo     = 0;
    GLuint ibo     = 0;
    GLint  uMVP    = -1;

    // VBO sub-region sizes
    GLsizeiptr posSize = 8 * 3 * sizeof(GLfloat);  // 96 bytes
    GLsizeiptr colSize = 8 * 3 * sizeof(GLfloat);  // 96 bytes

    float rotDeg  = 0.0f;
    bool  animate = true;
    mat4  projMatrix;
};
