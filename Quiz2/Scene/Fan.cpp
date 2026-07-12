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

// look at this later
static constexpr GLuint ATTRIB_POSITION = 0;
static constexpr GLuint ATTRIB_COLOR = 1;

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
    uPartColor = glGetUniformLocation(program, "PARTCOLOR");

    // VBO: positions sub-region then colours sub-region
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, posSize + colSize, nullptr, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, posSize, kPositions);
    glBufferSubData(GL_ARRAY_BUFFER, posSize, colSize, kColors);

    // IBO
    glGenBuffers(1, &ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(kIndices), kIndices, GL_STATIC_DRAW);

    // VAO: captures attribute layout + IBO binding
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glEnableVertexAttribArray(ATTRIB_POSITION);
    glEnableVertexAttribArray(ATTRIB_COLOR);
    glVertexAttribPointer(ATTRIB_POSITION, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);
    glVertexAttribPointer(ATTRIB_COLOR, 3, GL_FLOAT, GL_FALSE, 0, (void *)posSize);

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

void Fan::Render(){

    if(!program || !vao) return;
    glEnable(GL_DEPTH_TEST);
    glUseProgram(program);

    // select MODEL_MATRIX stack, load identity
    transform.TransformSetMatrixMode(MODEL_MATRIX);
    transform.TransformLoadIdentity();

    // world : translate (0, 0.8, -8) -> Rotate (20*, y) <- pull-back + 3D tilt, shared by every part
    transform.TransformTranslate(0.f, 0.8f, -8.f); 
    transform.TransformRotate(glm::radians(20.f), 0.f, 1.f, 0.f);

    // base : push -> T(0, -2.6, 0) -> S(1.6, 0.25, 0.8) -> draw(brown) -> pop
    transform.TransformPushMatrix();
    transform.TransformTranslate(0.f, -2.6f, 0.f);
    transform.TransformScale(1.6f, 0.25f, 0.8f);
    glm::vec3 brown{0.45f, 0.28f, 0.12f};
    draw(brown);
    transform.TransformPopMatrix();

    // pole : push -> T(0, -1.21, 0) -> S(0.15, 2.53, 0.15) -> draw(gray) -> pop
    transform.TransformPushMatrix();
    transform.TransformTranslate(0.f, -1.21f, 0.f);
    transform.TransformScale(0.15f, 2.53f, 0.15f);
    glm::vec3 gray{0.55f, 0.55f, 0.58f};
    draw(gray);
    transform.TransformPopMatrix();
    
    // hub : push -> T(0, 0.2, 0) -> S (0.3, 0.3, 0.3) -> draw(dark gray) -> pop
    transform.TransformPushMatrix();
    transform.TransformTranslate(0.f, 0.2f, 0.f);
    transform.TransformScale(0.3f, 0.3f, 0.3f); 
    glm::vec3 darkGray{0.2f, 0.2f, 0.22f};
    draw(darkGray);
    transform.TransformPopMatrix();

    // for i in 0...3:
        // blade : push -> T(0, 0.2, 0.15 ) -> R(spinAngle + i * 90, Z)
                    //  -> T(0, 0.55, 0) -> S(0.22, 0.8, 0.05) -> draw(bladeColor[i]) -> pop
    
    std::vector<glm::vec3> bladeColor{
        glm::vec3{1.f, 0.f, 0.f}, // red
        glm::vec3{0.f, 1.f, 0.f}, // blue
        glm::vec3{1.0f, 0.65f, 0.f}, // orange
        glm::vec3{0.f, 0.f, 1.f} // green 
    };

    if(fanOn) spinAngle += kBaseSpeed + dragBoost;
    if(spinAngle >= 360.f){
        spinAngle -= 360.0f;
    }

    for(int i =0; i < 4; ++i){
        transform.TransformPushMatrix();
        transform.TransformTranslate(0.f, 0.2f, 0.15f);
        transform.TransformRotate(glm::radians(spinAngle + i* 90.f), 0.f, 0.f, 1.f);

        transform.TransformTranslate(0.f, 0.55f, 0.f);
        transform.TransformScale(0.22f, 0.8f, 0.05f);
        draw(bladeColor[i]);
    
        transform.TransformPopMatrix();
    }



    // draw(color) = upload mVP (TransformGetModelViewProjectionMatrix)
                // + upload PARTCOLOR (glUniform3fv, one colour per part)
                // + glDrawElements (the ONE shared cube, 36 indices)
}


// render fan ?? do i need this ??

void Fan::draw(glm::vec3 color)
{
    // ????
    glm::mat4 mvp = *transform.TransformGetModelViewProjectionMatrix();

    //mvp *= *m_transform.TransformGetModelMatrix();
    if(uMVP >= 0){
        glUniformMatrix4fv(uMVP, 1, GL_FALSE, glm::value_ptr(mvp));
    }
    
    if(uPartColor >= 0){
        glUniform3fv(uPartColor, 1, glm::value_ptr(color));
    }
    

    glBindVertexArray(vao);
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_SHORT, (void*)0);
    glBindVertexArray(0);
}

// TouchEventDown - pause / resume rotation (?)

void Fan::TouchEventDown(float /*x*/, float /*y*/){
    fanOn = !fanOn;
    LOGI("Fan: spin = %s", fanOn ? "true" : "false (paused)");
}