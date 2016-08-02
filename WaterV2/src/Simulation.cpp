




using namespace Water;

GLfloat const PI = 3.14159265;

Simulation::Simulation(size_t particles){
	N = particles;

	x = new glm::vec3[N];
	dx = new glm::vec3[N];
	dens = new GLfloat[N];

	tmpdx = new glm::vec3[N];
	tmpdens = new GLfloat[N];
}

void Simulation::step(){
	applyForces();

	moveParticles();
}

void applyForces(){
	//Compute new density
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
		for(int j=0; j<N; j++){
			//Forces are computed for one time step and therefore
			//we always multiply with dt.

			//Gravitational force
			tmpdx[i].y += dt * g;

			//Presure force
			tmpdx[i] += - dt * kappa * (dens[j] - dens_0) / dens[j] * dkernel(x[i] - x[j], kernelRadius);

			//Viscosity force
			tmpdx[i] += dt * mu * dx[j] / dens[j] * ddkernel(x[i] - x[j], kernelRadius);
		}
	}
}

//Kernel specifically for the presure force
glm::vec3 dkernel(glm::vec3 r, GLfloat h){
	return -r*(45.0f*pow(h - glm::length(r),2)) / (PI * pow(h,6) * glm::length(r));
}

//Kernel specifically for the viscosity force
GLfloat ddkernel(glm::vec3 r, GLfloat h){
	return 45.0f * (h - glm::length(r)) / (PI * pow(h,6));
}










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
