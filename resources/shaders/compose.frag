#version 450 core
out vec4 FragColor;
  
in vec2 TexCoords;
in vec4 ModelPos;

uniform vec2 screenSize;
uniform float ProjectFov = 65.0;
uniform mat4 projection;
uniform mat4 view;
uniform vec4 ourColor;

layout(binding = 0) uniform sampler2D particleTexture;
layout(binding = 1) uniform sampler2D backgroundTexture;

float dz2x(float x, float y)
{
	if(x<=0 || y<=0 || x>=1 || y>=1)
	{
		return 0;
	}

	float v0 = textureOffset(particleTexture, vec2(x, y), ivec2(-1, 0)).x;
	float v1 = texture(particleTexture, vec2(x, y) ).x;
	float v2 = textureOffset(particleTexture, vec2(x, y), ivec2(1, 0)).x;

	float ret = 0;

	if( (v0 == 0) && (v2 != 0))
	{
		ret = (v2 - v1);
	}
	else if((v2 ==0) && (v0 != 0))
	{
		ret = (v1 - v0);
	}
	else 
	{
		ret = (v2 - v0) / 2.0f;
	}

	return ret;
}

float dz2y(float x, float y)
{
	if( x<=0 || y<=0 || x>=1 || y>=1)
	{
		return 0;
	}

	float v0 = textureOffset(particleTexture, vec2(x, y), ivec2(0, -1)).x;
	float v1 = texture(particleTexture, vec2(x, y)).x;
	float v2 = textureOffset(particleTexture, vec2(x, y), ivec2(0, 1)).x;

	float ret = 0;

	if( (v0 == 0 && v2 != 0) )
	{
		ret = (v2 - v1);
	}
	else if( (v2 == 0 && v0 != 0) )
	{
		ret = (v1 - v0);
	}
	else
	{
		ret = ( v2 - v0) / 2.f;
	}

	return ret;
}

vec4 GetNormal()
{
	float Depth = texture(particleTexture, TexCoords).x;
	vec3 Normal = vec3(0.0, 0.0, 0.0);

	float OffsetX = 1 / screenSize.x;
	float OffsetY = 1 / screenSize.y;

	int SampleOffset = 3;
	int Counter = 0;

	if (Depth > 0)
	{
		for (int i=-SampleOffset; i<=SampleOffset; i++)
			for (int k=-SampleOffset; k<=SampleOffset; k++)
			{
				float fd = textureOffset(particleTexture, TexCoords, ivec2(i, k)).x;
				float fx = gl_FragCoord.x + i;
				float fy = gl_FragCoord.y + k;
				if ((fx>=0) && (fx<=screenSize.x) && (fy>=0) && (fy<=screenSize.y) && (fd>0))
				{
					float dz_x = dz2x(TexCoords.x+i*OffsetX, TexCoords.y+k*OffsetY);
					float dz_y = dz2y(TexCoords.x+i*OffsetX, TexCoords.y+k*OffsetY);

//					float Cx = fx==0 ? 0 : 2 / (fx * screenSize.x);
//					float Cy = fy==0 ? 0 : 2 / (fy * screenSize.y);

					float Cx = fx==0 ? 0 : 2 / (fx * tan(ProjectFov /2));
					float Cy = fy==0 ? 0 : 2 / (fy * tan(ProjectFov /2));
//					float Cy = fy==0 ? 0 : 2 / (fy * ( 1 / tan(ProjectFov / 2 ))); 

					float D = Cy * Cy * dz_x * dz_x + Cx * Cx * dz_y * dz_y + Cx * Cx * Cy * Cy * fd * fd;
					if (D == 0) continue;

					float inv_sqrtD = 1 / sqrt(D);

					Normal.x += Cy * dz_x * inv_sqrtD;
					Normal.y += Cx * dz_y * inv_sqrtD;
					Normal.z += Cx * Cy * fd * inv_sqrtD;
					Counter++;
				}
			}
		Normal = Normal / float(Counter);
		Normal = normalize(Normal);
		Normal = Normal*0.5 + vec3(0.5);
	}
	return vec4(Normal, 1.0);
}

void main()
{ 
	vec4 backgroundColor = texture(backgroundTexture, TexCoords);
	float particleDepth = texture(particleTexture, TexCoords).x;

	//FragColor = vec4(vec3(particleDepth),1.0f);
	if(particleDepth == 0.0f) 
	{
		FragColor = vec4(0.0f);
	}
	else
	{
		vec3 normal = vec3(GetNormal());
		//normal = normal * mat3(inverse(view));
		//normal.xz = -normal.xz;

		//single hardcoded directional light for testing
		vec3 LightDirection = normalize(vec3( 0.2f, 1.0f, 0.3f));
		// diffuse shading
		float diff = max(dot(normal, LightDirection), 0.0);
		float lightIntensity = 1.0;

		//FragColor = vec4(normal, 1.0f);
		FragColor = ourColor * diff * lightIntensity;
	}
}