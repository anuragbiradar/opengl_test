#include <iostream>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <fstream>
#include <sstream>
#include <math.h>

#include "render.h"
using namespace std;

#define PI 3.14159265

glm::vec3 cameraPosition1(4.0f, 4.0f, -60.0f); 

glm::vec3 positions[] = {
	glm::vec3(10.0f, 10.0f, -40.3f),
	glm::vec3(10.0f, 10.0f, -40.3f),
};

glm::vec3 spherePosition[] = {
	// Middle Column Bottom First
	glm::vec3(0.0f, -2.0f, 0.0f),
	glm::vec3(0.0f,  0.0f, 0.0f),
	glm::vec3(0.0f,  2.0f, 0.0f),
	// Last Column Bottom First
	glm::vec3(-2.0f, -2.0f, 0.0f),
	glm::vec3(-2.0f,  0.0f, 0.0f),
	glm::vec3(-2.0f,  2.0f, 0.0f),
	// First Column Bottom First
	glm::vec3(2.0f, -2.0f, 0.0f),
	glm::vec3(2.0f,  0.0f, 0.0f),
	glm::vec3(2.0f,  2.0f, 0.0f)
};

GLenum glCheckError_(const char *file, int line)
{
	GLenum errorCode;
	while ((errorCode = glGetError()) != GL_NO_ERROR)
	{
		std::string error;
		switch (errorCode)
		{
			case GL_INVALID_ENUM:                  error = "INVALID_ENUM"; break;
			case GL_INVALID_VALUE:                 error = "INVALID_VALUE"; break;
			case GL_INVALID_OPERATION:             error = "INVALID_OPERATION"; break;
			case GL_STACK_OVERFLOW:                error = "STACK_OVERFLOW"; break;
			case GL_STACK_UNDERFLOW:               error = "STACK_UNDERFLOW"; break;
			case GL_OUT_OF_MEMORY:                 error = "OUT_OF_MEMORY"; break;
			case GL_INVALID_FRAMEBUFFER_OPERATION: error = "INVALID_FRAMEBUFFER_OPERATION"; break;
		}
		std::cout << error << " | " << file << " (" << line << ")" << std::endl;
	}
	return errorCode;
}
#define glCheckError() glCheckError_(__FILE__, __LINE__) 

render::render() {
	moveObject = 0.0f;
	sphereProgramId = 0;
	sphereVertexArrayObject = 0;
	sphereVertexBufferObject = 0;
}

render::~render() {
	glDeleteBuffers(1, &sphereVertexArrayObject);
	glDeleteBuffers(1, &sphereVertexBufferObject);
	glDeleteProgram(sphereProgramId);
}

void render::initRender(int width, int height, ply_parser *parser) {
	sphereProgramId = LoadShaders("data/sphere.vs", "data/sphere.fs");
	SCR_WIDTH = width;
	SCR_HEIGHT = height;
	glEnable(GL_DEPTH_TEST);
	/* Build Vertex vector */
	points = parser->get_element_face_points();
	for (int i = 0; i < points.size(); i++) {
		vertex.push_back(points[i].x);
		vertex.push_back(points[i].y);
		vertex.push_back(points[i].z);
		GLfloat x, y, z;
		points[i].get_avg_vertex_normal(&x, &y, &z);
		vertex.push_back(x);
		vertex.push_back(y);
		vertex.push_back(z);
		// Calculate UV
#if 0
		float u = asin(x)/PI + 0.5;
		float v = asin(y)/PI + 0.5;
		cout << "u v " << u << " " << v << endl;
		vertex.push_back(u);
		vertex.push_back(v);
#endif
		glm::vec3 vert = glm::vec3(points[i].x, points[i].y, points[i].z);
		float theta = atan2(vert.z, vert.x);
		float u = (theta + PI) / (2 * PI);
		float phi = acos(vert.y / glm::length(vert));
		float v = phi / PI;
		vertex.push_back(u);
		vertex.push_back(v);
	}

	// Sphere Data
	glGenVertexArrays(1, &sphereVertexArrayObject);
	glGenBuffers(1, &sphereVertexBufferObject);
	glBindVertexArray(sphereVertexArrayObject);

	glBindBuffer(GL_ARRAY_BUFFER, sphereVertexBufferObject);
	// Add texture
	glBufferData(GL_ARRAY_BUFFER, vertex.size() * sizeof(float), &vertex[0], GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(3 * sizeof(float)));
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(6 * sizeof(float)));

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	diffuseMap = loadTexture("data/container2.png");	
	diffuseMapSphere = loadTexture("data/earth.png");	
	//diffuseMapSphere = loadTexture("data/chess.png");	
	return;
}

