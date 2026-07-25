
#include "SceneHUD.h"
#include "ShaderHelper.h"


static const glm::vec3 pressedColor{243/255.f, 151/255.f, 25/255.f};
static const glm::vec3 speedColor{45/255.f, 176/255.f, 177/255.f};
static const glm::vec3 zoomColor{121/255.f, 154/255.f, 220/255.f};

static const float PAD_SIDE = 130.0f; // distance from right or left
static const float BUTTON_HEIGHT = 20.0f;
static const float BUTTON_WIDTH = 60.0f;
static const float BUTTON_DIST = 100.0f;
static const float BUTTON_Y = 200.f;

// Write super simple shader program
static const char* VS_HUD = R"GLSL(#version 300 es
precision highp float;

layout(location = 0) in vec2 Position;

uniform mat4 PROJECTION;

void main() {
    gl_Position = PROJECTION * vec4(Position, 0.0, 1.0);
}
)GLSL";

static const char* FS_HUD = R"GLSL(#version 300 es
precision mediump float;

uniform vec4 Color;
out vec4 outColor;

void main() {
    outColor = Color;
}
)GLSL";

#ifdef PLATFORM_ANDROID
SceneHUD::SceneHUD(AAssetManager* assetMgr) : mgr(assetMgr);
#else
SceneHUD::SceneHUD()
#endif
{
    modelType = TriangleType;

    transform.TransformInit();
    transform.TransformSetMatrixMode(MODEL_MATRIX);
    transform.TransformOrtho(0, (float)screenW, (float)screenH, 0, -1, 1);

    glGenBuffers(1, &vbo_speed);
    glGenVertexArrays(1, &vao_speed);
    glBindVertexArray(vao_speed);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_speed);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glGenBuffers(1, &vbo_zoom);
    glGenVertexArrays(1, &vao_zoom);
    glBindVertexArray(vao_zoom);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_zoom);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    buildGeometry(screenW, screenH);
}

SceneHUD::~SceneHUD(){
    if (vao_speed)     glDeleteVertexArrays(1, &vao_speed);
    if (vbo_speed)     glDeleteBuffers(1, &vbo_speed);

    if (vao_zoom)     glDeleteVertexArrays(1, &vao_zoom);
    if (vbo_zoom)     glDeleteBuffers(1, &vbo_zoom);

    if (program) glDeleteProgram(program);
}

void SceneHUD::buildGeometry(int w, int h){

    screenW = w;
    screenH = h;

    float left = PAD_SIDE;
    float right = left + BUTTON_WIDTH;

    float top = BUTTON_Y;
    float bottom = top - BUTTON_WIDTH;

    const float speedUp[12] = {
        left, bottom,
        left, top,
        right, bottom,

        left, top,
        right, top,
        right, bottom
    };

    top = BUTTON_Y - BUTTON_DIST;
    bottom = top - BUTTON_WIDTH;

    const float speedDown[12] ={
        left, bottom,
        left, top,
        right, bottom,

        left, top,
        right, top,
        right, bottom
    };

    left = screenW - PAD_SIDE - BUTTON_WIDTH;
    right = left + BUTTON_WIDTH;

    const float zoomOut[12] ={
        left, bottom,
        left, top,
        right, bottom,

        left, top,
        right, top,
        right, bottom
    };

    top = BUTTON_Y;
    bottom = top - BUTTON_WIDTH;

    const float zoomIn[12] ={
        left, bottom,
        left, top,
        right, bottom,

        left, top,
        right, top,
        right, bottom
    };

    memcpy(speedRects[0], speedUp, sizeof(speedUp));
    memcpy(speedRects[1], speedDown, sizeof(speedDown));

    memcpy(zoomRects[0], zoomIn, sizeof(zoomIn));
    memcpy(zoomRects[1], zoomOut, sizeof(zoomOut));

    glBindBuffer(GL_ARRAY_BUFFER, vbo_speed);
    glBufferData(GL_ARRAY_BUFFER, sizeof(speedRects), speedRects, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindBuffer(GL_ARRAY_BUFFER, vbo_zoom);
    glBufferData(GL_ARRAY_BUFFER, sizeof(zoomRects), zoomRects, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

}

void SceneHUD::InitModel(){

    program = ShaderHelper::buildProgram(VS_HUD, FS_HUD);
    if(!program){
        LOGE("SceneHUD: shader build failed");
        return;
    }

    uProjection = glGetUniformLocation(program, "PROJECTION");
    uColor = glGetUniformLocation(program, "Color");

}

void SceneHUD::Render(){

    if(!program){
        return;
    }

    // render the squares or something
    glDisable(GL_DEPTH_TEST);

    glUseProgram(program);

    glm::mat4 orthoMatrix = *transform.TransformGetModelMatrix();

    if(uProjection >=0){
        glUniformMatrix4fv(uProjection, 1, GL_FALSE, glm::value_ptr(orthoMatrix));
    }

    glBindVertexArray(vao_speed);
    for(int i = 0 ; i < 2; ++i){
        if(uColor >= 0){
            if(i == activeButton){ // if i is the currently pressed button
                glUniform4f(uColor, pressedColor.x, pressedColor.y, pressedColor.z, 1.0f);
            }
            else{
                glUniform4f(uColor, speedColor.x, speedColor.y, speedColor.z, 1.0f);
            }
        }
        glDrawArrays(GL_TRIANGLES, i*6, 6);
    }
    glBindVertexArray(0);

    glBindVertexArray(vao_zoom);
    for(int i = 0 ; i < 2; ++i){
        if(uColor >= 0){
            if(i == activeButton){ // if i is the currently pressed button
                glUniform4f(uColor, pressedColor.x, pressedColor.y, pressedColor.z, 1.0f);
            }
            else{
                glUniform4f(uColor, zoomColor.x, zoomColor.y, zoomColor.z, 1.0f);
            }
        }
        glDrawArrays(GL_TRIANGLES, i*6, 6);
    }
    glBindVertexArray(0);

}

void SceneHUD::Resize(int w, int h){
    if(w <=0 || h <=0){
        return;
    }

    transform.TransformOrtho(0.0f, (float)w, (float)h, 0.0f, -1.0f, 1.0f);

    buildGeometry(w, h);
}

bool SceneHUD::TouchDown(float x, float y){
    // ??? TODO
    return false;
}

void SceneHUD::TouchRelease(){
    //????
}
