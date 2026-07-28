#include "Fan.h"
#include "ShaderHelper.h"

// ---------------------------------------------------------------------------
// Geometry – 8 vertices, 36 indices (6 faces × 2 triangles × 3 vertices)
// ---------------------------------------------------------------------------

static const GLfloat kPositions[8][3] = {
    {-1.0f, -1.0f, 1.0f},  // V0
    {-1.0f, 1.0f, 1.0f},   // V1
    {1.0f, 1.0f, 1.0f},    // V2
    {1.0f, -1.0f, 1.0f},   // V3
    {-1.0f, -1.0f, -1.0f}, // V4
    {-1.0f, 1.0f, -1.0f},  // V5
    {1.0f, 1.0f, -1.0f},   // V6
    {1.0f, -1.0f, -1.0f},  // V7
};

GLfloat kNormals[8][3]{0.f};

static const GLfloat kColors[8][3] = {
    {0.6f, 0.6f, 0.6f}, // V0 gray shades
    {0.6f, 0.6f, 0.6f}, // V1
    {0.6f, 0.6f, 0.6f}, // V2
    {0.6f, 0.6f, 0.6f}, // V3
    {0.1f, 0.1f, 0.1f}, // V4
    {0.1f, 0.1f, 0.1f}, // V5
    {0.1f, 0.1f, 0.1f}, // V6
    {0.1f, 0.1f, 0.1f}, // V7
};

static const GLushort kIndices[36] = {
    0, 3, 1, 3, 2, 1,
    7, 4, 6, 4, 5, 6,
    4, 0, 5, 0, 1, 5,
    3, 7, 2, 7, 6, 2,
    1, 2, 5, 2, 6, 5,
    3, 0, 7, 0, 4, 7};

static constexpr GLuint ATTRIB_POSITION = 0;
static constexpr GLuint ATTRIB_NORMAL = 1;

// Ctor / Dtor

#ifdef PLATFORM_ANDROID
Fan::Fan(AAssetManager *assetMgr) : mgr(assetMgr)
{
#else
Fan::Fan()
{

#endif
    modelType = FanType;
    transform.TransformInit();
    transform.TransformSetMatrixMode(PROJECTION_MATRIX);
    transform.TransformSetPerspective(glm::radians(60.f), 1.0f, 0.01f, 1000.0f, 0.f);
}

Fan::~Fan()
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
    if (ibo)
    {
        glDeleteBuffers(1, &ibo);
        ibo = 0;
    }
    if (program)
    {
        glDeleteProgram(program);
        program = 0;
    }
}

void Fan::InitNormals()
{
    // calculating the normals (lets do smooth shading..)

    // get the vertices for each triangle
    for (size_t i = 0; i < sizeof(kIndices) / sizeof(kIndices[0]); i += 3)
    {

        GLushort idx0 = kIndices[i];
        GLushort idx1 = kIndices[i + 1];
        GLushort idx2 = kIndices[i + 2];

        GLfloat p0[3] = {kPositions[idx0][0], kPositions[idx0][1], kPositions[idx0][2]};
        GLfloat p1[3] = {kPositions[idx1][0], kPositions[idx1][1], kPositions[idx1][2]};
        GLfloat p2[3] = {kPositions[idx2][0], kPositions[idx2][1], kPositions[idx2][2]};

        // get the edge equations
        GLfloat e0[3] = {p1[0] - p0[0], p1[1] - p0[1], p1[2] - p0[2]};
        GLfloat e1[3] = {p2[0] - p0[0], p2[1] - p0[1], p2[2] - p0[2]};

        glm::vec3 faceNormal = glm::cross(glm::vec3(e0[0], e0[1], e0[2]), glm::vec3(e1[0], e1[1], e1[2]));

        // set the normals accordingly
        kNormals[idx0][0] += faceNormal.x;
        kNormals[idx0][1] += faceNormal.y;
        kNormals[idx0][2] += faceNormal.z;

        kNormals[idx1][0] += faceNormal.x;
        kNormals[idx1][1] += faceNormal.y;
        kNormals[idx1][2] += faceNormal.z;

        kNormals[idx2][0] += faceNormal.x;
        kNormals[idx2][1] += faceNormal.y;
        kNormals[idx2][2] += faceNormal.z;
    }

    // normalize accumulated normals
    for (size_t i = 0; i < sizeof(kNormals) / sizeof(kNormals[0]); ++i)
    {
        glm::vec3 normalizedNormal = glm::normalize(glm::vec3{kNormals[i][0], kNormals[i][1], kNormals[i][2]});
        kNormals[i][0] = normalizedNormal.x;
        kNormals[i][1] = normalizedNormal.y;
        kNormals[i][2] = normalizedNormal.z;
    }
}

// InitModel

