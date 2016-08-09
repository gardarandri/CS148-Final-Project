#include <iostream>
#include <cmath>

// GLEW
#define GLEW_STATIC
#include <GL/glew.h>

// GLFW
#include <GLFW/glfw3.h>

// GLM Mathematics
#define GLM_FORCE_RADIANS // force everything in radian
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// Other includes
#include "Shader.h"
#include "Camera.h"
#include "Sphere.h"
#include "Simulation.h"
#include "model.h"

using namespace Water;

// Function prototypes
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void do_movement();

// Window dimensions
const GLuint WIDTH = 1500, HEIGHT = 1000;

//Collision render info
GLuint collisionVBO, collisionVAO, collisionVBOnormals;

// Camera
Camera  camera(glm::vec3(0.0f, -3.5f, 20.0f));
GLfloat lastX  =  WIDTH  / 2.0;
GLfloat lastY  =  HEIGHT / 2.0;
bool    keys[1024];

// Light attributes
glm::vec3 lightPos(3.0f, 1.0f, 0.0f);

// Deltatime
GLfloat deltaTime = 0.0f;	// Time between current frame and last frame
GLfloat lastFrame = 0.0f;  	// Time of last frame

GLint modelLoc;
GLint viewLoc;
GLint projLoc;
GLint objectColorLoc;
GLint lightColorLoc;
GLint lightPosLoc;
GLint viewPosLoc;
int w,h;

void configureShader(Shader s){
	// Use cooresponding shader when setting uniforms/drawing objects
	s.Use();
	objectColorLoc = glGetUniformLocation(s.Program, "objectColor");
	lightColorLoc  = glGetUniformLocation(s.Program, "lightColor");
	lightPosLoc    = glGetUniformLocation(s.Program, "lightPos");
	viewPosLoc     = glGetUniformLocation(s.Program, "viewPos");
	glUniform3f(objectColorLoc, 1.0f, 0.5f, 0.31f);
	glUniform3f(lightColorLoc,  1.0f, 1.0f, 1.0f);
	glUniform3f(lightPosLoc,    lightPos.x, lightPos.y, lightPos.z);
	glUniform3f(viewPosLoc,     camera.Position.x, camera.Position.y, camera.Position.z);

	// Create camera transformations
	glm::mat4 view;
	view = camera.GetViewMatrix();
	glm::mat4 projection = glm::perspective(camera.Zoom, (GLfloat)w / (GLfloat)h, 0.1f, 100.0f);
	// Get the uniform locations
	modelLoc = glGetUniformLocation(s.Program, "model");
	viewLoc  = glGetUniformLocation(s.Program,  "view");
	projLoc  = glGetUniformLocation(s.Program,  "projection");
	// Pass the matrices to the shader
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
}

