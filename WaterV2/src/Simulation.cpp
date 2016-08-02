#include <iostream>
#include <cmath>
#include <vector>
#include <GL/glew.h>
#include <GL/glu.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Simulation.h"


using namespace Water;
using namespace std;

GLfloat const PI = 3.14159265;

GLfloat randomGLfloat(){
	return (rand() % 100000) / 100000.0;
}

Simulation::Simulation(size_t particles){
	N = particles;

	x = new glm::vec3[N];
	dx = new glm::vec3[N];
	dens = new GLfloat[N];

	tmpdx = new glm::vec3[N];
	tmpdens = new GLfloat[N];

	for(int i=0; i<N; i++){
		x[i] = glm::vec3(randomGLfloat(), randomGLfloat(), randomGLfloat());
	}
}

void Simulation::step(){
	applyForces();

	moveParticles();
}

void Simulation::moveParticles(){
	for(int i=0; i<N; i++){
		glm::vec3 d = dt * dx[i];
		while(collideAndMove(i, d)){}
		x[i] += d;
	}
}

bool Simulation::collideAndMove(int index, glm::vec3 &particleStep){
	int i = index;

	GLfloat mint = 1.0;
	int minat = -1;
	for(int j=0; j<surfaces.size(); j++){
		GLfloat t = findCollision(i, surfaces[j], particleStep);
		if(0.0 < t && t < mint){
			mint = t;
			minat = j;
		}
	}
	if(minat == -1) return false;

	glm::vec3 n = glm::normalize(glm::cross(surfaces[minat].a - surfaces[minat].c,surfaces[minat].a - surfaces[minat].b));

	x[i] = x[i] + mint*particleStep;
	dx[i] = dx[i] - (1.0f + c_R)*glm::dot(n,dx[i])*n;
	particleStep = (1-mint)*(particleStep - (1.0f + c_R)*glm::dot(n,particleStep)*n);

	return true;
}

GLfloat Simulation::findCollision(int index, Triangle tri, glm::vec3 &particleStep){
	int i = index;
	glm::vec3 n = glm::normalize(glm::cross(tri.a - tri.c,tri.a - tri.b));

	if(glm::dot(n, x[i] - tri.a)*glm::dot(n, x[i] + particleStep - tri.a) <= 0.0){
		float d = glm::dot(particleStep, n);
		float side = (d<0) - (d>0);
		n = side*n;

		glm::mat3 A(0.0);
		A[0] = tri.a - tri.b;
		A[1] = tri.a - tri.c;
		A[2] = particleStep;

		glm::mat3 A_t(0.0);
		A_t[0] = tri.a - tri.b;
		A_t[1] = tri.a - tri.c;
		A_t[2] = tri.a - x[i];

		GLfloat t = glm::determinant(A_t) / glm::determinant(A);

		glm::mat3 A_gamma(0.0);
		A_gamma[0] = tri.a - tri.b;
		A_gamma[1] = tri.a - x[i];
		A_gamma[2] = particleStep;

		GLfloat gamma = glm::determinant(A_gamma) / glm::determinant(A);

		if(gamma < 0 || gamma > 1) return -1.0; // no hit!

		glm::mat3 A_beta(0.0);
		A_beta[0] = tri.a - x[i];
		A_beta[1] = tri.a - tri.c;
		A_beta[2] = particleStep;

		GLfloat beta = glm::determinant(A_beta) / glm::determinant(A);

		if (beta < 0 || beta > 1 - gamma) return -1.0; // no hit!

		return t;
	}
	return -1.0;
}

