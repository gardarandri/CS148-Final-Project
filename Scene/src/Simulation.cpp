#include <iostream>
#include <thread>
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
	pm = 10.0;				//Particle mass
	p_0 = 1001.3;				//Rest presure
	d_0 = 998.0;			//Rest density
	dt = 0.01;				//Time step
	c_R = 0.1;
	/*
	v = 3.5; 				//Viscosity
	k = 3.0;				//Presure constant
	g = -9.81;				//Gravitational force
	pm = 10.0;				//Particle mass
	p_0 = 1.0;				//Rest presure
	d_0 = 500.0;			//Rest density
	dt = 0.01;				//Time step
	c_R = 0.1;
	*/

	effectiveRadius = 0.50;

	N = particles;

	density = new GLfloat[N];
	presure = new GLfloat[N];

	x = new glm::vec3[N];
	dx = new glm::vec3[N];

	xcopy = new glm::vec3[N];
	dxcopy = new glm::vec3[N];

	ht = new int*[htSize];
	for(int i=0; i<htSize; i++){
		ht[i] = new int[htBucket];
	}
	htBuckets = new int[htSize];

	int cnt = 0;
	for(int m=0; m<100 && cnt < N; m++)
	for(int l=0; l<10 && cnt < N; l++)
	for(int n=0; n<10 && cnt < N; n++)
		x[cnt++] = glm::vec3(l*0.3 + 0.5, m*0.3 - 1.19, n*0.3 - 4.37);
}

static void applyForcesThreaded(Simulation* w, int imod);

void Simulation::step(){
	hashParticles();
	
	bool doThreading = false;
	if(doThreading == true){
		for(int i=0; i<N; i++){
			dxcopy[i] = dx[i];
		}

		vector<thread> t;
		for(int i=0; i<N; i++){
			t.push_back(thread(applyForcesThreaded, this, i));
		}

		for(int i=0; i<N; i++){
			t[i].join();
		}

		for(int i=0; i<N; i++){
			 dx[i] = dxcopy[i];
		}
	}else{
		applyForces();
	}

	for(int i=0; i<N; i++){
		glm::vec3 d	= dt*dx[i];
		while(collideAndMove(i,d)) {}

		x[i] += d;

		if(x[i].y < -4.5 || x[i].x > 6.0 || x[i].x < -2.0 || x[i].z > 10.0 || x[i].z < -8.0){
			x[i] = glm::vec3(1.40767 + 2.0*randomGLfloat() - 1.0,-1.19419,-4.87515 + 2.0*randomGLfloat() - 1.0);
			dx[i] *= 0.0f;
			dx[i].z = 1.7;
		}
	}
}

const GLfloat EPS = 1e-10;

GLfloat kernel(glm::vec3 r, GLfloat r_e){
	GLfloat l = glm::length(r);
	if(l > r_e) return 0.0f;
	return 315.0f*pow(r_e*r_e - l*l, 3.0f) / (64.0f*PI*pow(r_e,9.0f));
}

glm::vec3 presurekernel(glm::vec3 r, GLfloat r_e){
	GLfloat l = glm::length(r);
	if(l > r_e) return glm::vec3(0.0,0.0,0.0);
	return 45.0f*pow(r_e - l,3.0f)*r / (PI*pow(r_e,6.0f)*l + EPS);
}

GLfloat viscositykernel(glm::vec3 r, GLfloat r_e){
	GLfloat l = glm::length(r);
	if(l > r_e) return 0.0f;
	return 45.0f*(r_e - l) / (PI*pow(r_e,6.0f));
}

static void applyForcesThreaded(Simulation* w, int imod){
	cout<<"Running imod = "<<imod<<endl;
	w -> applyForces(imod);
}