// The MAIN function, from here we start the application and run the game loop
int main()
{
	// Init GLFW
	glfwInit();
	// Set all the required options for GLFW
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

	// Create a GLFWwindow object that we can use for GLFW's functions
	GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Water simulation V2", nullptr, nullptr);
	glfwMakeContextCurrent(window);

	// Set the required callback functions
	glfwSetKeyCallback(window, key_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);

	// Set this to true so GLEW knows to use a modern approach to retrieving function pointers and extensions
	glewExperimental = GL_TRUE;
	// Initialize GLEW to setup the OpenGL Function pointers
	glewInit();

	glfwGetFramebufferSize( window, &w, &h);

	// Define the viewport dimensions
	glViewport(0, 0, w, h);

	// OpenGL options
	glEnable(GL_DEPTH_TEST);

	// Build and compile our shader program
	Shader lightingShader("shaders/phong.vs", "shaders/phong.frag");
	Shader waterShader("shaders/water.vs", "shaders/water.frag");

	Sphere sphere(50, 0.1f);

	Model ourModel("/home/gardar/Downloads/Waterfall/Smallersizetrees.obj");
	//Model ourModel("/home/gardar/Documents/Forritun/ICGAI/ModelV1/nanosuit/nanosuit.obj");

	//Water spawn spot
	//1.40767 -1.19419 -4.87515
	Simulation watersim(1000);

	GLfloat PI = 3.14159265;
	watersim.addPlane(glm::scale(glm::rotate(glm::translate(glm::mat4(1.0), glm::vec3(0.0,-4.0,2.0)),0.0f,glm::vec3(1.0,0.0,0.1)),glm::vec3(20.0,20.0,20.0)));

	watersim.addPlane(glm::scale(glm::rotate(glm::translate(glm::mat4(1.0), glm::vec3(1.44,-1.68,-4.67)),0.10f,glm::vec3(1.0,0.0,0.1)),glm::vec3(2.0,2.0,2.0)));

	//watersim.addPlane(glm::scale(glm::rotate(glm::translate(glm::mat4(1.0), glm::vec3(0.0,0.0,-1.0)),PI/2.0f,glm::vec3(1.0,0.0,0.0)),glm::vec3(2.0,2.0,2.0)));
	//watersim.addPlane(glm::scale(glm::rotate(glm::translate(glm::mat4(1.0), glm::vec3(0.0,0.0,1.0)),-PI/2.0f,glm::vec3(1.0,0.0,0.0)),glm::vec3(2.0,2.0,2.0)));

	watersim.addPlane(glm::scale(glm::rotate(glm::translate(glm::mat4(1.0), glm::vec3(-0.53,-2.43,-2.0)),-PI/2.1f,glm::vec3(0.0,0.0,1.0)),glm::vec3(2.0,2.0,2.0)));
	watersim.addPlane(glm::scale(glm::rotate(glm::translate(glm::mat4(1.0), glm::vec3(-0.53,-3.43,-2.0)),-PI/4.1f,glm::vec3(0.0,0.0,1.0)),glm::vec3(2.0,2.0,2.0)));
	watersim.addPlane(glm::scale(glm::rotate(glm::translate(glm::mat4(1.0), glm::vec3(3.32,-2.43,-2.0)),PI/2.0f,glm::vec3(0.0,0.0,1.0)),glm::vec3(2.0,2.0,2.0)));

	watersim.addPlane(glm::scale(glm::rotate(glm::translate(glm::mat4(1.0), glm::vec3(1.43,-3.81,-2.16)),PI/2.5f,glm::vec3(1.0,0.0,0.0)),glm::vec3(2.0,2.0,2.0)));
	watersim.addPlane(glm::scale(glm::rotate(glm::translate(glm::mat4(1.0), glm::vec3(1.43,0.2,-5.16)),PI/2.0f,glm::vec3(1.0,0.0,0.0)),glm::vec3(2.0,2.0,2.0)));

	watersim.addPlane(glm::scale(glm::rotate(glm::rotate(glm::translate(glm::mat4(1.0), glm::vec3(2.26,-0.99,-4.93)),PI/6.0f,glm::vec3(0.0,1.0,0.0)),PI/2.0f,glm::vec3(0.0,0.0,1.0)),glm::vec3(2.0,2.0,2.0)));
	watersim.addPlane(glm::scale(glm::rotate(glm::rotate(glm::translate(glm::mat4(1.0), glm::vec3(-0.51,-1.44,-4.36)),-PI/8.0f,glm::vec3(0.0,1.0,0.0)),PI/2.0f,glm::vec3(0.0,0.0,1.0)),glm::vec3(2.0,2.0,2.0)));
	watersim.addPlane(glm::scale(glm::rotate(glm::rotate(glm::translate(glm::mat4(1.0), glm::vec3(-0.42,-2.43,0.93)),PI/6.0f,glm::vec3(0.0,1.0,0.0)),-PI/2.1f,glm::vec3(0.0,0.0,1.0)),glm::vec3(2.0,2.0,2.0)));
	watersim.addPlane(glm::scale(glm::rotate(glm::rotate(glm::translate(glm::mat4(1.0), glm::vec3(-0.60,-3.46,0.04)),-PI/5.0f,glm::vec3(0.0,1.0,0.0)),-PI/3.2f,glm::vec3(0.0,0.0,1.0)),glm::vec3(1.4,0.4,0.4)));

	watersim.addPlane(glm::scale(glm::rotate(glm::rotate(glm::translate(glm::mat4(1.0), glm::vec3(3.59171,-3.60756,0.89)),PI/10.0f,glm::vec3(0.0,1.0,0.0)),PI/2.0f,glm::vec3(0.0,0.0,1.0)),glm::vec3(2.0,2.0,2.0)));
	watersim.addPlane(glm::scale(glm::rotate(glm::rotate(glm::translate(glm::mat4(1.0), glm::vec3(4.21,-2.87,4.73)),PI/10.0f,glm::vec3(0.0,1.0,0.0)),PI/2.0f,glm::vec3(0.0,0.0,1.0)),glm::vec3(2.0,2.0,2.0)));

	watersim.addPlane(glm::scale(glm::rotate(glm::rotate(glm::translate(glm::mat4(1.0), glm::vec3(3.94,-2.52,2.83)),PI/2.0f+PI/10.0f,glm::vec3(0.0,1.0,0.0)),PI/2.0f,glm::vec3(0.0,0.0,1.0)),glm::vec3(2.0,0.6,0.6)));
	watersim.addPlane(glm::scale(glm::rotate(glm::rotate(glm::translate(glm::mat4(1.0), glm::vec3(4.25,-3.08,5.84)),PI/2.0f+PI/5.0f,glm::vec3(0.0,1.0,0.0)),PI/2.0f,glm::vec3(0.0,0.0,1.0)),glm::vec3(2.0,1.0,0.5)));

	watersim.addPlane(glm::scale(glm::rotate(glm::translate(glm::mat4(1.0), glm::vec3(-0.2,-3.60756,4.83412)),-PI/2.0f,glm::vec3(0.0,0.2,1.0)),glm::vec3(3.0,2.0,3.0)));

	glGenBuffers(1, &collisionVBO);
	glGenBuffers(1, &collisionVBOnormals);
	glGenVertexArrays(1, &collisionVAO);

	std::vector<glm::vec3> surfaceNormals;
	for(int i=0; i<watersim.surfaces.size(); i++){
		Triangle tmp = watersim.surfaces[i];
		glm::vec3 norm = glm::cross(tmp.a - tmp.b, tmp.a - tmp.c);
		surfaceNormals.push_back(norm);
		surfaceNormals.push_back(norm);
		surfaceNormals.push_back(norm);
	}

	glBindVertexArray(collisionVAO);
	glBindBuffer(GL_ARRAY_BUFFER, collisionVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*watersim.surfaces.size()*9, (GLfloat*)watersim.surfaces.data(), GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3*sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, collisionVBOnormals);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*watersim.surfaces.size()*9, (GLfloat*)surfaceNormals.data(), GL_STATIC_DRAW);

	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3*sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(1);
	glBindVertexArray(0);

	// Game loop
	while (!glfwWindowShouldClose(window))
	{
		// Calculate deltatime of current frame
		GLfloat currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		// Check if any events have been activiated (key pressed, mouse moved etc.) and call corresponding response functions
		glfwPollEvents();
		do_movement();

		// Clear the colorbuffer
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		configureShader(waterShader);

		// Draw the container (using container's vertex attributes)

		for(int i=0; i<watersim.getNumberOfParticles(); i++){
			sphere.draw(waterShader, watersim.getPosition(i));
			//sphere.draw(lightingShader, watersim.getPosition(i));
		}
		configureShader(lightingShader);

		glUniform3f(objectColorLoc, 1.0f, 0.5f, 0.31f);
		glm::mat4 model(1.0);
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

		/*
		glBindVertexArray(collisionVAO);
		glDrawArrays(GL_TRIANGLES, 0, watersim.surfaces.size()*3);
		glBindVertexArray(0);
		*/

		glm::mat4 sceneModel = glm::translate(glm::mat4(1.0),glm::vec3(-2.0,-4.0,0.0));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(sceneModel));

		ourModel.Draw(lightingShader);

		//cout<<camera.Position.x<<" "<<camera.Position.y<<" "<<camera.Position.z<<" "<<endl;

		watersim.step();

		// Swap the screen buffers
		glfwSwapBuffers(window);
	}

	// Terminate GLFW, clearing any resources allocated by GLFW.
	glfwTerminate();
	return 0;
}

