#include <iostream>
#include <cmath>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "WaterSim.h"
#include "Shader.h"


using namespace Water;
using namespace std;

GLfloat* genNormals(GLfloat* v, int L){
	GLfloat* res = (GLfloat*)malloc(sizeof(GLfloat)*L);

	for(int i=0; i<L; i+=9){
		glm::vec3 a(v[i],v[i+1],v[i+2]);
		glm::vec3 b(v[i+3],v[i+4],v[i+5]);
		glm::vec3 c(v[i+6],v[i+7],v[i+8]);

		glm::vec3 n = glm::cross(b-a,c-a);

		for(int k=0; k<3; k++){
			res[i+k] = n[k];
			res[i+3+k] = n[k];
			res[i+6+k] = n[k];
		}
	}

	return res;
}

WaterSim::WaterSim(int numberOfParticles){
	N = numberOfParticles;
	//Particle mass
	pm = 1.0;
	//gas stiffness constant
	kappa = 0.0000001;
	//Viscosity
	mu = 0.000001;
	kernelWidth = 0.1;

	x = (glm::vec3*)malloc(sizeof(glm::vec3)*N);
	dx = (glm::vec3*)malloc(sizeof(glm::vec3)*N);
	dens = (float*)malloc(sizeof(glm::vec3)*N);
	pr = (float*)malloc(sizeof(glm::vec3)*N);

	for(int i=0; i<N; i++){
		x[i] = glm::vec3(1.0*(rand()%1000)/1000.0,1.0*(rand()%1000)/1000.0,1.0*(rand()%1000)/1000.0);
		dx[i] = glm::vec3(0.0,0.0,0.0);
	}

	Np = 12;
	b = new plane[Np];
	GLfloat sideSize = 1.0;
	b[0].a = glm::vec3(sideSize,1.0,-sideSize);
	b[0].b = glm::vec3(sideSize,1.0,sideSize);
	b[0].c = glm::vec3(-sideSize,1.0,sideSize);

	b[1].a = glm::vec3(-sideSize,1.0,sideSize);
	b[1].b = glm::vec3(-sideSize,1.0,-sideSize);
	b[1].c = glm::vec3(sideSize,1.0,-sideSize);

	b[2].a = glm::vec3(-sideSize,0.0,sideSize);
	b[2].b = glm::vec3(-sideSize,5.0,sideSize);
	b[2].c = glm::vec3(sideSize,0.0,sideSize);

	b[3].a = glm::vec3(sideSize,0.0,sideSize);
	b[3].b = glm::vec3(sideSize,5.0,sideSize);
	b[3].c = glm::vec3(-sideSize,5.0,sideSize);

	b[4].a = glm::vec3(-sideSize,0.0,-sideSize);
	b[4].b = glm::vec3(-sideSize,5.0,-sideSize);
	b[4].c = glm::vec3(sideSize,0.0,-sideSize);

	b[5].a = glm::vec3(sideSize,0.0,-sideSize);
	b[5].b = glm::vec3(sideSize,5.0,-sideSize);
	b[5].c = glm::vec3(-sideSize,5.0,-sideSize);

	b[6].a = glm::vec3(sideSize,0.0,-sideSize);
	b[6].b = glm::vec3(sideSize,5.0,-sideSize);
	b[6].c = glm::vec3(sideSize,5.0,sideSize);

	b[7].a = glm::vec3(sideSize,0.0,-sideSize);
	b[7].b = glm::vec3(sideSize,0.0,sideSize);
	b[7].c = glm::vec3(sideSize,5.0,sideSize);

	b[8].a = glm::vec3(-sideSize,0.0,-sideSize);
	b[8].b = glm::vec3(-sideSize,5.0,-sideSize);
	b[8].c = glm::vec3(-sideSize,5.0,sideSize);

	b[9].a = glm::vec3(-sideSize,0.0,-sideSize);
	b[9].b = glm::vec3(-sideSize,0.0,sideSize);
	b[9].c = glm::vec3(-sideSize,5.0,sideSize);

	b[10].a = glm::vec3(2*sideSize + sideSize,0.0,-sideSize);
	b[10].b = glm::vec3(2*sideSize + sideSize,0.0,sideSize);
	b[10].c = glm::vec3(2*sideSize - sideSize,0.0,sideSize);

	b[11].a = glm::vec3(2*sideSize -sideSize,0.0,sideSize);
	b[11].b = glm::vec3(2*sideSize -sideSize,0.0,-sideSize);
	b[11].c = glm::vec3(2*sideSize + sideSize,0.0,-sideSize);


	GLfloat tmp = 1.0f/sqrt(2.0f);
	GLfloat vertices[] = {
		1.0f,0.0f,-tmp,
		-1.0f,0.0f,-tmp,
		0.0f,1.0f,tmp,
		1.0f,0.0f,-tmp,
		0.0f,1.0f,tmp,
		0.0f,-1.0f,tmp,
		1.0f,0.0f,-tmp,
		0.0f,-1.0f,tmp,
		-1.0f,0.0f,-tmp,
		-1.0f,0.0f,-tmp,
		0.0f,1.0f,tmp,
		0.0f,-1.0f,tmp
	};

	glGenBuffers(1, &VBO);
	glGenBuffers(1, &VBOnormals);
	glGenVertexArrays(1, &VAO);

	glBindVertexArray(VAO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
		glEnableVertexAttribArray(0);

		glBindBuffer(GL_ARRAY_BUFFER, VBOnormals);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), genNormals(vertices,12*3), GL_STATIC_DRAW);

		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
		glEnableVertexAttribArray(1);
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glGenBuffers(1, &planesVBO);
	glGenBuffers(1, &planesVBOnormals);
	glGenVertexArrays(1, &planesVAO);


	glBindVertexArray(planesVAO);
		glBindBuffer(GL_ARRAY_BUFFER, planesVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*9*Np, (GLfloat*)b, GL_STATIC_DRAW);

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
		glEnableVertexAttribArray(0);

		glBindBuffer(GL_ARRAY_BUFFER, VBOnormals);
		glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*9*Np, genNormals((GLfloat*)b,12*3), GL_STATIC_DRAW);

		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
		glEnableVertexAttribArray(1);
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void WaterSim::iter(){
	updatedx();
	updatex();
}

