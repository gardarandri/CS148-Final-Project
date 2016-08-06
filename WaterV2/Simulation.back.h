#ifndef SIMULATION_H
#define SIMULATION_H

#include <vector>
#include <iostream>
#include <cmath>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <cmath>


namespace Water{
	struct Triangle{
		glm::vec3 a,b,c;
	};

	class Simulation{
		public:
			Simulation(size_t particles);

			//Call this to progress the simulation one time step
			void step();

			//Returns the position of particle at index
			glm::vec3 getPosition(size_t index);

			//Returns the number of particles in the simulation
			size_t getNumberOfParticles(){ return N; }

			//Adds a collision plane which is the rectangle (-1,0,-1) x (1,0,1)
			//transformed by modelMatrix
			void addPlane(glm::mat4 modelMatrix);

			//Collision surfaces
			std::vector<Triangle> surfaces;
		private:
			//Number of particles
			size_t N;

			//Physics constats
			GLfloat dens_0 = 998.0;	//Rest density
			GLfloat g = -9.81;	  	//Gravity acceleration
			GLfloat kappa = 3.0;	//Gas constant
			GLfloat mu = 3.5;
			GLfloat sigma = 0.0728;	//Surface tension constant
			GLfloat ell = 7.065;	//Surface tension threshold
			GLfloat dt = 0.01;		//Time step
			GLfloat pm = 10.0;		//Mass of one particle
			GLfloat c_R = 0.2;		//Collision constant of reflection

			//Simulation constans
			GLfloat kernelRadius = 0.3;//0435;	//How big the smoothing kernel is

			//Particle information
			glm::vec3* x;		//Particle positions
			glm::vec3* dx;		//Particles velocity between dx_1 and dx_2
			glm::vec3* dx_1;	//Particles velocity at most recent timestep
			glm::vec3* dx_2;	//Particles velocity at second most recent timestep
			glm::vec3* a;		//Particles acceleration
			GLfloat* dens; 	//Particles density

			glm::vec3* tmpdx;		//Particles velocity copy board
			GLfloat* tmpdens; 	//Particles density copy board


			//Calculates various forces and applies them to tmpdx
			void applyForces();

			//Moves particels by dx
			void moveParticles();

			//Moves particle x[index] and handels collision to any surfaces.
			//Returns true if the particle reflects off of something.
			bool collideAndMove(int index, glm::vec3 &particleStep);

			//Helper function to find how far along a collision is.
			//Returns -1.0 if no collision otherwise is returns t such
			//that x[index] + t * dx[index] is in the triangle
			GLfloat findCollision(int index, Triangle tri, glm::vec3 &particleStep);

			//Kernels used to approximate various field properties
			GLfloat kernel(glm::vec3 r, GLfloat h);
			glm::vec3 dkernel(glm::vec3 r, GLfloat h);
			GLfloat ddkernel(glm::vec3 r, GLfloat h);
	};
}

#endif
