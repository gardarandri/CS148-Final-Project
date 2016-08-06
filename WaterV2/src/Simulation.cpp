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
	v = 3.5; 				//Viscosity
	k = 3.0;				//Presure constant
	g = -9.81;				//Gravitational force
	m = 10.0;				//Particle mass
	p_0 = 1.0;				//Rest presure
	d_0 = 900.0;			//Rest density
	dt = 0.01;				//Time step
	c_R = 0.1;

	effectiveRadius = 0.50;

	N = particles;

	density = new GLfloat[N];
	presure = new GLfloat[N];

	x = new glm::vec3[N];
	dx = new glm::vec3[N];

	xcopy = new glm::vec3[N];
	dxcopy = new glm::vec3[N];

	int cnt = 0;
	for(int l=0; l<10 && cnt < N; l++)
	for(int m=0; m<10 && cnt < N; m++)
	for(int n=0; n<10 && cnt < N; n++)
		x[cnt++] = glm::vec3(l*0.2 - 1.0, m*0.2, n*0.2 - 1.0);
}

void Simulation::step(){
	applyForces();

	for(int i=0; i<N; i++){
		glm::vec3 d	= dt*dx[i];
		while(collideAndMove(i,d)) {}

		x[i] += d;
	}
}

GLfloat kernel(glm::vec3 r, GLfloat r_e){
	GLfloat l = glm::length(r);
	if(l > r_e) return 0.0f;
	return 315.0f*pow(r_e*r_e - l*l, 3.0f) / (64.0f*PI*pow(r_e,9.0f));
}

glm::vec3 presurekernel(glm::vec3 r, GLfloat r_e){
	GLfloat l = glm::length(r);
	if(l > r_e) return glm::vec3(0.0,0.0,0.0);
	return 45.0f*pow(r_e - l,3.0f)*r / (PI*pow(r_e,6.0f)*l);
}

GLfloat viscositykernel(glm::vec3 r, GLfloat r_e){
	GLfloat l = glm::length(r);
	if(l > r_e) return 0.0f;
	return 45.0f*(r_e - l) / (PI*pow(r_e,6.0f));
}

void Simulation::applyForces(){

	for(int i=0; i<N; i++){
		dxcopy[i] = dx[i];

		density[i] = 1.0;
		for(int j=0; j<N; j++){
			density[i] += m * kernel(x[i] - x[j], effectiveRadius);
		}
		presure[i] = p_0 + k*(density[i] - d_0);
	}

	for(int i=0; i<N; i++){
		for(int j=0; j<N; j++){
			if(i != j){
				dxcopy[i] += dt * (presure[i] + presure[j]) / (2.0f*density[j]) * presurekernel(x[i] - x[j], effectiveRadius);

				dxcopy[i] += -dt * v * (dx[i] - dx[j]) / density[j] * viscositykernel(x[i] - x[j], effectiveRadius);

				if(glm::dot(dx[i],dx[j]) < 0.0){
					dxcopy[i] += dt * dx[i] * 0.01f * glm::dot(dx[i],dx[j]) / (glm::length(dx[i])*glm::length(dx[j]));
				}
			}
		}

		dxcopy[i].y += dt*g;

		for(int j=0; j<surfaces.size(); j++){
			glm::vec3 n = glm::normalize(glm::cross( surfaces[j].a - surfaces[j].b, surfaces[j].a - surfaces[j].c));
			GLfloat d = glm::dot(x[i] - surfaces[j].a, n);
			GLfloat side = (d>0) - (d<0);
			n = -side*n;
			GLfloat t = findCollision(i, surfaces[j], n);
			if(t > 0.0 && t < 1.0){
				dxcopy[i] += - dt * .1f* n / (t*t + 1.0f);
			}
		}
	}


	for(int i=0; i<N; i++){
		dx[i] = dxcopy[i];
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

	x[i] = x[i] + (mint - 0.001f)*particleStep;
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



glm::vec3 Simulation::getPosition(size_t index){
	return x[index];
}

void Simulation::addPlane(glm::mat4 modelMatrix){
	Triangle t1;
	t1.a = glm::vec3(modelMatrix*glm::vec4(-1.0,0.0,-1.0,1.0));
	t1.c = glm::vec3(modelMatrix*glm::vec4(1.0,0.0,-1.0,1.0));
	t1.b = glm::vec3(modelMatrix*glm::vec4(1.0,0.0,1.0,1.0));

	Triangle t2;
	t2.a = glm::vec3(modelMatrix*glm::vec4(-1.0,0.0,-1.0,1.0));
	t2.c = glm::vec3(modelMatrix*glm::vec4(1.0,0.0,1.0,1.0));
	t2.b = glm::vec3(modelMatrix*glm::vec4(-1.0,0.0,1.0,1.0));

	surfaces.push_back(t1);
	surfaces.push_back(t2);
}
