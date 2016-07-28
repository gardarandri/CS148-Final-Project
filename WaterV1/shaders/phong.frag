// CS148 Summer 2016 Homework 3 - Shaders

#version 330 core
out vec4 color;

in vec3 FragPos;  
in vec3 Normal;  

uniform vec3 lightPos; 
uniform vec3 viewPos;
uniform vec3 lightColor;
uniform vec3 objectColor;

void main()
{
	vec3 n = normalize(Normal);
	vec3 lightDir = normalize(FragPos - lightPos);
	vec3 camDir = normalize(FragPos - viewPos);
	vec3 halfAngle = normalize(camDir + lightDir);

	float diffusion = max(dot(lightDir,n),0.0);
	float specular = pow(dot(halfAngle,n),50.0);
	vec4 ambiant = vec4(0.1,0.03,0.0,1.0);

	color = vec4(diffusion*objectColor, 1.0f) + ambiant;
} 
