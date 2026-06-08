#version 430 core

layout (location = 0) in vec3  vertex;
layout (location = 1) in vec3  normal;
layout (location = 2) in vec4  color;
layout (location = 3) in vec2  uv;
layout (location = 4) in uvec4 boneIndices;
layout (location = 5) in vec4  boneWeights;

uniform mat4 model;
uniform mat4 matWorldToClip; 

out vec4 vertexColor;

void main()
{
    vertexColor = color;
    gl_Position = matWorldToClip * model * vec4(vertex, 1.0);
}