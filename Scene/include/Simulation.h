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

			//Physical arrays
			GLfloat* density;
			GLfloat* presure;
			glm::vec3* x;
			glm::vec3* dx;

			glm::vec3* xcopy;
			glm::vec3* dxcopy;

			int* ht;

			//Physical constans
			GLfloat v = 1.0; 				//Viscosity
			GLfloat k = 0.0001;				//Presure constant
			GLfloat g = -9.81;				//Gravitational force
			GLfloat m = 1.0;				//Particle mass
			GLfloat p_0 = 1.0;				//Rest presure
			GLfloat d_0 = 1.0;				//Rest density
			GLfloat dt = 0.04;				//Time step
			GLfloat c_R = 0.1;

			GLfloat effectiveRadius = 0.4;
			GLfloat gridRes = 0.2;
			int checkGridHalfWidth = 2;

			void applyForces();

			bool collideAndMove(int index, glm::vec3 &particleStep);
			GLfloat findCollision(int index, Triangle tri, glm::vec3 &particleStep);

			size_t hash(glm::vec3 t);
	};
}

#endif
