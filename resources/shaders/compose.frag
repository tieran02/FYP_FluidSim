#version 450 core
out vec4 FragColor;
  
in vec2 TexCoords;
in vec4 ModelPos;

uniform vec2 screenSize;
uniform float ProjectFov = 65.0;
uniform mat4 projection;
uniform mat4 view;
uniform vec4 ourColor;

layout(binding = 0) uniform sampler2D depthTexture;
layout(binding = 1) uniform sampler2D normalTexture;

void main()
{ 
	float particleDepth = texture(depthTexture, TexCoords).x;
	vec4 particleNormal = texture(normalTexture, TexCoords);


	//FragColor = vec4(vec3(particleDepth),1.0f);
	if(particleDepth == 0.0f) 
	{
		FragColor = vec4(0.2, 0.3, 0.3, 1.0);
	}
	else
	{
		vec3 normal = particleNormal.xyz;

		//single hardcoded directional light for testing
		vec3 LightDirection = normalize(vec3( 0.2f, 1.0f, 0.3f));
		// diffuse shading
		float diff = max(dot(normal, LightDirection), 0.0);
		float lightIntensity = 1.0;

		//FragColor = vec4(normal, 1.0f);
		FragColor = ourColor * diff * lightIntensity;
	}
}