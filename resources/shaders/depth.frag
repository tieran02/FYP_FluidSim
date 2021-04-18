#version 450 core
out float particleDepth;

uniform vec4 ourColor;

in vec3 Normal;
in vec3 Color;
in vec4 clipspacePos;


float near = 0.1; 
float far  = 50.0; 

float LinearizeDepth(float depth) 
{
	float zNDC = depth * 2.0 - 1.0;
	return ( 2.0 * near * far ) / (far + near - zNDC * (far - near));
}

void main()
{           
	float depth = (2 * near) / (far + near -  gl_FragCoord.z * (far - near));
	
	if (depth > 1.0) 
		discard;

	particleDepth = depth;
}