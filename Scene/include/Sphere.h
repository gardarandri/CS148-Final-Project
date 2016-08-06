#ifndef SPHERE_H
#define SPHERE_H

#include <iostream>
#include <cmath>
#include <vector>
#include <GL/glew.h>
#include <GL/glu.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Shader.h"

namespace Water{
	class Sphere{
		public:
			Sphere(int resolution, GLfloat radius);

			void draw(Shader s, glm::vec3 position);
		private:
			GLuint VBO, VAO;

			size_t numberOfVertices;
			GLfloat radius;

			std::vector<glm::vec3> Sphere::generateSphereVertices(int resolution);
	};
}

#endif
