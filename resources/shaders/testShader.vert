#version 450 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNorm;
layout (location = 2) in vec2 aTex;

out vec4 vertexColor;

void main()
{
    gl_Position = vec4(aPos, 1.0);
}