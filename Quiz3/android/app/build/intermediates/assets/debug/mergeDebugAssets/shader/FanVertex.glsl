#version 300 es
// Vertex information
layout(location = 0) in vec4 VertexPosition;
layout(location = 1) in vec3 Normal;

// Model View Project matrix....
uniform mat4 MODELVIEWPROJECTIONMATRIX;
uniform mat4 MODELVIEWMATRIX;
uniform mat3 NORMALMATRIX;

// do i need the rest ?? oh i guess so

out vec3 normalCoord;
out vec3 eyeCoord;

void main(){
    normalCoord = NORMALMATRIX * Normal;
    eyeCoord = vec3 (MODELVIEWMATRIX * VertexPosition);
    gl_Position = MODELVIEWPROJECTIONMATRIX * VertexPosition;
}

