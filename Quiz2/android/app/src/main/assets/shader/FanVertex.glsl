#version 300 es

layout (location = 0) in vec4 VertexPosition;
layout (location = 1) in vec4 VertexColor;

out vec4 Color; 

uniform mat4 MODELVIEWPROJECTIONMATRIX;
uniform vec3 PARTCOLOR;

void main(){

    gl_Position = MODELVIEWPROJECTIONMATRIX * VertexPosition;
    Color = vec4((PARTCOLOR.x * VertexColor.x),(PARTCOLOR.y * VertexColor.y), (PARTCOLOR.z * VertexColor.z), 1.0);

}