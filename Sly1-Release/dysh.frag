#version 430 core

in vec4 vertexColor;

out vec4 FragColor;

void main()
{
    FragColor = vec4(1.0, 1.0, 1.0, vertexColor.a);
}