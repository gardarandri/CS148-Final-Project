#include <iostream>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "WaterSim.h"
#include "Shader.h"


using namespace Water;
using namespace std;


void WaterSim::iter(){
	updatedx();
	updatex();
}

void WaterSim::updatedx(){
	for(int i=0; i<N; i++){
		dx[i].x -= 0.0001;
		dx[i].y -= 0.001;
	}
}

bool WaterSim::collide(int particleIndex, plane pl){
	int i = particleIndex;

	glm::vec3 n = glm::normalize(glm::cross(pl.a - pl.c,pl.a - pl.b));

	if(glm::dot(n, x[i] - pl.a)*glm::dot(n, x[i] + dx[i] - pl.a) < 0.0){
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


		x[i] = x[i] + dx[i] * t + (1-t)*glm::reflect(dx[i],n);
		dx[i] = dx[i] - 1.4f*glm::dot(dx[i],n)*n;

		return true;
	}
	return false;
}

void WaterSim::updatex(){
	for(int i=0; i<N; i++){
		bool hasUpdated = collide(i,b[0]) || collide(i,b[1]);
		if(hasUpdated == false){
			x[i] += dx[i];
		}
		if(x[i].x > 10) x[i].x -= 20.0;
		if(x[i].x < -10) x[i].x += 20.0;
		if(x[i].y < -1) x[i].y += 10.0;
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
		drawTetrahedron();
}

void WaterSim::draw(Shader s){
	for(int i=0; i<N; i++){
		drawParticle(i, s);
	}
}



