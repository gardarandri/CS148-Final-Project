#include <iostream>
#include <cmath>

// GLEW
#define GLEW_STATIC
#include <GL/glew.h>

// GLFW
#include <GLFW/glfw3.h>

//SOIL
#include <SOIL.h>

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
const GLuint WIDTH = 1800, HEIGHT = 1000;

//Collision render info
GLuint collisionVBO, collisionVAO, collisionVBOnormals;

// Camera
Camera  camera(glm::vec3(4.32825,-3.28939,9.06154));
GLfloat lastX  =  WIDTH  / 2.0;
GLfloat lastY  =  HEIGHT / 2.0;
bool    keys[1024];

// Light attributes
glm::vec3 lightPos(3.0f, 1.0f, 0.0f);

// Deltatime
GLfloat deltaTime = 0.0f;	// Time between current frame and last frame
GLfloat lastFrame = 0.0f;  	// Time of last frame

//Recording
bool isRecording = false;

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
	//glEnable (GL_BLEND);
	//glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Build and compile our shader program
	Shader domeShader("shaders/phong.vs", "shaders/phong.frag");
	Shader waterShader("shaders/water.vs", "shaders/water.frag");
	Shader treeShader("shaders/tree.vs", "shaders/tree.frag");
	Shader rockShader("shaders/rock.vs", "shaders/rock.frag");
	Shader planeShader("shaders/plane.vs", "shaders/plane.frag");

	Sphere sphere(50, 0.1f);

	Model treesObj("/home/gardar/Downloads/Blend in Pieces/Blend in Pieces/Highest.obj");
	Model sphereObj("/home/gardar/Downloads/Blend in Pieces/Blend in Pieces/Sphere.obj");
	Model rockObj("/home/gardar/Downloads/Blend in Pieces/Blend in Pieces/OnlyRocks.obj");

	//Water spawn spot
	//1.40767 -1.19419 -4.87515
	Simulation watersim(3000);

	GLfloat PI = 3.14159265;
	watersim.addPlane(glm::scale(glm::rotate(glm::translate(glm::mat4(1.0), glm::vec3(0.0,-4.0,2.0)),0.0f,glm::vec3(1.0,0.0,0.1)),glm::vec3(20.0,20.0,20.0)));

	watersim.addPlane(glm::scale(glm::rotate(glm::translate(glm::mat4(1.0), glm::vec3(1.44,-1.28,-7.67)),0.10f,glm::vec3(1.0,0.0,0.1)),glm::vec3(2.0,2.0,5.0)));

	watersim.addPlane(glm::scale(glm::rotate(glm::translate(glm::mat4(1.0), glm::vec3(-0.53,-2.43,-2.0)),-PI/2.1f,glm::vec3(0.0,0.0,1.0)),glm::vec3(2.0,2.0,2.0)));
	watersim.addPlane(glm::scale(glm::rotate(glm::translate(glm::mat4(1.0), glm::vec3(-0.53+0.3,-3.43+0.3,-2.0)),-PI/4.1f,glm::vec3(0.0,0.0,1.0)),glm::vec3(2.0,2.0,2.0)));
	watersim.addPlane(glm::scale(glm::rotate(glm::translate(glm::mat4(1.0), glm::vec3(3.32,-2.43,-2.0)),PI/2.0f,glm::vec3(0.0,0.0,1.0)),glm::vec3(2.0,2.0,2.0)));

	watersim.addPlane(glm::scale(glm::rotate(glm::translate(glm::mat4(1.0), glm::vec3(1.43,-3.81,-2.16)),PI/2.5f,glm::vec3(1.0,0.0,0.0)),glm::vec3(2.0,2.0,2.0)));
	watersim.addPlane(glm::scale(glm::rotate(glm::translate(glm::mat4(1.0), glm::vec3(1.43,0.2,-8.16)),PI/2.0f,glm::vec3(1.0,0.0,0.0)),glm::vec3(2.0,2.0,2.0)));

	watersim.addPlane(glm::scale(glm::rotate(glm::rotate(glm::translate(glm::mat4(1.0), glm::vec3(3.06,-0.99,-5.53)),0.0f,glm::vec3(0.0,1.0,0.0)),PI/2.0f,glm::vec3(0.0,0.0,1.0)),glm::vec3(2.0,2.0,4.0)));
	watersim.addPlane(glm::scale(glm::rotate(glm::rotate(glm::translate(glm::mat4(1.0), glm::vec3(0.55,-1.44,-5.56)),0.0f,glm::vec3(0.0,1.0,0.0)),PI/2.0f,glm::vec3(0.0,0.0,1.0)),glm::vec3(2.0,2.0,4.0)));
	watersim.addPlane(glm::scale(glm::rotate(glm::rotate(glm::translate(glm::mat4(1.0), glm::vec3(-0.42,-2.43,0.93)),PI/6.0f,glm::vec3(0.0,1.0,0.0)),-PI/2.1f,glm::vec3(0.0,0.0,1.0)),glm::vec3(2.0,2.0,2.0)));
	watersim.addPlane(glm::scale(glm::rotate(glm::rotate(glm::translate(glm::mat4(1.0), glm::vec3(-0.60,-3.46,0.04)),-PI/5.0f,glm::vec3(0.0,1.0,0.0)),-PI/3.2f,glm::vec3(0.0,0.0,1.0)),glm::vec3(1.4,0.4,0.4)));

	watersim.addPlane(glm::scale(glm::rotate(glm::rotate(glm::translate(glm::mat4(1.0), glm::vec3(3.59171,-3.60756,0.89)),PI/10.0f,glm::vec3(0.0,1.0,0.0)),PI/2.0f,glm::vec3(0.0,0.0,1.0)),glm::vec3(2.0,2.0,2.0)));
	watersim.addPlane(glm::scale(glm::rotate(glm::rotate(glm::translate(glm::mat4(1.0), glm::vec3(4.21,-2.87,4.73)),PI/10.0f,glm::vec3(0.0,1.0,0.0)),PI/2.0f,glm::vec3(0.0,0.0,1.0)),glm::vec3(2.0,2.0,2.0)));

	watersim.addPlane(glm::scale(glm::rotate(glm::rotate(glm::translate(glm::mat4(1.0), glm::vec3(3.94,-2.52,2.33)),PI/2.0f+PI/4.0f,glm::vec3(0.0,1.0,0.0)),PI/2.0f,glm::vec3(0.0,0.0,1.0)),glm::vec3(2.0,0.6,0.6)));
	watersim.addPlane(glm::scale(glm::rotate(glm::rotate(glm::translate(glm::mat4(1.0), glm::vec3(4.25,-3.08,5.84)),PI/2.0f+PI/5.0f,glm::vec3(0.0,1.0,0.0)),PI/2.0f,glm::vec3(0.0,0.0,1.0)),glm::vec3(2.0,1.0,0.5)));

	watersim.addPlane(glm::scale(glm::rotate(glm::translate(glm::mat4(1.0), glm::vec3(-0.2,-3.60756,4.83412)),-PI/2.0f,glm::vec3(0.0,0.2,1.0)),glm::vec3(3.0,2.0,3.0)));

	watersim.addPlane(glm::scale(glm::rotate(glm::rotate(glm::translate(glm::mat4(1.0), glm::vec3(2.19,-4.01,5.48+0.1)),-PI/4.0f,glm::vec3(0.0,1.0,0.0)),PI/2.0f,glm::vec3(0.0,0.0,1.0)),glm::vec3(0.2,0.2,0.2)));
	watersim.addPlane(glm::scale(glm::rotate(glm::rotate(glm::translate(glm::mat4(1.0), glm::vec3(2.39+0.1,-4.01,5.48-0.05)),PI/2.0f,glm::vec3(0.0,1.0,0.0)),PI/2.0f,glm::vec3(0.0,0.0,1.0)),glm::vec3(0.2,0.2,0.2)));
	watersim.addPlane(glm::scale(glm::rotate(glm::rotate(glm::translate(glm::mat4(1.0), glm::vec3(2.19+0.6,-4.01,5.48+0.1)),PI/4.0f,glm::vec3(0.0,1.0,0.0)),PI/2.0f,glm::vec3(0.0,0.0,1.0)),glm::vec3(0.2,0.2,0.2)));
	watersim.addPlane(glm::scale(glm::rotate(glm::rotate(glm::translate(glm::mat4(1.0), glm::vec3(2.19+0.3,-3.81,5.48+0.1)),-0.1f,glm::vec3(1.0,0.0,0.0)),0.0f,glm::vec3(0.0,0.0,1.0)),glm::vec3(0.2,0.2,0.2)));

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

	GLuint surfaceVBO, surfaceVBOnormals, surfaceVAO;

	glGenBuffers(1, &surfaceVBO);
	glGenBuffers(1, &surfaceVBOnormals);
	glGenVertexArrays(1, &surfaceVAO);

	GLfloat surfaceSizeLen = 20.0f;
	GLfloat surfaceVertices[] = {
		surfaceSizeLen, -4.0f, surfaceSizeLen,
		surfaceSizeLen, -4.0f, -surfaceSizeLen,
		-surfaceSizeLen, -4.0f, surfaceSizeLen,

		-surfaceSizeLen, -4.0f, surfaceSizeLen,
		-surfaceSizeLen, -4.0f, -surfaceSizeLen,
		surfaceSizeLen, -4.0f, -surfaceSizeLen
	};
	
	GLfloat surfaceN[] = {
		0.0f, 1.0f, 0.0f,
		0.0f, 1.0f, 0.0f,
		0.0f, 1.0f, 0.0f,
		0.0f, 1.0f, 0.0f,
		0.0f, 1.0f, 0.0f,
		0.0f, 1.0f, 0.0f
	};

	glBindVertexArray(surfaceVAO);
		glBindBuffer(GL_ARRAY_BUFFER, surfaceVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(surfaceVertices), surfaceVertices, GL_STATIC_DRAW);

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3*sizeof(GLfloat), (GLvoid*)0);
		glEnableVertexAttribArray(0);

		glBindBuffer(GL_ARRAY_BUFFER, surfaceVBOnormals);
		glBufferData(GL_ARRAY_BUFFER, sizeof(surfaceN), surfaceN, GL_STATIC_DRAW);

		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3*sizeof(GLfloat), (GLvoid*)0);
		glEnableVertexAttribArray(1);
	glBindVertexArray(0);
	

	//cnt for filenames
	int cnt=1;

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


		// Draw the container (using container's vertex attributes)

		configureShader(waterShader);
		for(int i=0; i<watersim.getNumberOfParticles(); i++){
			sphere.draw(waterShader, watersim.getPosition(i), watersim.getVelocity(i));
			//sphere.draw(domeShader, watersim.getPosition(i));
		}
		//configureShader(domeShader);

		glUniform3f(objectColorLoc, 1.0f, 0.5f, 0.31f);
		glm::mat4 model(1.0);
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

		/*
		glBindVertexArray(collisionVAO);
		glDrawArrays(GL_TRIANGLES, 0, watersim.surfaces.size()*3);
		glBindVertexArray(0);
		*/
		
		configureShader(planeShader);
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		glBindVertexArray(surfaceVAO);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		glBindVertexArray(0);

		glm::mat4 sceneModel = glm::translate(glm::mat4(1.0),glm::vec3(-2.0,-4.0,0.0));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(sceneModel));

		configureShader(treeShader);
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(sceneModel));
		treesObj.Draw(treeShader);

		configureShader(domeShader);
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(sceneModel));
		sphereObj.Draw(domeShader);


		configureShader(rockShader);
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(sceneModel));
		rockObj.Draw(rockShader);

		cout<<camera.Position.x<<" "<<camera.Position.y<<" "<<camera.Position.z<<" "<<endl;

		watersim.step();

		// Swap the screen buffers
		glfwSwapBuffers(window);

		if(isRecording == true){
			stringstream ss;
			ss << "movie" << cnt++ << ".bmp";
			string filename = ss.str();
			int save_result = SOIL_save_screenshot(filename.c_str(), SOIL_SAVE_TYPE_BMP, 0, 0, WIDTH, HEIGHT);
		}
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
	if (keys[GLFW_KEY_R])
		isRecording = !isRecording;
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