// Is called whenever a key is pressed/released via GLFW
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);
	if (key >= 0 && key < 1024)
	{
		if (action == GLFW_PRESS)
			keys[key] = true;
		else if (action == GLFW_RELEASE)
			keys[key] = false;
	}
}

void do_movement()
{
	// Camera controls
	if (keys[GLFW_KEY_W])
		camera.ProcessKeyboard(FORWARD, deltaTime);
	if (keys[GLFW_KEY_S])
		camera.ProcessKeyboard(BACKWARD, deltaTime);
	if (keys[GLFW_KEY_A])
		camera.ProcessKeyboard(LEFT, deltaTime);
	if (keys[GLFW_KEY_D])
		camera.ProcessKeyboard(RIGHT, deltaTime);
	if (keys[GLFW_KEY_UP])
		camera.ProcessMouseMovement(0.0, 20.1);
	if (keys[GLFW_KEY_DOWN])
		camera.ProcessMouseMovement(0.0, -20.1);
	if (keys[GLFW_KEY_LEFT])
		camera.ProcessMouseMovement(-20.1, 0.0);
	if (keys[GLFW_KEY_RIGHT])
		camera.ProcessMouseMovement(20.1, 0.0);
}

bool firstMouse = true;
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	GLfloat xoffset = xpos - lastX;
	GLfloat yoffset = lastY - ypos;  // Reversed since y-coordinates go from bottom to left

	lastX = xpos;
	lastY = ypos;

	camera.ProcessMouseMovement(xoffset, yoffset);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	camera.ProcessMouseScroll(yoffset);
}


