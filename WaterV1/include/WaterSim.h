// GL Includes
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <random>

#include "Shader.h"


namespace Water{

	struct plane{
		glm::vec3 a,b,c;
	};

	class WaterSim{
		public:
			WaterSim(int numberOfParticles);

			//Number of particles
			int N;
			//Positions
			glm::vec3* x;
			//Velocities
			glm::vec3* dx;
			//Mass densities
			float* dens;
			//Presures
			float* pr;

			//Planes
			plane* b;
			//Number of planes
			int Np;

			void iter();
			void draw(Shader s);
			void drawCollision(Shader s);
		private:
			float kernelWidth;
			//Particle mass
			float pm;
			float kappa;
			//Viscosity
			float mu;

			GLfloat* tv;
			GLuint VBO, VBOnormals, VAO;

			GLuint planesVBO, planesVBOnormals, planesVAO;

			void updatedx();
			void updatex();
			void updatedens();
			void updatepr();
			void applyPressureForce();
			void applyViscosityForce();
			void applyExternalForce();

			void loadParticleDrawData();

			//Returns true if the x[particleIndex] is updated
			bool collide(int particleIndex, plane pl);
			void drawParticle(int index, Shader s);
			void drawTetrahedron();

			float kernel(glm::vec3 r, float h);
			glm::vec3 dkernel(glm::vec3 r, float h);
			float ddkernel(glm::vec3 r, float h);
	};
}
