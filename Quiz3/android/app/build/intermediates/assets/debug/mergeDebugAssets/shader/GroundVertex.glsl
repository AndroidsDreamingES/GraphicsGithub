// Ground plane: the quad's model matrix is identity, so the vertex position
// IS the world position - a stable anchor for the checkerboard.

#version 300 es
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