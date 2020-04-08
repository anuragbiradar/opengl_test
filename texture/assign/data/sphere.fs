#version 330 core
out vec4 FragColor;

in vec3 Normal;  
in vec3 FragPos;
in vec2 UV;

uniform sampler2D myTextureSampler;

void main()
{
	FragColor = vec4(1.0, 0.0, 0.0, 1.0);
	FragColor = vec4(texture( myTextureSampler, UV ).rgb, 1.0);
}
