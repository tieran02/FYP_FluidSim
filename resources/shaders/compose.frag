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
layout(binding = 2) uniform sampler2D backgroundTexture;
layout(binding = 3) uniform samplerCube skybox;
layout(binding = 4) uniform sampler2D thicknessTexture;

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
	return viewportToEyeSpace(TexCoords, depth);
}

void main()
{ 
	float particleDepth = texture(depthTexture, TexCoords).x;
	vec4 particleNormal = texture(normalTexture, TexCoords);

	if(particleDepth == 0.0) 
	{
		FragColor = texture(backgroundTexture, TexCoords);
	}
	else
	{
		vec3 normal = particleNormal.xyz;
		vec3 ViewPos = eyespacePos(TexCoords, particleDepth);
		vec3 WorldPos = WorldPosFromDepth(particleDepth);

		vec3 LightDirection = normalize(vec3( 0.0, 1.0, 0.0));
		vec4 lightColor = vec4(1.0, 1.0, 1.0, 1.0);

		vec3 fromEye = normalize(-WorldPos);
		vec4 SpecularColor = vec4(0, 0, 0, 0);
		vec3 LightReflect = normalize(reflect(LightDirection, normal));
		float spec = dot(fromEye, LightReflect);
		if (spec > 0)
		{
			spec = pow(spec, SpecularPower);
			SpecularColor   = lightColor * SpecularIntensity * spec;
		}

		float thickness = texture(thicknessTexture, TexCoords).r;

		//TODO z posiing is incorrect in get pos
		//vec3 ViewDirection = normalize(-WorldPos).xyz;
		vec3 ViewDirection = normalize(mat3(inverse(view)) * fromEye).xyz;
		vec3 N =  mat3(inverse(view)) * normal;

		vec3 Reflection = reflect(ViewDirection, N);
		vec4 ReflectionColor = vec4(texture(skybox, Reflection).rgb, 1.0);

		float Radio = 1.0 / 1.33;
		vec3 transmission = (1.0-(1.0-ourColor.xyz)*thickness);
		vec3 Refraction = refract(ViewDirection, N, Radio);
		vec4 RefractionColor = texture(skybox, Refraction) * vec4(transmission, 1.0);
		
		//single hardcoded directional light for testing
		// diffuse shading
		float diff = max(dot(normal, LightDirection), 0.0) * 0.5 + 0.5;
		float lightIntensity = 1.0;

		//FragColor = vec4(normal, 1.0f);
		vec4 diffuse = lightColor * diff * lightIntensity;

		FragColor = (ourColor * diffuse) + SpecularColor + mix(ReflectionColor, RefractionColor, 0.7);
	}
}