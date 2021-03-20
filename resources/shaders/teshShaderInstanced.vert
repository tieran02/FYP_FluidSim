#version 450 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNorm;
layout (location = 2) in vec2 aTex;

struct Position {
    float x, y, z, w;
};

layout(std430, binding = 0) buffer ParticleData
{
    Position positions[];
};

uniform mat4 view;
uniform mat4 perspective;

out vec3 Normal;

void main()
{
    vec3 instancePos = vec3(positions[gl_InstanceID].x,positions[gl_InstanceID].y,positions[gl_InstanceID].z);
    gl_Position = perspective * view * vec4(instancePos + aPos, 1.0);
    Normal = aNorm;
}