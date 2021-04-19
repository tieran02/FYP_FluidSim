#version 450 core
out vec4 FragColor;
  
in vec2 TexCoords;
in vec4 ModelPos;

uniform vec2 screenSize;
uniform vec3 eyePosition;
uniform vec2 clipPositionToEye;
uniform float ProjectFov = 65.0;
uniform float SpecularPower = 32.0;
uniform float SpecularIntensity = 0.9;
uniform mat4 projection;
uniform mat4 view;
uniform vec4 ourColor;

layout(binding = 0) uniform sampler2D depthTexture;
layout(binding = 1) uniform sampler2D normalTexture;

vec3 viewportToEyeSpace(vec2 vCoord, float vEyeZ)
{
	// find position at z=1 plane
	vec2 UV = (vCoord*2.0 - vec2(1.0))* clipPositionToEye;

	return vec3(-UV * vEyeZ, vEyeZ);
}

vec3 eyespacePos(vec2 pos, float depth) {
	pos = (pos - vec2(0.5f)) * 2.0f;
	return(depth * vec3(-pos.x * projection[0][0], -pos.y / projection[1][1], 1.0f));
}

vec3 WorldPosFromDepth(float depth) {
    float z = depth * 2.0 - 1.0;

    vec4 clipSpacePosition = vec4(TexCoords * 2.0 - 1.0, z, 1.0);
    vec4 viewSpacePosition = inverse(projection) * clipSpacePosition;

    // Perspective division
    viewSpacePosition /= viewSpacePosition.w;

    vec4 worldSpacePosition = inverse(view) * viewSpacePosition;

    return worldSpacePosition.xyz;
}

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
		vec3 ViewPos = eyespacePos(TexCoords, particleDepth);
		vec3 WorldPos = WorldPosFromDepth(particleDepth);

		vec3 LightDirection = normalize(vec3( 0.0f, 1.0f, 0.0f));
		vec4 lightColor = vec4(1.0, 1.0, 1.0, 1.0);

		vec4 SpecularColor = vec4(0, 0, 0, 0);
		vec3 VertexToEye = normalize(eyePosition - WorldPos);
		vec3 LightReflect = normalize(reflect(LightDirection, normal));
		float Specular = dot(VertexToEye, LightReflect);
		if (Specular > 0)
		{
			Specular = pow(Specular, SpecularPower);
			SpecularColor = lightColor * SpecularIntensity * Specular;
		}
		//float Fresnel = 0.1 + (1.0 - 0.1)* pow((1.0-max(dot(normal,-normalize(ViewPos) ), 0.0)), 5.0);

		
		//single hardcoded directional light for testing
		// diffuse shading
		float diff = max(dot(normal, LightDirection), 0.0) * 0.5 + 0.5;
		float lightIntensity = 1.0;

		//FragColor = vec4(normal, 1.0f);
		vec4 diffuse = lightColor * diff * lightIntensity;
		FragColor = (ourColor * diffuse) + SpecularColor;
	}
}