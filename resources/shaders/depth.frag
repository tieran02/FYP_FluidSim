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
	particleDepth = (2 * near) / (far + near -  gl_FragCoord.z * (far - near));
	//particleDepth = gl_FragCoord.z;


	// Set up output
	//float far = gl_DepthRange.far; 
	//float near = gl_DepthRange.near;
	//float deviceDepth = clipspacePos.z / clipspacePos.w;
	//float fragDepth = (((far - near) * deviceDepth) + near + far) / 2.0;
	//gl_FragDepth = fragDepth;

	//float linearDepth =  LinearizeDepth(clipspacePos.z / clipspacePos.w);

	//if(fragDepth > texture(terrainTexture, gl_FragCoord.xy / screenSize).w) {
		//discard;
	//}

	//particleDepth = gl_FragCoord.z;
}