#define LOG_TAG "Cube"
#include "Cube.h"
#include "ShaderHelper.h"

// ---------------------------------------------------------------------------
// Geometry – 8 vertices, 36 indices (6 faces × 2 triangles × 3 vertices)
// ---------------------------------------------------------------------------

static const GLfloat kPositions[8][3] = {
    {-1.0f, -1.0f,  1.0f},  // V0
    {-1.0f,  1.0f,  1.0f},  // V1
    { 1.0f,  1.0f,  1.0f},  // V2
    { 1.0f, -1.0f,  1.0f},  // V3
    {-1.0f, -1.0f, -1.0f},  // V4
    {-1.0f,  1.0f, -1.0f},  // V5
    { 1.0f,  1.0f, -1.0f},  // V6
    { 1.0f, -1.0f, -1.0f},  // V7
};

static const GLfloat kColors[8][3] = {
    {0.0f, 0.0f, 0.0f},  // V0 black
    {0.0f, 0.0f, 1.0f},  // V1 blue
    {0.0f, 1.0f, 0.0f},  // V2 green
    {0.0f, 1.0f, 1.0f},  // V3 cyan
    {1.0f, 0.0f, 0.0f},  // V4 red
    {1.0f, 0.0f, 1.0f},  // V5 magenta
    {1.0f, 1.0f, 0.0f},  // V6 yellow
    {1.0f, 1.0f, 1.0f},  // V7 white
};

static const GLushort kIndices[36] = {
    0,3,1, 3,2,1,
    7,4,6, 4,5,6,
    4,0,5, 0,1,5,
    3,7,2, 7,6,2,
    1,2,5, 2,6,5,
    3,0,7, 0,4,7
};

static constexpr GLuint ATTRIB_POSITION = 0;
static constexpr GLuint ATTRIB_COLOR    = 1;

// ---------------------------------------------------------------------------
// Constructor / Destructor
// ---------------------------------------------------------------------------

#ifdef PLATFORM_ANDROID
Cube::Cube(AAssetManager* assetMgr) : mgr(assetMgr) {
#else
Cube::Cube() {
#endif
    modelType  = CubeType;
    projMatrix = mat4Perspective(60.0f, 1.0f, 0.01f, 1000.0f);
}

Cube::~Cube()
{
    if (vao)     { glDeleteVertexArrays(1, &vao);  vao     = 0; }
    if (vbo)     { glDeleteBuffers(1, &vbo);        vbo     = 0; }
    if (ibo)     { glDeleteBuffers(1, &ibo);        ibo     = 0; }
    if (program) { glDeleteProgram(program);        program = 0; }
}

// ---------------------------------------------------------------------------
// InitModel – compile shaders, upload VBO/IBO, create VAO
// ---------------------------------------------------------------------------
void Cube::InitModel()
{
    LOGI("Cube::InitModel");

#ifdef PLATFORM_ANDROID
    program = ShaderHelper::buildProgramFromAssets(
        mgr,
        "shader/VAOVertex.glsl",
        "shader/VAOFragment.glsl");
#else
    program = ShaderHelper::buildProgramFromFile(
        "VAOVertex.glsl",
        "VAOFragment.glsl");
#endif

    if (!program) { LOGE("Cube: failed to build shader program"); return; }
    uMVP = glGetUniformLocation(program, "MODELVIEWPROJECTIONMATRIX");

    // VBO: positions sub-region then colours sub-region
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, posSize + colSize, nullptr, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0,       posSize, kPositions);
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
    glVertexAttribPointer(ATTRIB_POSITION, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glVertexAttribPointer(ATTRIB_COLOR,    3, GL_FLOAT, GL_FALSE, 0, (void*)posSize);

    // IBO binding is recorded inside the VAO
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);

    // Seal the VAO
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER,         0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    LOGI("Cube::InitModel done (VAO=%u, VBO=%u, IBO=%u)", vao, vbo, ibo);
}

// ---------------------------------------------------------------------------
// Resize
// ---------------------------------------------------------------------------
void Cube::Resize(int w, int h)
{
    float aspect = (h > 0) ? (float)w / (float)h : 1.0f;
    projMatrix   = mat4Perspective(60.0f, aspect, 0.01f, 1000.0f);
}

// ---------------------------------------------------------------------------
// Render
// ---------------------------------------------------------------------------
void Cube::Render()
{
    if (!program || !vao) return;
    glEnable(GL_DEPTH_TEST);
    glUseProgram(program);
    renderCube();
}

// ---------------------------------------------------------------------------
// renderCube – build MVP, bind VAO, draw
// ---------------------------------------------------------------------------
void Cube::renderCube()
{
    mat4 model = mat4Identity();
    model = mat4Rotate(model, rotDeg, 1.0f, 1.0f, 1.0f);
    model = mat4Translate(model, 0.0f, 0.0f, -5.0f);
    if (animate) rotDeg += 1.0f;

    mat4 mvp = mat4Mul(projMatrix, model);
    if (uMVP >= 0)
        glUniformMatrix4fv(uMVP, 1, GL_FALSE, mvp.m);

    // VAO encapsulates all attribute + IBO state: no per-draw setup required
    glBindVertexArray(vao);
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_SHORT, (void*)0);
    glBindVertexArray(0);
}

// ---------------------------------------------------------------------------
// TouchEventDown – pause / resume rotation
// ---------------------------------------------------------------------------
void Cube::TouchEventDown(float /*x*/, float /*y*/)
{
    animate = !animate;
    LOGI("Cube: animate = %s", animate ? "true" : "false (paused)");
}
