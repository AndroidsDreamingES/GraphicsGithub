#include "Ground.h"
#include "ShaderHelper.h"

// Ground plane: the quad's model matrix is identity, so the vertex position
// IS the world position - a stable anchor for the checkerboard.
static const char* VS_GROUND = R"GLSL(#version 300 es
precision highp float;

layout(location = 0) in vec3 VertexPosition;

uniform mat4 MODELVIEWPROJECTIONMATRIX;

out vec3 WorldPos;

void main() {
    WorldPos    = VertexPosition;
    gl_Position = MODELVIEWPROJECTIONMATRIX * vec4(VertexPosition, 1.0);
}
)GLSL";

// PROCEDURAL checkerboard, no texture involved. floor() quantizes world x/z
// into cells; the parity of (cellX + cellZ) alternates like a chessboard.
// When the Textures chapter arrives, this collapses into one texture() fetch.
static const char* FS_GROUND = R"GLSL(#version 300 es
precision highp float;

in vec3 WorldPos;
out vec4 outColor;

void main() {
    float cellX = floor(WorldPos.x * 1.5);
    float cellZ = floor(WorldPos.z * 1.5);
    float check = mod(cellX + cellZ, 2.0);

    vec3 dark  = vec3(0.16, 0.19, 0.23);
    vec3 light = vec3(0.75, 0.78, 0.82);
    vec3 col   = mix(dark, light, check);

    // Subtle radial fade so the plane's edge doesn't end in a hard line
    float d = clamp(length(WorldPos.xz) / 8.0, 0.0, 1.0);
    col *= 1.0 - 0.45 * d * d;

    outColor = vec4(col, 1.0);
}
)GLSL";


static const float GROUND_HALF = 12.0f;

#ifdef PLATFORM_ANDROID
Ground::Ground(AAssetManager *assetMgr) : mgr(assetMgr)
{
#else
Ground::Ground(){

#endif 
    modelType = GridType;
    transform.TransformInit();
    // transform.TransformSetMatrixMode(PROJECTION_MATRIX);
    glm::perspective(glm::radians(60.0f), 1.0f, 0.01f, 100.0f);
}

Ground::~Ground(){
    
    if (vao)
    {
        glDeleteVertexArrays(1, &vao);
        vao = 0;
    }
    if (vbo)
    {
        glDeleteBuffers(1, &vbo);
        vbo = 0;
    }
    if (program)
    {
        glDeleteProgram(program);
        program = 0;
    }
}

void Ground::BuildGround(){
    const float h = GROUND_HALF;

    const float y = groundY;

    const float quad[] = {
        -h, y, -h,   -h, y,  h,    h, y,  h,
        -h, y, -h,    h, y,  h,    h, y, -h,
    };

    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quad), quad, GL_STATIC_DRAW);

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

}

void Ground::InitModel(){

    program = ShaderHelper::buildProgram(VS_GROUND, FS_GROUND);

    if(!program){
        LOGE("Ground: shader build failed"); 
        return;
    }

    BuildGround();

    uGroundMVP = glGetUniformLocation(program, "MODELVIEWPROJECTIONMATRIX");
}

void Ground::Render(){
    if(!program){
        return;
    }

    glEnable(GL_DEPTH_TEST);

    // ground model = identity, so vertex positions ARE world space
    transform.TransformSetMatrixMode(MODEL_MATRIX);
    transform.TransformLoadIdentity();

    // world : translate (0, 0.8, -8) -> Rotate (20*, y) <- pull-back + 3D tilt, shared by every part
    transform.TransformTranslate(0.f, 0.8f, -8.f); 
    transform.TransformRotate(glm::radians(20.f), 0.f, 1.f, 0.f);

    // MVP assembled by hand: glm projection x (view*model from the stack)
    glm::mat4 mvp = projMatrix * (*transform.TransformGetModelViewMatrix());

    glUseProgram(program);
    if(uGroundMVP >= 0){
        glUniformMatrix4fv(uGroundMVP, 1, GL_FALSE, glm::value_ptr(mvp));
    }
    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);

    glUseProgram(0);
}

void Ground::Resize(int w, int h)
{
    float aspect = (h > 0) ? (float)w / (float)h : 1.0f;
    projMatrix = glm::perspective(glm::radians(60.0f), aspect, 0.01f, 100.0f);
}