void Simulation::applyForces(int imod){
	glm::vec3 mx(gridRes*1.0001f, 0.0, 0.0);
	glm::vec3 my(0.0, gridRes*1.0001f, 0.0);
	glm::vec3 mz(0.0, 0.0, gridRes*1.0001f);
	int i = imod;

	density[i] = 0.0;
	for(int l=-checkGridHalfWidth; l<=checkGridHalfWidth; l++){
		for(int m=-checkGridHalfWidth; m<=checkGridHalfWidth; m++){
			for(int n=-checkGridHalfWidth; n<=checkGridHalfWidth; n++){
				int h=hash(x[i] + (GLfloat)l*mx + (GLfloat)m*my + (GLfloat)n*mz);
				int* bucket = ht[h];
				for(int j=0; j<htBucket && j<htBuckets[h]; j++){
					int k = ht[h][j];
					density[i] += pm * kernel(x[i] - x[k], effectiveRadius);
				}
			}
		}
	}
	presure[i] = p_0 + k*(density[i] - d_0);

	for(int l=-checkGridHalfWidth; l<=checkGridHalfWidth; l++){
		for(int m=-checkGridHalfWidth; m<=checkGridHalfWidth; m++){
			for(int n=-checkGridHalfWidth; n<=checkGridHalfWidth; n++){
				int h=hash(x[i] + (GLfloat)l*mx + (GLfloat)m*my + (GLfloat)n*mz);
				int* bucket = ht[h];
				for(int j=0; j<htBucket && j<htBuckets[h]; j++){
					int k = ht[h][j];
					if(k != i){
						dxcopy[i] += dt * (presure[i] + presure[k]) / (2.0f*density[k]) * presurekernel(x[i] - x[k], effectiveRadius);

						dxcopy[i] += -dt * v * (dx[i] - dx[k]) / density[k] * viscositykernel(x[i] - x[k], effectiveRadius);

						if(glm::dot(dx[i],dx[k]) < 0.0){
							dxcopy[i] += dt * dx[i] * 0.008f * glm::dot(dx[i],dx[k]) / (glm::length(dx[i])*glm::length(dx[k]) + EPS);
						}
					}
				}
			}
		}
	}
	dxcopy[i].y += dt*g;
}

void Simulation::applyForces(){
	glm::vec3 mx(gridRes*1.0001f, 0.0, 0.0);
	glm::vec3 my(0.0, gridRes*1.0001f, 0.0);
	glm::vec3 mz(0.0, 0.0, gridRes*1.0001f);

	for(int i=0; i<N; i++){
		dxcopy[i] = dx[i];
		density[i] = 0.0;
	}
	
	for(int i=0; i<N; i++){
		for(int l=-checkGridHalfWidth; l<=checkGridHalfWidth; l++){
			for(int m=-checkGridHalfWidth; m<=checkGridHalfWidth; m++){
				for(int n=-checkGridHalfWidth; n<=checkGridHalfWidth; n++){
					int h=hash(x[i] + (GLfloat)l*mx + (GLfloat)m*my + (GLfloat)n*mz);
					for(int j=0; j<htBucket && j<htBuckets[h]; j++){
						density[i] += kernel(x[i] - x[ht[h][j]], effectiveRadius);
					}
				}
			}
		}
		density[i] *= pm;
		presure[i] = p_0 + k*(density[i] - d_0);
	}

	glm::vec3 f(0.0,0.0,0.0);
	for(int i=0; i<N; i++){
		f *= 0.0f;
		for(int l=-checkGridHalfWidth; l<=checkGridHalfWidth; l++){
			for(int m=-checkGridHalfWidth; m<=checkGridHalfWidth; m++){
				for(int n=-checkGridHalfWidth; n<=checkGridHalfWidth; n++){
					int h=hash(x[i] + (GLfloat)l*mx + (GLfloat)m*my + (GLfloat)n*mz);
					int* bucket = ht[h];
					for(int j=0; j<htBucket && j<htBuckets[h]; j++){
						int k = bucket[j];
						if(k != i){
							//dxcopy[i] += dt * (presure[i] + presure[k]) / (2.0f*density[k]) * presurekernel(x[i] - x[k], effectiveRadius);
							f += (presure[i] + presure[k]) / (2.0f*density[k]) * presurekernel(x[i] - x[k], effectiveRadius);

							//dxcopy[i] += -dt * v * (dx[i] - dx[k]) / density[k] * viscositykernel(x[i] - x[k], effectiveRadius);
							f +=  - v * (dx[i] - dx[k]) / density[k] * viscositykernel(x[i] - x[k], effectiveRadius);

							if(glm::dot(dx[i],dx[k]) < 0.0){
								//dxcopy[i] += dt * dx[i] * 0.001f * glm::dot(dx[i],dx[k]) / (glm::length(dx[i])*glm::length(dx[k]) + EPS);
								f += dx[i] * 0.01f * glm::dot(dx[i],dx[k]) / (glm::length(dx[i])*glm::length(dx[k]) + EPS);
							}
						}
					}
				}
			}
		}
		dxcopy[i] += dt*f;
		dxcopy[i].y += dt*g;
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

const int p1 = 73856093;
const int p2 = 19349663;
const int p3 = 83492791;

size_t Simulation::hash(glm::vec3 t){
	return (((p1*(int)(t.x / gridRes)) ^ (p2*(int)(t.y / gridRes)) ^ (p2*(int)(t.z / gridRes))) % htSize + htSize) % htSize;
}

void Simulation::hashParticles(){
	for(int i=0; i<htSize; i++){
		htBuckets[i] = 0;
	}

	for(int i=0; i<N; i++){
		int h = hash(x[i]);
		ht[h][htBuckets[h]++] = i;
	}
}

glm::vec3 Simulation::getPosition(size_t index){
	return x[index];
}

glm::vec3 Simulation::getVelocity(size_t index){
	return dx[index];
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
