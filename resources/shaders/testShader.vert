#version 450 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNorm;
layout (location = 2) in vec2 aTex;

uniform mat4 model;
uniform mat4 view;
uniform mat4 perspective;

out vec3 Normal;

void main()
{
    gl_Position = perspective * view * model * vec4(aPos, 1.0);
    Normal = mat3(transpose(inverse(model))) * aNorm;
}