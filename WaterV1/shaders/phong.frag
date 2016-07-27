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
	vec3 lightDir = normalize(lightPos - FragPos);
	vec3 camDir = normalize(viewPos - FragPos);
	vec3 halfAngle = normalize(camDir + lightDir);

	float diffusion = max(dot(lightDir,n),0.0);
	float specular = pow(dot(halfAngle,n),50.0);
	vec4 ambiant = vec4(0.1,0.03,0.0,1.0);

	color = dot(lightDir,n) > 0 ? 
		vec4(diffusion*vec3(1.f,0.3f,0.f), 1.0f)
		+vec4(specular*vec3(1.f,1.f,1.f), 1.0f)
		+ambiant
		:
		vec4(diffusion*vec3(1.f,0.3f,0.f), 1.0f)
		+ambiant;
} 
