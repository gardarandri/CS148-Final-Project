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
	//float specular = pow(dot(halfAngle,n),50.0);
	float mossratio = max(-n.y, 0.0);
	vec4 ambiant = vec4(0.0,0.1,0.0,1.0);

	vec3 dcol = texture(texture_diffuse, TexCoord).xyz;
	vec3 mosscol = texture(texture_specular, TexCoord).xyz;

	if(diffusion > 0.0){
		color = vec4((diffusion+0.2)*(dcol + mossratio*mosscol),1.0);
	}else{
		color = vec4(0.2*(dcol + mossratio*mosscol),1.0);
	}


	/*
	if(diffusion > 0.0){
		color = vec4((diffusion+0.1)*dcol + specular*scol,1.0);
	}else{
		color = vec4(0.1*dcol,1.0);
	}
	*/
} 
