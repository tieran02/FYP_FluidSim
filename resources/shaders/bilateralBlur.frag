#version 450 core
#define SIGMA 5.0
#define BSIGMA 0.1
#define MSIZE 20

out float particleDepth;
  
in vec2 TexCoords;

uniform sampler2D image;
  
float normpdf(float x, float sigma)
{
	return 0.39894*exp(-0.5*x*x/(sigma*sigma))/sigma;
}

void main()
{             
    float c = texture( image, TexCoords).r;

    //declare stuff
	const int kSize = (MSIZE-1)/2;
	float kernel[MSIZE];
	float final_colour = 0.0;
	
	//create the 1-D kernel
	float Z = 0.0;
	for (int j = 0; j <= kSize; ++j)
	{
		kernel[kSize+j] = kernel[kSize-j] = normpdf(float(j), SIGMA);
	}

    float cc;
	float factor;
	float bZ = 1.0/normpdf(0.0, BSIGMA);
    vec2 tex_offset = 1.0 / textureSize(image, 0); // gets size of single texel

	//read out the texels
	for (int i=-kSize; i <= kSize; ++i)
	{
		for (int j=-kSize; j <= kSize; ++j)
		{
			cc = texture(image, TexCoords + vec2(tex_offset.x * i, tex_offset.y * j)).r;
			factor = normpdf(cc-c, BSIGMA)*bZ*kernel[kSize+j]*kernel[kSize+i];
			Z += factor;
			final_colour += factor*cc;

		}
	}
	
	particleDepth = final_colour/Z;
}