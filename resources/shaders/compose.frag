#version 450 core
out vec4 FragColor;
  
in vec2 TexCoords;
in vec4 ModelPos;

uniform vec2 screenSize;
uniform mat4 projection;
uniform mat4 view;
uniform vec4 ourColor;

layout(binding = 0) uniform sampler2D particleTexture;
layout(binding = 1) uniform sampler2D backgroundTexture;

vec3 eyespacePos(vec2 pos) {
	float depth = texture(particleTexture, pos).x;
	pos = (pos - vec2(0.5f)) * 2.0f;
	return(depth * vec3(-pos.x * projection[0][0], -pos.y / projection[1][1], 1.0f));
}

// Compute eye-space normal. Adapted from PySPH.
vec3 eyespaceNormal(vec2 pos) {
	// Width of one pixel
	vec2 dx = vec2(1.0f / screenSize.x, 0.0f);
	vec2 dy = vec2(0.0f, 1.0f / screenSize.y);

	// Central z
	float zc =  texture(particleTexture, pos).x;

	// Derivatives of z
	// For shading, one-sided only-the-one-that-works version
	float zdxp = texture(particleTexture, pos + dx).x;
	float zdxn = texture(particleTexture, pos - dx).x;
	float zdx = (zdxp == 0.0f) ? (zdxn == 0.0f ? 0.0f : (zc - zdxn)) : (zdxp - zc);

	float zdyp = texture(particleTexture, pos + dy).x;
	float zdyn = texture(particleTexture, pos - dy).x;
	float zdy = (zdyp == 0.0f) ? (zdyn == 0.0f ? 0.0f : (zc - zdyn)) : (zdyp - zc);

	// Projection inversion
	float cx = 2.0f / (screenSize.x * -projection[0][0]);
	float cy = 2.0f / (screenSize.y * -projection[1][1]);

	// Screenspace coordinates
	float sx = floor(pos.x * (screenSize.x - 1.0f));
	float sy = floor(pos.y * (screenSize.y - 1.0f));
	float wx = (screenSize.x - 2.0f * sx) / (screenSize.x * projection[0][0]);
	float wy = (screenSize.y - 2.0f * sy) / (screenSize.y * projection[1][1]);

	// Eyespace position derivatives
	vec3 pdx = normalize(vec3(cx * zc + wx * zdx, wy * zdx, zdx));
	vec3 pdy = normalize(vec3(wx * zdy, cy * zc + wy * zdy, zdy));

	return normalize(cross(pdx, pdy));
}

void main()
{ 
	vec4 backgroundColor = texture(backgroundTexture, TexCoords);
	float particleDepth = texture(particleTexture, TexCoords).x;

	//FragColor = vec4(vec3(particleDepth.r),1.0);

	if(particleDepth == 0.0f) 
	{
		FragColor = vec4(0.0f);
	}
	else
	{
		vec3 normal = eyespaceNormal(TexCoords);
		//normal = normal * mat3(inverse(view));
		//normal.xz = -normal.xz;

		 //single hardcoded directional light for testing
		vec3 LightDirection = normalize(vec3( 0.2f, 1.0f, 0.3f));
		// diffuse shading
		float diff = max(dot(normal, LightDirection), 0.0);
		float lightIntensity = 1.0;

		FragColor = vec4(normal, 1.0f);
		//FragColor = ourColor * diff * lightIntensity;

		/*vec3 lightDir = vec3(1.0f, 1.0f, -1.0f);

		vec3 pos = eyespacePos(TexCoords);
		pos = (vec4(pos, 1.0f) * inverse(view)).xyz;

		float lambert = max(0.0f, dot(normalize(lightDir), normal));
		vec3 fromEye = normalize(pos);
		fromEye.xz = -fromEye.xz;
		vec3 reflectedEye = normalize(reflect(fromEye, normal));

		vec4 particleColor = exp(-vec4(0.6f, 0.2f, 0.05f, 3.0f));
		particleColor.w = clamp(1.0f - particleColor.w, 0.0f, 1.0f);
		particleColor.rgb = (lambert + 0.2f) * particleColor.rgb;
		particleColor.w = 1.0f;
		particleColor.rgb = vec3(lambert + 0.2f);*/


		//FragColor = particleColor;
	}
}