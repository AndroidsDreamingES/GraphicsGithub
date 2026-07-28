
#include "SceneHUD.h"
#include "ShaderHelper.h"


static const glm::vec3 pressedColor{243/255.f, 151/255.f, 25/255.f};
static const glm::vec3 speedColor{45/255.f, 176/255.f, 177/255.f};
static const glm::vec3 zoomColor{121/255.f, 154/255.f, 220/255.f};

static const glm::vec3 indicatorGreen{64/255.f, 217/255.f, 76/255.f};
static const glm::vec3 indicatorGray{115/255.f, 115/255.f, 122/255.f};

static const float PAD_SIDE = 40.0f; // distance from right or left
static const float BUTTON_HEIGHT = 60.0f;
static const float BUTTON_WIDTH = 120.0f;
static const float BUTTON_DIST = 30.0f + BUTTON_HEIGHT;
static const float BUTTON_Y = 70.f;

static const float INDICATOR_PAD_UP = 45.f; 
static const float INDICATOR_UNIT_WIDTH = 10.f;
static const float INDICATOR_UNIT_HEIGHT = 20.f;

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
SceneHUD::SceneHUD(AAssetManager* assetMgr) : mgr(assetMgr)
#else
SceneHUD::SceneHUD()
#endif
{
    modelType = TriangleType;

    transform.TransformInit();
    transform.TransformSetMatrixMode(MODEL_MATRIX);
    transform.TransformOrtho(0, (float)screenW, (float)screenH, 0, -1, 1);

    glGenBuffers(1, &vbo);
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glGenBuffers(1, &vbo_indicator);
    glGenVertexArrays(1, &vao_indicator);
    glBindVertexArray(vao_indicator);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_indicator);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);


    buildGeometry(screenW, screenH);
}

SceneHUD::~SceneHUD(){
    if (vao)     glDeleteVertexArrays(1, &vao);
    if (vbo)     glDeleteBuffers(1, &vbo);
    
    if (program) glDeleteProgram(program);
}