void Fan::InitModel()
{
    LOGI("Fan::InitModel");

#ifdef PLATFORM_ANDROID
    program = ShaderHelper::buildProgramFromAssets(
        mgr,
        "shader/FanVertex.glsl",
        "shader/FanFragment.glsl");
#else
    program = ShaderHelper::buildProgramFromFile(
        "FanVertex.glsl",
        "FanFragment.glsl");
#endif

    if (!program)
    {
        LOGE("Fan: failed to build shader program");
        return;
    }
    uMVP = glGetUniformLocation(program, "MODELVIEWPROJECTIONMATRIX");
    uNormalMatrix = glGetUniformLocation(program, "NORMALMATRIX");
    uPartColor = glGetUniformLocation(program, "PARTCOLOR");
    uMV = glGetUniformLocation(program, "MODELVIEWMATRIX");

    // uniform locations for light
    GLint uMatAmb = glGetUniformLocation(program, "MaterialAmbient");
    uMatDiff = glGetUniformLocation(program, "MaterialDiffuse");
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

    InitNormals();

    // VBO: positions sub-region then colours sub-region
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, posSize + norSize, nullptr, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, posSize, kPositions);
    glBufferSubData(GL_ARRAY_BUFFER, posSize, norSize, kNormals);

    // IBO
    glGenBuffers(1, &ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(kIndices), kIndices, GL_STATIC_DRAW);

    // VAO: captures attribute layout + IBO binding
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glEnableVertexAttribArray(ATTRIB_POSITION);
    glEnableVertexAttribArray(ATTRIB_NORMAL);
    glVertexAttribPointer(ATTRIB_POSITION, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);
    glVertexAttribPointer(ATTRIB_NORMAL, 3, GL_FLOAT, GL_FALSE, 0, (void *)posSize);

    // IBO binding is recorded inside the VAO
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);

    // Seal the VAO
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    LOGI("Fan::InitModel done (VAO=%u, VBO=%u, IBO=%u)", vao, vbo, ibo);
}

// Resize

void Fan::Resize(int w, int h)
{
    float aspect = (h > 0) ? (float)w / (float)h : 1.0f;
    transform.TransformSetMatrixMode(PROJECTION_MATRIX);
    transform.TransformSetPerspective(glm::radians(60.f), aspect, 0.01f, 1000.0f, 0.f);
}

// Render

glm::vec3 selectedColor = glm::vec3{255 / 255.f, 210 / 255.f, 42 / 255.f};

void Fan::Render()
{

    if (!program || !vao)
        return;
    glEnable(GL_DEPTH_TEST);
    glUseProgram(program);

    // select MODEL_MATRIX stack, load identity
    transform.TransformSetMatrixMode(MODEL_MATRIX);
    transform.TransformLoadIdentity();

    // world : translate (0, 0.8, -8) -> Rotate (20*, y) <- pull-back + 3D tilt, shared by every part
    transform.TransformTranslate(0.f, 0.8f, -cameraDistance);
    transform.TransformRotate(glm::radians(20.f), 0.f, 1.f, 0.f);

    // base : push -> T(0, -2.6, 0) -> S(1.6, 0.25, 0.8) -> draw(brown) -> pop
    transform.TransformPushMatrix();
    transform.TransformTranslate(0.f, -3.6f, 0.f);

    baseWorldMatrix = *transform.TransformGetModelMatrix();

    transform.TransformScale(1.6f, 0.25f, 0.8f);

    glm::vec3 brown{0.45f, 0.28f, 0.12f};
    // before draw, calculate the normal matrix
    if (baseStatus)
    {
        draw(selectedColor);
    }
    else
    {
        draw(brown);
    }

    transform.TransformPopMatrix();

    // pole : push -> T(0, -1.21, 0) -> S(0.15, 2.53, 0.15) -> draw(gray) -> pop
    transform.TransformPushMatrix();
    transform.TransformTranslate(0.f, -1.21f, 0.f);

    poleWorldMatrix = *transform.TransformGetModelMatrix();

    transform.TransformScale(0.15f, 2.53f, 0.15f);
    glm::vec3 gray{0.55f, 0.55f, 0.58f};
    if (poleStatus)
    {
        draw(selectedColor);
    }
    else
    {
        draw(gray);
    }

    transform.TransformPopMatrix();

    // hub : push -> T(0, 0.2, 0) -> S (0.3, 0.3, 0.3) -> draw(dark gray) -> pop
    transform.TransformPushMatrix();
    transform.TransformTranslate(0.f, 1.2f, 0.f);

    hubWorldMatrix = *transform.TransformGetModelMatrix();

    transform.TransformScale(0.3f, 0.3f, 0.3f);
    glm::vec3 darkGray{0.2f, 0.2f, 0.22f};
    if (hubStatus)
    {
        draw(selectedColor);
    }
    else
    {
        draw(darkGray);
    }

    transform.TransformPopMatrix();

    // for i in 0...3:
    // blade : push -> T(0, 0.2, 0.15 ) -> R(spinAngle + i * 90, Z)
    //  -> T(0, 0.55, 0) -> S(0.22, 0.8, 0.05) -> draw(bladeColor[i]) -> pop

    std::vector<glm::vec3> bladeColor{
        glm::vec3{1.f, 0.f, 0.f},    // red
        glm::vec3{0.f, 1.f, 0.f},    // blue
        glm::vec3{1.0f, 0.65f, 0.f}, // orange
        glm::vec3{0.f, 0.f, 1.f}     // green
    };

    // dragBoost *= 0.98f; // fan decay

    // if(fanOn) spinAngle += kBaseSpeed + dragBoost;

    if (fanOn)
        spinAngle += rotationSpeed;

    if (spinAngle >= 360.f)
    {
        spinAngle -= 360.0f;
    }

    bladesWorldMatrix.clear();

    for (int i = 0; i < 4; ++i)
    {
        transform.TransformPushMatrix();
        transform.TransformTranslate(0.f, 1.2f, 0.15f);
        transform.TransformRotate(glm::radians(spinAngle + i * 90.f), 0.f, 0.f, 1.f);

        transform.TransformTranslate(0.f, 0.55f, 0.f);

        bladesWorldMatrix.push_back(*transform.TransformGetModelMatrix());

        transform.TransformScale(0.22f, 0.8f, 0.05f);
        if (bladesStatus[i])
        {
            draw(selectedColor);
        }
        else
        {
            draw(bladeColor[i]);
        }

        transform.TransformPopMatrix();
    }
    glDisable(GL_DEPTH_TEST);
    // draw(color) = upload mVP (TransformGetModelViewProjectionMatrix)
    // + upload PARTCOLOR (glUniform3fv, one colour per part)
    // + glDrawElements (the ONE shared cube, 36 indices)

    glUseProgram(0);
}


