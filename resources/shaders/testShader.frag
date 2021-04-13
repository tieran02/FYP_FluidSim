#version 450 core
out vec4 FragColor;

uniform vec4 ourColor;

in vec3 Normal;
in vec3 Color;

void main()
{
    //single hardcoded directional light for testing
    vec3 LightDirection = normalize(vec3( 0.2f, 1.0f, 0.3f));
    // diffuse shading
    float diff = max(dot(Normal, LightDirection), 0.0);
    float lightIntensity = 1.0;

    FragColor = (ourColor + vec4(Color,1.0f)) * diff * lightIntensity;
}