void SceneHUD::buildGeometry(int w, int h){

    screenW = w;
    screenH = h;

    float left = PAD_SIDE;
    float right = left + BUTTON_WIDTH;

    float top = BUTTON_Y;
    float bottom = top + BUTTON_HEIGHT;

    const float speedUp[12] = { // index 0
        left, bottom,
        right, bottom,
        left, top,

        left, top,
        right, bottom,
        right, top
    };

    top = BUTTON_Y + BUTTON_DIST;
    bottom = top + BUTTON_HEIGHT;

    const float speedDown[12] ={ // index 1
        left, bottom,
        right, bottom,
        left, top,

        left, top,
        right, bottom,
        right, top
    };

    left = screenW - PAD_SIDE - BUTTON_WIDTH;
    right = left + BUTTON_WIDTH;

    const float zoomOut[12] ={ // index 4
        left, bottom,
        right, bottom,
        left, top,

        left, top,
        right, bottom,
        right, top
    };

    top = BUTTON_Y;
    bottom = top + BUTTON_HEIGHT;

    const float zoomIn[12] ={ // index 3
        left, bottom,
        right, bottom,
        left, top,

        left, top,
        right, bottom,
        right, top
    };

    memcpy(buttons[0], speedUp, sizeof(speedUp));
    memcpy(buttons[1], speedDown, sizeof(speedDown));
    memcpy(buttons[2], zoomIn, sizeof(zoomIn));
    memcpy(buttons[3], zoomOut, sizeof(zoomOut));

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(buttons), buttons, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // build for the speed indicator
    // top left should be ... screenwidth /2  then - 10*unit width
    // this is a bit of hard coding with the 10 tho
    left = screenW/2 - 10*INDICATOR_UNIT_WIDTH;
    right = left + INDICATOR_UNIT_WIDTH;

    top = screenH - INDICATOR_PAD_UP;
    bottom = top + INDICATOR_UNIT_HEIGHT;

    for(int i = 0  ; i < 20; ++i){
        const float indicator_unit[12] = { 
        left, bottom,
        right, bottom,
        left, top,

        left, top,
        right, bottom,
        right, top
        };

        memcpy(indicatorHUD[i], indicator_unit, sizeof(indicator_unit));

        // increment the left
        left += INDICATOR_UNIT_WIDTH;
        right = left + INDICATOR_UNIT_WIDTH;
    }
    
    glBindBuffer(GL_ARRAY_BUFFER, vbo_indicator);
    glBufferData(GL_ARRAY_BUFFER, sizeof(indicatorHUD), indicatorHUD, GL_DYNAMIC_DRAW);
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

    glBindVertexArray(vao);
    for(int i = 0 ; i < 4; ++i){
        if(uColor >= 0){
            if(i == activeButton){ // if i is the currently pressed button
                glUniform4f(uColor, pressedColor.x, pressedColor.y, pressedColor.z, 1.0f);
            }
            else{
                if(i==0 || i == 1){
                    glUniform4f(uColor, speedColor.x, speedColor.y, speedColor.z, 1.0f);
                }
                else{
                    glUniform4f(uColor, zoomColor.x, zoomColor.y, zoomColor.z, 1.0f);
                }
                
            }
        }
        glDrawArrays(GL_TRIANGLES, i*6, 6);
    }
    glBindVertexArray(0);

    glBindVertexArray(vao_indicator);
    for(int i = 0 ; i < 20; ++i){
        if(uColor >=0){
            if(i < speed){
                glUniform4f(uColor, indicatorGreen.x, indicatorGreen.y, indicatorGreen.z, 1.0f);
            }
            else{
                glUniform4f(uColor, indicatorGray.x, indicatorGray.y, indicatorGray.z, 1.0f);
            }
        }
        glDrawArrays(GL_TRIANGLES, i*6, 6);
    }
    glBindVertexArray(0);

    glUseProgram(0);
}

void SceneHUD::Resize(int w, int h){
    if(w <=0 || h <=0){
        return;
    }

    transform.TransformOrtho(0.0f, (float)w, (float)h, 0.0f, -1.0f, 1.0f);

    buildGeometry(w, h);
}


bool SceneHUD::pointInTriangle(float px, float py, const float* t){
    
    // do a cross product to figure out where point is in relation to edges
    auto side = [](float x, float y, float x1, float y1, float x2, float y2){
            return (x-x2)*(y1-y2) - (x1-x2) * (y-y2);
    };

    float d1 = side(px, py, t[0], t[1], t[2], t[3]);
    float d2 = side(px, py, t[2], t[3], t[4], t[5]);
    float d3 = side(px, py, t[4], t[5], t[0], t[1]);
    bool hasNeg = (d1 < 0) || (d2 < 0) || (d3 < 0);
    bool hasPos = (d1 > 0) || (d2 > 0) || (d3 > 0);
    return !(hasNeg && hasPos);
}

bool SceneHUD::TouchDown(float x, float y){
    for(int i = 0; i < 4; ++i){
        
        // since we have rectangles, we check for both triangles of a button
        bool tri1 = pointInTriangle(x, y, buttons[i]);
        bool tri2 = pointInTriangle(x, y, buttons[i]+6);

        if(tri1 || tri2){
            activeButton = i;
            
            std::string buttonPressed;
            switch(i){
                case 0:
                buttonPressed = "+SPD";
                IncreaseSpeed();
                break;
                
                case 1:
                buttonPressed = "-SPD";
                DecreaaseSpeed();
                break;

                case 2:
                buttonPressed = "+ZOOM";
                ZoomIn();
                break;

                case 3:
                buttonPressed = "-ZOOM";
                ZoomOut();
            }

            LOGI("SceneHUD: %s pressed", buttonPressed.c_str());

            return true;
        }
    }
    return false;
}

void SceneHUD::TouchRelease(){
    activeButton = -1;
}