void WaterSim::updatedx(){
	updatedens();
	applyPressureForce();
	applyViscosityForce();
	applyExternalForce();
}


bool WaterSim::collide(int particleIndex, plane pl){
	int i = particleIndex;

	glm::vec3 n = glm::normalize(glm::cross(pl.a - pl.c,pl.a - pl.b));

	if(glm::dot(n, x[i] - pl.a)*glm::dot(n, x[i] + dx[i] - pl.a) <= 0.0){
		float d = glm::dot(dx[i], n);
		float side = (d<0) - (d>0);
		n = side*n;
		/*
	 	*          A
	 	*		n	|     /   dx[i]
	 	* --------------/--------
	 	*              V
	 	*/

		glm::mat3 A(0.0);
		A[0] = pl.a - pl.b;
		A[1] = pl.a - pl.c;
		A[2] = dx[i];

		glm::mat3 A_t(0.0);
		A[0] = pl.a - pl.b;
		A[1] = pl.a - pl.c;
		A[2] = pl.a - x[i];

		GLfloat t = glm::determinant(A_t) / glm::determinant(A);

		glm::vec3 dir = glm::normalize(dx[i]);


		float eps = 0.000001;
		x[i] = x[i] - n * glm::dot(n, dx[i]);
		dx[i] = dx[i] - 1.2f*glm::dot(dx[i],n)*n;

		return true;
	}
	return false;
}

void WaterSim::updatex(){
	for(int i=0; i<N; i++){
		for(int j=0; j<Np; j++){
			collide(i,b[j]);
		}
		x[i] += dx[i];
		if(x[i].x > 10) x[i].x -= 20.0;
		if(x[i].x < -10) x[i].x += 20.0;
		if(x[i].y < -1) x[i].y += 10.0;
	}
}

void WaterSim::updatedens(){
	for(int i=0; i<N; i++){
		float newdens = 0.0;
		for(int j=0; j<N; j++){
			newdens += pm*kernel(x[i] - x[j], kernelWidth);
		}
		dens[i] = newdens;
	}
}

void WaterSim::updatepr(){
}

void WaterSim::applyPressureForce(){
	for(int i=0; i<N; i++){
		glm::vec3 f(0.0,0.0,0.0);
		for(int j=0; j<N; j++){
			if(i != j){
				dx[i] += -kappa * dkernel(x[i] - x[j], kernelWidth);
			}
		}
	}
}

void WaterSim::applyViscosityForce(){
	for(int i=0; i<N; i++){
		for(int j=0; j<N; j++){
			if(i != j){
				dx[i] += - dx[j] * (mu/dens[j]) * ddkernel(x[i] - x[j], kernelWidth);
			}
		}
	}
}

void WaterSim::applyExternalForce(){
	glm::vec3 f(0.0,-0.0001,0.0);
	for(int i=0; i<N; i++){
		dx[i] += f / pm;
	}
}

void WaterSim::drawTetrahedron(){
	glBindVertexArray(VAO);
	glDrawArrays(GL_TRIANGLES, 0, 12);
	glBindVertexArray(0);
}

void WaterSim::drawParticle(int index, Shader s){
	GLint modelLoc = glGetUniformLocation(s.Program, "model");
	glm::mat4 model = glm::scale(glm::translate(glm::mat4(1.0), x[index]),glm::vec3(0.1,0.1,0.1));
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	GLint colorLoc = glGetUniformLocation(s.Program, "objectColor");
	glm::vec3 color(1.0f,1.0f,0.0f);
	glUniform3fv(colorLoc, 1, (GLfloat*)glm::value_ptr(color));
	drawTetrahedron();
}

void WaterSim::draw(Shader s){
	for(int i=0; i<N; i++){
		drawParticle(i, s);
	}
}

void WaterSim::drawCollision(Shader s){
	GLint colorLoc = glGetUniformLocation(s.Program, "objectColor");
	glm::vec3 color(0.0f,0.0f,1.0f);
	glUniform3fv(colorLoc, 1, glm::value_ptr(color));
	glBindVertexArray(planesVAO);
	glDrawArrays(GL_TRIANGLES, 0, 3*Np);
	glBindVertexArray(0);
}

float WaterSim::kernel(glm::vec3 r, float h){
	if(glm::length(r) > h){
		return 0.0;
	}else{
		return 15.0/(3.14159265*pow(h,6.0f)) * pow(h - glm::length(r),3.0f);
	}
}

glm::vec3 WaterSim::dkernel(glm::vec3 r, float h){
	if(glm::length(r) > h){
		return glm::vec3(0.0,0.0,0.0);
	}else{
		return -90.0f/(3.14159265f*pow(h,6.0f)) * pow(h - glm::length(r),2.0f)*r*(1/glm::length(r));
	}
}

float WaterSim::ddkernel(glm::vec3 r, float h){
	if(glm::length(r) > h){
		return 0.0;
	}else{
		return -90.0f/(3.14159265f*pow(h,6.0f))*(1/glm::length(r))*(h - glm::length(r))*(h - 2.0f*glm::length(r));
	}
}



