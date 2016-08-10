// CS148 Summer 2016 Homework 3 - Shaders

#version 330 core
out vec4 color;

in vec3 FragPos;  
in vec3 Normal;  
in vec2 TexCoord;

uniform vec3 lightPos; 
uniform vec3 viewPos;
uniform vec3 lightColor;
uniform vec3 objectColor;

uniform sampler2D texture_diffuse;
uniform sampler2D texture_specular;

void main()
{
	vec3 n = normalize(Normal);
	vec3 lightDir = normalize(lightPos - FragPos);
	vec3 camDir = normalize(viewPos - FragPos);
	vec3 halfAngle = normalize(camDir + lightDir);

	float diffusion = max(dot(lightDir,n),0.0);
	float specular = pow(dot(halfAngle,n),30.0);
	vec4 ambiant = vec4(0.0,0.1,0.0,1.0);

	vec3 dcol = texture(texture_diffuse, TexCoord).rgb;
	float dalpha = texture(texture_diffuse, TexCoord).a;
	vec3 scol = texture(texture_specular, TexCoord).rgb;
	if(diffusion > 0.0){
		color = vec4((0.5*diffusion+0.5*0.6)*dcol,dalpha);
	}else{
		color = vec4(0.5*0.6*dcol,dalpha);
	}
} 