void render::setView1() {
	projectionMatrix1 = glm::perspective(glm::radians(10.0f), (float) SCR_WIDTH/ (float) SCR_HEIGHT, 0.1f, 100.0f);

	//projectionMatrix = glm::ortho(-100.0f,100.0f,-100.0f,100.0f,0.0f,50.0f); // In world coordinates
	glCheckError();

	viewMatrix1 = glm::lookAt(
			cameraPosition1, // Camera is at (4,3,3), in World Space
			glm::vec3(0,0,0), // and looks at the origin
			glm::vec3(0,1,0)  // Head is up (set to 0,-1,0 to look upside-down)
			);
	GLuint viewID = glGetUniformLocation(sphereProgramId, "view");
	glUniformMatrix4fv(viewID, 1, GL_FALSE, &viewMatrix1[0][0]);

	GLuint projectionID = glGetUniformLocation(sphereProgramId, "projection");
	glUniformMatrix4fv(projectionID, 1, GL_FALSE, &projectionMatrix1[0][0]);

	glm::vec3 ObjectColor(1.0f, 0.0f, 0.0f);
	GLuint ObjectColorID = glGetUniformLocation(sphereProgramId, "objectColor");
	glUniform3fv(ObjectColorID, 1, &ObjectColor[0]);

	glm::vec3 LightColor(1.0f, 1.0f, 1.0f);
	GLuint lightColorID = glGetUniformLocation(sphereProgramId, "lightColor");
	glUniform3fv(lightColorID, 1, glm::value_ptr(LightColor));

	glBindVertexArray(sphereVertexArrayObject);
}

unsigned int render::loadTexture(char const * path)
{
	unsigned int textureID;
	glGenTextures(1, &textureID);

	int width, height, nrComponents;
	unsigned char *data = stbi_load(path, &width, &height, &nrComponents, 0);
	if (data)
	{
		GLenum format;
		if (nrComponents == 1)
			format = GL_RED;
		else if (nrComponents == 3)
			format = GL_RGB;
		else if (nrComponents == 4)
			format = GL_RGBA;

		glBindTexture(GL_TEXTURE_2D, textureID);
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		stbi_image_free(data);
	}
	else
	{
		std::cout << "Texture failed to load at path: " << path << std::endl;
		stbi_image_free(data);
	}

	return textureID;
}

void render::drawSphere(int sphereIndex, objectAttributes attr) {

	if (attr.translate != MOVE_INVALID) {
		if (attr.translate == MOVE_LEFT) {
			spherePosition[sphereIndex].x += 1.0f;
		} else if (attr.translate == MOVE_RIGHT) {
			spherePosition[sphereIndex].x -= 1.0f;
		} else if (attr.translate == MOVE_UP) {
			spherePosition[sphereIndex].y += 1.0f;
		} else {
			spherePosition[sphereIndex].y -= 1.0f;
		}
	}

	// Model matrix : an identity matrix (model will be at the origin)
	glm::mat4 modelMatrix = glm::translate(glm::mat4(1.0f), spherePosition[sphereIndex]);
	glm::vec3 scale = glm::vec3(0.05f, 0.05f, 0.05f);
	modelMatrix = glm::scale(modelMatrix, scale);
	modelMatrix = glm::rotate(modelMatrix, glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	static float rot = 0.0f;
	if (attr.axis != ROTATE_INVALID)
	{
		if (attr.axis == ROTATE_X)
			modelMatrix = glm::rotate(modelMatrix, glm::radians(rot), glm::vec3(1.0f, 0.0f, 0.0f));
		if (attr.axis == ROTATE_Y)
			modelMatrix = glm::rotate(modelMatrix, glm::radians(rot), glm::vec3(0.0f, 1.0f, 0.0f));
		if (attr.axis == ROTATE_Z)
			modelMatrix = glm::rotate(modelMatrix, glm::radians(rot), glm::vec3(0.0f, 0.0f, 1.0f));
		rot = rot + 3;
		prev_axis = attr.axis;
	} else {
		if (prev_axis == ROTATE_X)
			modelMatrix = glm::rotate(modelMatrix, glm::radians(rot), glm::vec3(1.0f, 0.0f, 0.0f));
		if (prev_axis == ROTATE_Y)
			modelMatrix = glm::rotate(modelMatrix, glm::radians(rot), glm::vec3(0.0f, 1.0f, 0.0f));
		if (prev_axis == ROTATE_Z)
			modelMatrix = glm::rotate(modelMatrix, glm::radians(rot), glm::vec3(0.0f, 0.0f, 1.0f));
	}
	if (rot > 360)
		rot = 0.0f;
	glCheckError();

	GLuint lightPosID0 = glGetUniformLocation(sphereProgramId, "lightPos");
	glUniform3fv(lightPosID0, 1, &positions[attr.lightPositionIndex][0]);

	GLuint lightPosID1 = glGetUniformLocation(sphereProgramId, "lightPos1");
	glUniform3fv(lightPosID1, 1, &positions[attr.lightPositionIndex+1][0]);

	GLuint diffusedStrengthID = glGetUniformLocation(sphereProgramId, "diffuseStrength");
	glUniform1f(diffusedStrengthID, attr.diffuseStrength);

	GLuint specularStrengthID = glGetUniformLocation(sphereProgramId, "specularStrength");
	glUniform1f(specularStrengthID, attr.specularStrength);

	GLuint modelID = glGetUniformLocation(sphereProgramId, "model");
	glUniformMatrix4fv(modelID, 1, GL_FALSE, &modelMatrix[0][0]);

	GLuint cameraPositionID = glGetUniformLocation(sphereProgramId, "cameraPosition");
	glUniform3fv(cameraPositionID, 1, &cameraPosition1[0]);

	glm::mat4 mvp = projectionMatrix1 * viewMatrix1 * modelMatrix;
	GLuint mvpID = glGetUniformLocation(sphereProgramId, "MVP");
	glUniformMatrix4fv(mvpID, 1, GL_FALSE, &mvp[0][0]);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, diffuseMapSphere);

	glDrawArrays(GL_TRIANGLES, 0, vertex.size());
}