void Simulation::applyForces(){
	//Copy into dx copy board
	for(int i=0; i<N; i++){
		tmpdx[i] = dx[i];
	}

	//Compute density
	for(int i=0; i<N; i++){
		tmpdens[i] = 0.0;
		for(int j=0; j<N; j++){
			tmpdens[i] += kernel(x[i] - x[j], kernelRadius);
		}
		tmpdens[i] *= pm;
	}
	//Copy updated density to dens
	for(int i=0; i<N; i++){
		dens[i] = tmpdens[i];
	}

	//Compute various forces
	for(int i=0; i<N; i++){
		GLfloat ddcolor = 0.0;
		glm::vec3 n(0.0f,0.0f,0.0f);
		for(int j=0; j<N; j++){
			//Forces are computed for one time step and therefore
			//we always multiply with dt.

			//Gravitational force
			tmpdx[i].y += dt * g;
				
			//Presure force
			tmpdx[i] += - ( dt * kappa * (dens[j] - dens_0) / dens[j]) * dkernel(x[i] - x[j], kernelRadius);

			//Viscosity force
			tmpdx[i] += dt * mu * dx[j] / dens[j] * ddkernel(x[i] - x[j], kernelRadius);

			//Color field
			ddcolor += ddkernel(x[i] - x[j], kernelRadius) / dens[j];

			//Surface normal
			n += dkernel(x[i] - x[j], kernelRadius);
		}

		//Surface tension is only applied if 
		//the interpolated surface normal is 
		//longer than a threshold (ell)
		if(glm::length(n) > ell){
			tmpdx[i] += - dt * sigma * ddcolor * glm::normalize(n);
		}
	}

	//Copy from dx copy board to dx
	for(int i=0; i<N; i++){
		dx[i] = tmpdx[i];
	}
}

GLfloat Simulation::kernel(glm::vec3 r, GLfloat h){
	if(glm::length(r) > h){
		return 0.0;
	}else{
		return 315.0f*pow(h*h - pow(glm::length(r),2),3) / (64.0f * PI * pow(h,9));
	}
}

//Kernel specifically for the presure force
glm::vec3 Simulation::dkernel(glm::vec3 r, GLfloat h){
	if(glm::length(r) > h) return glm::vec3(0.0,0.0,0.0);
	if(glm::length(r) < 0.0001){
		return glm::sign(r) * 45.0f / (PI * pow(h,6.0f));
	}else{
		return -r*(GLfloat)(45.0f*pow(h - glm::length(r),2)) / (GLfloat)(PI * pow(h,6) * glm::length(r));
	}
}

//Kernel specifically for the viscosity force
GLfloat Simulation::ddkernel(glm::vec3 r, GLfloat h){
	if(glm::length(r) > h) return 0.0;
	return 45.0f * (h - glm::length(r)) / (PI * pow(h,6));
}


/*
GLfloat Simulation::kernel(glm::vec3 r, GLfloat h){
	return pow(2*PI*h*h,- 3.0f / 2.0f) * exp(-pow(glm::length(r),2.0f)/(2.0f*h));
}

//Kernel specifically for the presure force
glm::vec3 Simulation::dkernel(glm::vec3 r, GLfloat h){
	return pow(2*PI*h*h,- 3.0f / 2.0f) * 2.0f*r*exp(-pow(glm::length(r),2.0f)/(2.0f*h)) / (2.0f * h);
}

//Kernel specifically for the viscosity force
GLfloat Simulation::ddkernel(glm::vec3 r, GLfloat h){
	return pow(2*PI*h*h,- 3.0f / 2.0f) *2.0f * exp(-pow(glm::length(r),2.0f)/(2.0f*h))*(pow(glm::length(r) / (2.0f*h),2.0f) + 6.0f / (2.0f*h));
}
*/








glm::vec3 Simulation::getPosition(size_t index){
	return x[index];
}

void Simulation::addPlane(glm::mat4 modelMatrix){
	Triangle t1;
	t1.a = glm::vec3(modelMatrix*glm::vec4(-1.0,0.0,-1.0,1.0));
	t1.b = glm::vec3(modelMatrix*glm::vec4(1.0,0.0,-1.0,1.0));
	t1.c = glm::vec3(modelMatrix*glm::vec4(1.0,0.0,1.0,1.0));

	Triangle t2;
	t2.a = glm::vec3(modelMatrix*glm::vec4(-1.0,0.0,-1.0,1.0));
	t2.b = glm::vec3(modelMatrix*glm::vec4(1.0,0.0,1.0,1.0));
	t2.c = glm::vec3(modelMatrix*glm::vec4(-1.0,0.0,1.0,1.0));

	surfaces.push_back(t1);
	surfaces.push_back(t2);
}
