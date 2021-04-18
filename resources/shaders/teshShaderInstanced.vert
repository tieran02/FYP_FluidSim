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

layout(std430, binding = 1) buffer ParticlePressures
{
    float pressures[];
};


uniform mat4 view;
uniform mat4 perspective;

out vec3 Normal;
out vec3 Color;
out vec4 clipspacePos;

void main()
{
    vec3 instancePos = vec3(positions[gl_InstanceID].x,positions[gl_InstanceID].y,positions[gl_InstanceID].z);
    clipspacePos = perspective * view * vec4(instancePos + aPos, 1.0);

    gl_Position = perspective * view * vec4(instancePos + aPos, 1.0);
    Normal = aNorm;

    float maxDensity = 2000.0;
    //float maxDensity = 600.0;
    Color = vec3(pressures[gl_InstanceID] / maxDensity,0.0,0.0);
}