void render::drawSpheres(rotationAxis axis, objectDirection translate) {

	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUseProgram(sphereProgramId);
	GLuint textureID  = glGetUniformLocation(sphereProgramId, "myTextureSampler");
	glUniform1i(textureID, 0);
	glCheckError();
	setView1();
	objectAttributes attr = {
		.diffuseStrength = 0.0,
		.specularStrength = 0.0,
		.axis = axis,
		.translate = translate,
		.lightPositionIndex = 0,
	};

	// (Middle Column) Column wise First is bottom
	attr.diffuseStrength = 0.0; attr.specularStrength = 0.5;
	drawSphere(1, attr);
}

GLuint render::LoadShaders(const char * vertex_file_path,const char * fragment_file_path){
	// Create the shaders
	GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

	// Read the Vertex Shader code from the file
	std::string VertexShaderCode;
	std::ifstream VertexShaderStream(vertex_file_path, std::ios::in);
	if(VertexShaderStream.is_open()){
		std::stringstream sstr;
		sstr << VertexShaderStream.rdbuf();
		VertexShaderCode = sstr.str();
		VertexShaderStream.close();
	}else{
		printf("Impossible to open %s. Are you in the right directory ? Don't forget to read the FAQ !\n", vertex_file_path);
		getchar();
		return 0;
	}

	// Read the Fragment Shader code from the file
	std::string FragmentShaderCode;
	std::ifstream FragmentShaderStream(fragment_file_path, std::ios::in);
	if(FragmentShaderStream.is_open()){
		std::stringstream sstr;
		sstr << FragmentShaderStream.rdbuf();
		FragmentShaderCode = sstr.str();
		FragmentShaderStream.close();
	}

	GLint Result = GL_FALSE;
	int InfoLogLength;

	// Compile Vertex Shader
	printf("Compiling shader : %s\n", vertex_file_path);
	char const * VertexSourcePointer = VertexShaderCode.c_str();
	glShaderSource(VertexShaderID, 1, &VertexSourcePointer , NULL);
	glCompileShader(VertexShaderID);

	// Check Vertex Shader
	glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if ( InfoLogLength > 0 ){
		std::vector<char> VertexShaderErrorMessage(InfoLogLength+1);
		glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
		printf("%s\n", &VertexShaderErrorMessage[0]);
	}

	// Compile Fragment Shader
	printf("Compiling shader : %s\n", fragment_file_path);
	char const * FragmentSourcePointer = FragmentShaderCode.c_str();
	glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer , NULL);
	glCompileShader(FragmentShaderID);

	// Check Fragment Shader
	glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if ( InfoLogLength > 0 ){
		std::vector<char> FragmentShaderErrorMessage(InfoLogLength+1);
		glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
		printf("%s\n", &FragmentShaderErrorMessage[0]);
	}

	// Link the program
	printf("Linking program\n");
	GLuint ProgramID = glCreateProgram();
	glAttachShader(ProgramID, VertexShaderID);
	glAttachShader(ProgramID, FragmentShaderID);
	glLinkProgram(ProgramID);

	// Check the program
	glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
	glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if ( InfoLogLength > 0 ){
		std::vector<char> ProgramErrorMessage(InfoLogLength+1);
		glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
		printf("%s\n", &ProgramErrorMessage[0]);
	}

	glDetachShader(ProgramID, VertexShaderID);
	glDetachShader(ProgramID, FragmentShaderID);

	glDeleteShader(VertexShaderID);
	glDeleteShader(FragmentShaderID);

	return ProgramID;
}