void Fan::draw(glm::vec3 color)
{
    
    glm::mat4 mvp = *transform.TransformGetModelViewProjectionMatrix();

    glm::mat4 mv = *transform.TransformGetModelViewMatrix();
    glm::mat3 normalMatrix = glm::inverse(glm::transpose(glm::mat3(mv)));

    // mvp *= *m_transform.TransformGetModelMatrix();
    if (uMVP >= 0)
    {
        glUniformMatrix4fv(uMVP, 1, GL_FALSE, glm::value_ptr(mvp));
    }

    if (uNormalMatrix >= 0)
    {
        glUniformMatrix3fv(uNormalMatrix, 1, GL_FALSE, glm::value_ptr(normalMatrix));
    }

    if (uMV >= 0)
    {
        glUniformMatrix4fv(uMV, 1, GL_FALSE, glm::value_ptr(mv));
    }

    if (uMatDiff >= 0)
    {
        // glUniform3fv(uPartColor, 1, glm::value_ptr(color));
        glUniform3fv(uMatDiff, 1, glm::value_ptr(color));
    }

    glBindVertexArray(vao);
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_SHORT, (void *)0);
    glBindVertexArray(0);
}

// TouchEventDown - pause / resume rotation (?)

void Fan::TouchEventDown(float x, float y)
{

    // remember the position and reset movedDistance
    lastX = x;
    lastY = y;

    movedDistance = 0;

    lastMoveTime = std::chrono::steady_clock::now();
}

void Fan::TouchEventMove(float x, float y){}

void Fan::ToggleAllFalse()
{
    baseStatus = false;
    poleStatus = false;
    hubStatus = false;
    for (int i = 0; i < 4; ++i)
    {
        bladesStatus[i] = false;
    }
}
void Fan::ToggleBaseStatus()
{
    bool oldStatus = baseStatus;
    ToggleAllFalse();
    baseStatus = !oldStatus;
    LOGI("Base: %s", baseStatus? "Selected" : "Unselected");
}
void Fan::TogglePoleStatus()
{
    bool oldStatus = poleStatus;
    ToggleAllFalse();
    poleStatus = !oldStatus;
    LOGI("Pole: %s", poleStatus? "Selected" : "Unselected");
}
void Fan::ToggleHubStatus()
{
    bool oldStatus = hubStatus;
    ToggleAllFalse();
    hubStatus = !oldStatus;
    LOGI("Hub: %s", hubStatus? "Selected" : "Unselected");
}
void Fan::ToggleBladesStatus(int index)
{
    bool oldStatus = bladesStatus[index];
    ToggleAllFalse();
    bladesStatus[index] = !oldStatus;
    LOGI("Blade %d: %s", index, bladesStatus[index]? "Selected" : "Unselected");
}

void Fan::TouchEventRelease(float x, float y){}