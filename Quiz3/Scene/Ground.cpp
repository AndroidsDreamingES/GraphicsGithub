#include "Ground.h"
#include "ShaderHelper.h"

static const float GROUND_HALF = 48.0f;

#ifdef PLATFORM_ANDROID
Ground::Ground(AAssetManager *assetMgr) : mgr(assetMgr)
{
#else
Ground::Ground()
{

#endif
    modelType = GridType;
    transform.TransformInit();
    transform.TransformSetMatrixMode(PROJECTION_MATRIX);
    transform.TransformSetPerspective(glm::radians(60.f), 1.0f, 0.01f, 1000.0f, 0.f);
}

Ground::~Ground()
{

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

void Ground::BuildGround()
{
    const float h = GROUND_HALF;

    const float y = groundY;

    const float quad[] = {
        -h,
        y,
        -h,
        -h,
        y,
        h,
        h,
        y,
        h,
        -h,
        y,
        -h,
        h,
        y,
        h,
        h,
        y,
        -h,
    };

    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quad), quad, GL_STATIC_DRAW);

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void Ground::InitModel()
{

#ifdef PLATFORM_ANDROID
    program = ShaderHelper::buildProgramFromAssets(
        mgr,
        "shader/GroundVertex.glsl",
        "shader/GroundFragment.glsl");
#else
    program = ShaderHelper::buildProgramFromFile(
        "GroundVertex.glsl",
        "GroundFragment.glsl");
#endif

    if (!program)
    {
        LOGE("Ground: shader build failed");
        return;
    }

    BuildGround();

    // uniform locations for light
    GLint uMatAmb = glGetUniformLocation(program, "MaterialAmbient");
    GLint uMatSpec = glGetUniformLocation(program, "MaterialSpecular");

    GLint uShiny = glGetUniformLocation(program, "ShininessFactor");
    GLint uLightAmb = glGetUniformLocation(program, "LightAmbient");
    GLint uLightDiff = glGetUniformLocation(program, "LightDiffuse");
    GLint uLightSpec = glGetUniformLocation(program, "LightSpecular");
    GLint uLightPos = glGetUniformLocation(program, "LightPosition");

    glUseProgram(program);
    if (uMatAmb >= 0)
        glUniform3f(uMatAmb, 0.1f, 0.1f, 0.1f);
    // matDiff will be the colour of the part later
    if (uMatSpec >= 0)
        glUniform3f(uMatSpec, 1.0f, 1.0f, 1.0f);
    if (uShiny >= 0)
        glUniform1f(uShiny, 40.0f);

    if (uLightAmb >= 0)
        glUniform3f(uLightAmb, 1.0f, 1.0f, 1.0f);
    if (uLightDiff >= 0)
        glUniform3f(uLightDiff, 1.0f, 1.0f, 1.0f);
    if (uLightSpec >= 0)
        glUniform3f(uLightSpec, 1.0f, 1.0f, 1.0f);
    if (uLightPos >= 0)
        glUniform3f(uLightPos, 0.0f, 10.0f, 0.0f);
    glUseProgram(0);

    uGroundMVP = glGetUniformLocation(program, "MODELVIEWPROJECTIONMATRIX");
    uGroundMV = glGetUniformLocation(program, "MODELVIEWMATRIX");
    uNormalMatrix = glGetUniformLocation(program, "NORMALMATRIX");
}

void Ground::Render()
{
    if (!program)
    {
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
    if (uGroundMVP >= 0)
    {
        glUniformMatrix4fv(uGroundMVP, 1, GL_FALSE, glm::value_ptr(mvp));
    }

    if (uGroundMV >= 0)
    {
        glUniformMatrix4fv(uGroundMV, 1, GL_FALSE, glm::value_ptr(mv));
    }

    if (uNormalMatrix >= 0)
    {
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