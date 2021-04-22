#version 450 core
out float thickness;


in vec3 Normal;
in vec3 Color;
in vec4 clipspacePos;

void main()
{
    thickness = 0.175;
}