#include "Ground.h"
#include "ShaderHelper.h"

// Ground plane: the quad's model matrix is identity, so the vertex position
// IS the world position - a stable anchor for the checkerboard.
static const char* VS_GROUND = R"GLSL(#version 300 es
precision highp float;

layout(location = 0) in vec3 VertexPosition;

uniform mat4 MODELVIEWPROJECTIONMATRIX;
uniform mat4 MODELVIEWMATRIX;
uniform mat3 NORMALMATRIX;

out vec3 WorldPos;
out vec3 eyeCoord;
out vec3 normalCoord;

void main() {

    normalCoord = NORMALMATRIX * vec3(0.0, 1.0, 0.0);
    WorldPos    = VertexPosition;
    eyeCoord = vec3 (MODELVIEWMATRIX * vec4(VertexPosition, 1.0));
    gl_Position = MODELVIEWPROJECTIONMATRIX * vec4(VertexPosition, 1.0);
}
)GLSL";

// PROCEDURAL checkerboard, no texture involved. floor() quantizes world x/z
// into cells; the parity of (cellX + cellZ) alternates like a chessboard.
// When the Textures chapter arrives, this collapses into one texture() fetch.
static const char* FS_GROUND = R"GLSL(#version 300 es
precision highp float;

// Material property
uniform vec3 MaterialAmbient;
uniform vec3 MaterialSpecular;

// Light property
uniform vec3 LightAmbient;
uniform vec3 LightSpecular;
uniform vec3 LightDiffuse;
uniform float ShininessFactor;

uniform vec3 LightPosition;

in vec3 WorldPos;
in vec3 eyeCoord;
in vec3 normalCoord;

vec3 normalizeNormal, normalizeEyeCoord, normalizeLightVec, V, R, ambient, diffuse, specular;
float sIntensity, cosAngle;

out vec4 outColor;

vec3 PhongShading(vec3 MaterialDiffuse){
    normalizeNormal = normalize(normalCoord);
    normalizeEyeCoord = normalize(eyeCoord);
    normalizeLightVec = normalize(LightPosition - eyeCoord);

    // Diffuse Intensity
    cosAngle = max(0.0, dot(normalizeNormal, normalizeLightVec));

    // Specular Intensity 
    V = -normalizeEyeCoord; // viewer's vector
    R = reflect (-normalizeLightVec, normalizeNormal); // reflectivity
    sIntensity = pow(max(0.0, dot(R, V)), ShininessFactor);

    // ADS color as result of Material & Light interaction
    ambient = MaterialAmbient * LightAmbient;
    diffuse = MaterialDiffuse * LightDiffuse;
    specular = MaterialSpecular * LightSpecular;

    return ambient + (cosAngle * diffuse) + (sIntensity * specular);

}

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

    outColor = vec4(PhongShading(col), 1.0);
}
)GLSL";


static const float GROUND_HALF = 48.0f;

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

    // uniform locations for light
    GLint uMatAmb    = glGetUniformLocation(program, "MaterialAmbient");
    GLint uMatSpec   = glGetUniformLocation(program, "MaterialSpecular");

    GLint uShiny     = glGetUniformLocation(program, "ShininessFactor");
    GLint uLightAmb  = glGetUniformLocation(program, "LightAmbient");
    GLint uLightDiff = glGetUniformLocation(program, "LightDiffuse");
    GLint uLightSpec = glGetUniformLocation(program, "LightSpecular");
    GLint uLightPos  = glGetUniformLocation(program, "LightPosition");

    glUseProgram(program);
    if (uMatAmb    >= 0) glUniform3f(uMatAmb,   0.1f,  0.1f,  0.1f);
    // matDiff will be the colour of the part later
    if (uMatSpec   >= 0) glUniform3f(uMatSpec,  1.0f,  1.0f,  1.0f);
    if (uShiny     >= 0) glUniform1f(uShiny, 40.0f);

    if (uLightAmb  >= 0) glUniform3f(uLightAmb,  1.0f, 1.0f, 1.0f);
    if (uLightDiff >= 0) glUniform3f(uLightDiff, 1.0f, 1.0f, 1.0f);
    if (uLightSpec >= 0) glUniform3f(uLightSpec, 1.0f, 1.0f, 1.0f);
    if (uLightPos  >= 0) glUniform3f(uLightPos,  0.0f, 10.0f, 0.0f);
    glUseProgram(0);

    uGroundMVP = glGetUniformLocation(program, "MODELVIEWPROJECTIONMATRIX");
    uGroundMV = glGetUniformLocation(program, "MODELVIEWMATRIX");
    uNormalMatrix = glGetUniformLocation(program, "NORMALMATRIX");
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
    transform.TransformTranslate(0.f, 0.8f, -cameraDistance); 
    transform.TransformRotate(glm::radians(20.f), 0.f, 1.f, 0.f);

    // MVP assembled by hand: glm projection x (view*model from the stack)
    glm::mat4 mvp = projMatrix * (*transform.TransformGetModelViewMatrix());
    glm::mat4 mv = *transform.TransformGetModelViewMatrix();
    glm::mat3 normalMatrix = glm::inverse(glm::transpose(glm::mat3(mv)));

    glUseProgram(program);
    if(uGroundMVP >= 0){
        glUniformMatrix4fv(uGroundMVP, 1, GL_FALSE, glm::value_ptr(mvp));
    }

    if(uGroundMV >= 0){
        glUniformMatrix4fv(uGroundMV, 1, GL_FALSE, glm::value_ptr(mv));
    }

    if(uNormalMatrix >=0){
        glUniformMatrix3fv(uNormalMatrix, 1, GL_FALSE, glm::value_ptr(normalMatrix));
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