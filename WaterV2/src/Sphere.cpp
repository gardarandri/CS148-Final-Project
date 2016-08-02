#include <iostream>
#include <cmath>
#include <vector>
#include <GL/glew.h>
#include <GL/glu.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Shader.h"
#include "Sphere.h"

using namespace Water;
using namespace std;

GLfloat const PI = 3.14159265;

Sphere::Sphere(int resolution, GLfloat r){
	radius = r;
	
	glGenBuffers(1, &VBO);
	glGenVertexArrays(1, &VAO);

	glBindVertexArray(VAO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);

		vector<glm::vec3> sphereVertices = generateSphereVertices(resolution);
		numberOfVertices = sphereVertices.size();
		glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*numberOfVertices*3, (GLfloat*)sphereVertices.data(), GL_STATIC_DRAW);
		
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3*sizeof(GLfloat), (GLvoid*)0);
		glEnableVertexAttribArray(0);

		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3*sizeof(GLfloat), (GLvoid*)0);
		glEnableVertexAttribArray(1);
	glBindVertexArray(0);
}

vector<glm::vec3> Sphere::generateSphereVertices(int resolution){
	vector<glm::vec3> res;

	GLfloat step = PI / resolution;

	GLfloat theta = 0;
	GLfloat phi = 0;

	for(int i=0; i<resolution; i++){
		for(int j=0; j<resolution; j++){
			glm::vec3 v1(sin(theta)*cos(phi), sin(theta)*sin(phi), cos(theta));
			glm::vec3 v2(sin(theta+step)*cos(phi), sin(theta+step)*sin(phi), cos(theta+step));
			glm::vec3 v3(sin(theta+step)*cos(phi+step*2), sin(theta+step)*sin(phi+step*2), cos(theta+step));
			glm::vec3 v4(sin(theta)*cos(phi+step*2), sin(theta)*sin(phi+step*2), cos(theta));

			res.push_back(radius*v1);
			res.push_back(radius*v2);
			res.push_back(radius*v3);

			res.push_back(radius*v1);
			res.push_back(radius*v3);
			res.push_back(radius*v4);

			phi += 2*step;
		}
		theta += step;
	}

	return res;
}

void Sphere::draw(Shader s, glm::vec3 position){
        GLint modelLoc = glGetUniformLocation(s.Program, "model");

        glm::mat4 model = glm::translate(glm::mat4(1.0), position);
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

		glBindVertexArray(VAO);
		glDrawArrays(GL_TRIANGLES, 0, numberOfVertices);
		glBindVertexArray(0);
}
