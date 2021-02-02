#version 450 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNorm;
layout (location = 2) in vec2 aTex;
layout (location = 3) in mat4 aModel;

out vec4 vertexColor;

uniform mat4 view;
uniform mat4 perspective;

void main()
{
    gl_Position = perspective * view * aModel * vec4(aPos, 1.0);
}