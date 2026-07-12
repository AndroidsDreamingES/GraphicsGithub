#define LOG_TAG "Grid"
#include "Grid.h"
#include "ShaderHelper.h"
#include <vector>

static constexpr float GRID_X_DIM = 400.0f;
static constexpr float GRID_Z_DIM = 400.0f;
static constexpr int   GRID_X_DIV = 100;
static constexpr int   GRID_Z_DIV = 100;

static constexpr GLuint ATTRIB_POSITION = 0;

// ---------------------------------------------------------------------------
// Constructor / Destructor
// ---------------------------------------------------------------------------

#ifdef PLATFORM_ANDROID
Grid::Grid(AAssetManager* assetMgr) : mgr(assetMgr) {
#else
Grid::Grid() {
#endif
    modelType  = GridType;
    projMatrix = mat4Perspective(60.0f, 1.0f, 0.01f, 1000.0f);
    // Geometry and VAO are set up before the GL context exists on Android –
    // they are set up here so that the VAO is ready before InitModel() is called.
    // Note: on Android the GL context IS active when the constructor is called
    // (construction happens from Renderer::createModels inside initializeRenderer).
    createGrid(GRID_X_DIM, GRID_Z_DIM, GRID_X_DIV, GRID_Z_DIV);
}

Grid::~Grid()
{
    if (vao)     { glDeleteVertexArrays(1, &vao);  vao     = 0; }
    if (vbo)     { glDeleteBuffers(1, &vbo);        vbo     = 0; }
    if (ibo)     { glDeleteBuffers(1, &ibo);        ibo     = 0; }
    if (program) { glDeleteProgram(program);        program = 0; }
}

// ---------------------------------------------------------------------------
// createGrid
//
// Generates a flat XZ grid of line segments.
//
// Lines parallel to Z (constant X):  (GRID_X_DIV + 1) lines
// Lines parallel to X (constant Z):  (GRID_Z_DIV + 1) lines
// Each line = 2 vertices → totalVerts = 2*((xDiv+1)+(zDiv+1))
// Indices are sequential pairs (0,1), (2,3), ... for GL_LINES.
// ---------------------------------------------------------------------------
void Grid::createGrid(float xDim, float zDim, int xDiv, int zDiv)
{
    const int totalVerts = 2 * ((xDiv + 1) + (zDiv + 1));
    const int totalIdx   = totalVerts;  // GL_LINES: each pair is one segment

    std::vector<GLfloat>  verts(totalVerts * 3);
    std::vector<GLushort> indices(totalIdx);

    const float xHalf     = xDim * 0.5f;
    const float zHalf     = zDim * 0.5f;
    const float xInterval = xDim / (float)xDiv;
    const float zInterval = zDim / (float)zDiv;

    int vi = 0;

    // Lines running along Z direction (constant X value)
    for (int j = 0; j <= xDiv; ++j) {
        float x = j * xInterval - xHalf;
        verts[vi++] = x;   verts[vi++] = 0.0f; verts[vi++] =  zHalf;
        verts[vi++] = x;   verts[vi++] = 0.0f; verts[vi++] = -zHalf;
    }
    // Lines running along X direction (constant Z value)
    for (int j = 0; j <= zDiv; ++j) {
        float z = j * zInterval - zHalf;
        verts[vi++] = -xHalf; verts[vi++] = 0.0f; verts[vi++] = z;
        verts[vi++] =  xHalf; verts[vi++] = 0.0f; verts[vi++] = z;
    }

    // Sequential indices so GL_LINES draws each consecutive pair as one segment
    for (int i = 0; i < totalIdx; ++i) indices[i] = static_cast<GLushort>(i);

    indexCount = totalIdx;

    // VBO
    const GLsizeiptr vboSize = static_cast<GLsizeiptr>(totalVerts * 3 * sizeof(GLfloat));
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, vboSize, verts.data(), GL_STATIC_DRAW);

    // IBO
    const GLsizeiptr iboSize = static_cast<GLsizeiptr>(totalIdx * sizeof(GLushort));
    glGenBuffers(1, &ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, iboSize, indices.data(), GL_STATIC_DRAW);

    // VAO: capture VBO attribute layout + IBO binding
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glEnableVertexAttribArray(ATTRIB_POSITION);
    glVertexAttribPointer(ATTRIB_POSITION, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);  // IBO stored in VAO
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER,         0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    LOGI("Grid::createGrid: %d vertices, %d indices, VAO=%u", totalVerts, totalIdx, vao);
}

// ---------------------------------------------------------------------------
// InitModel – compile and link the grid shader program
// ---------------------------------------------------------------------------
void Grid::InitModel()
{
    LOGI("Grid::InitModel");

#ifdef PLATFORM_ANDROID
    program = ShaderHelper::buildProgramFromAssets(
        mgr,
        "shader/GridVertex.glsl",
        "shader/GridFragment.glsl");
#else
    program = ShaderHelper::buildProgramFromFile(
        "GridVertex.glsl",
        "GridFragment.glsl");
#endif

    if (!program) { LOGE("Grid: failed to build shader program"); return; }
    uMVP = glGetUniformLocation(program, "MODELVIEWPROJECTIONMATRIX");

    LOGI("Grid::InitModel done");
}

// ---------------------------------------------------------------------------
// Resize
// ---------------------------------------------------------------------------
void Grid::Resize(int w, int h)
{
    float aspect = (h > 0) ? (float)w / (float)h : 1.0f;
    projMatrix   = mat4Perspective(60.0f, aspect, 0.01f, 1000.0f);
}

// ---------------------------------------------------------------------------
// Render – build MVP, upload, bind VAO, draw GL_LINES
// ---------------------------------------------------------------------------
void Grid::Render()
{
    if (!program || !vao) return;

    glEnable(GL_DEPTH_TEST);
    glUseProgram(program);

    // Tilt 20° on X (looks like a floor), spin on Y, push into scene
    mat4 model = mat4Identity();
    model = mat4Rotate(model, rotDeg, 0.0f, 1.0f, 0.0f);
    model = mat4Rotate(model, 20.0f,  1.0f, 0.0f, 0.0f);
    model = mat4Translate(model, 0.0f, -10.0f, -7.0f);
    rotDeg += 0.3f;

    mat4 mvp = mat4Mul(projMatrix, model);
    if (uMVP >= 0)
        glUniformMatrix4fv(uMVP, 1, GL_FALSE, mvp.m);

    glBindVertexArray(vao);
    glDrawElements(GL_LINES, indexCount, GL_UNSIGNED_SHORT, (void*)0);
    glBindVertexArray(0);
}
