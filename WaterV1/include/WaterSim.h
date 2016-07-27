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
			WaterSim(int numberOfParticles){
				N = numberOfParticles;

				x = malloc(sizeof(glm::vec3)*N);
				dx = malloc(sizeof(glm::vec3)*N);

				for(int i=0; i<N; i++){
					x[i] = glm::vec3(10.0*(rand()%1000)/1000.0,10.0*(rand()%1000)/1000.0,10.0*(rand()%1000)/1000.0 -5.0);
					dx[i] = glm::vec3(0.0,0.0,0.0);
				}

				b = malloc(sizeof(plane)*2);
				b[0].a = glm::vec3(0.0,0.0,0.0);
				b[0].b = glm::vec3(1.0,0.0,0.0);
				b[0].c = glm::vec3(1.0,1.0,1.0);

				b[1].a = glm::vec3(0.0,0.0,0.0);
				b[1].b = glm::vec3(1.0,0.0,0.0);
				b[1].c = glm::vec3(1.0,1.0,-1.0);

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
				glGenVertexArrays(1, &VAO);

				glBindVertexArray(VAO);
					glBindBuffer(GL_ARRAY_BUFFER, VBO);
					glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    				glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
    				glEnableVertexAttribArray(0);
				glBindVertexArray(0);
				glBindBuffer(GL_ARRAY_BUFFER, 0);
			}

			//Number of particles
			int N;
			//Positions
			glm::vec3* x;
			//Velocities
			glm::vec3* dx;
			//Planes
			plane* b;

			void iter();
			void draw(Shader s);
		private:
			GLfloat* tv;
			GLuint VBO, VAO;

			void updatedx();
			void updatex();

			//Returns true if the x[particleIndex] is updated
			bool collide(int particleIndex, plane pl);
			void drawParticle(int index, Shader s);
			void drawTetrahedron();
	};
}
