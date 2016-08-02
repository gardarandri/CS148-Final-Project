#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

#include <GL/glew.h>

#include "Shader.h"

using namespace Water;

void Shader::Use()
{ 
	glUseProgram(this->Program); 
}

