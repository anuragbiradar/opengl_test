#include <iostream>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <fstream>
#include <sstream>

#include "render.h"
#include "FastTrackball.h"
using namespace std;
glm::vec3 LightPos(1.2f, -1.0f, -1.0f);

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
}

render::~render() {
	glDeleteBuffers(1, &VAO);
	glDeleteBuffers(1, &VBO);
	glDeleteProgram(program_id);
}

void render::init_render(int width, int height, ply_parser *parser) {
	program_id = LoadShaders("data/sphere.vs", "data/sphere.fs");
	SCR_WIDTH = width;
	SCR_HEIGHT = height;
	glEnable(GL_DEPTH_TEST);
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
	}

	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glBindVertexArray(VAO);
	glCheckError();

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, vertex.size() * sizeof(GLfloat), &vertex[0], GL_STATIC_DRAW);
	glCheckError();

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glCheckError();
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_TRUE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
	glCheckError();
	glEnableVertexAttribArray(0);
	glCheckError();
	glEnableVertexAttribArray(1);
	glCheckError();

	glBindBuffer(GL_ARRAY_BUFFER, 0); 

	glBindVertexArray(0);
#if 0
	unsigned int lightVAO;
	glGenVertexArrays(1, &lightVAO);
	glBindVertexArray(lightVAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
#endif
	return;
}

void render::setView() {
	Projection = glm::perspective(glm::radians(10.0f), (float) SCR_WIDTH/ (float) SCR_HEIGHT, 0.1f, 100.0f);

	//Projection = glm::ortho(-50.0f,50.0f,-50.0f,50.0f,0.0f,10.0f); // In world coordinates
	glCheckError();

	View = glm::lookAt(
			glm::vec3(10,10,-50), // Camera is at (4,3,3), in World Space
			glm::vec3(0,0,0), // and looks at the origin
			glm::vec3(0,1,0)  // Head is up (set to 0,-1,0 to look upside-down)
			);
	GLuint ViewID = glGetUniformLocation(program_id, "view");
	glUniformMatrix4fv(ViewID, 1, GL_FALSE, &View[0][0]);

	GLuint ProjectionID = glGetUniformLocation(program_id, "projection");
	glUniformMatrix4fv(ProjectionID, 1, GL_FALSE, &Projection[0][0]);
	glCheckError();

	glm::vec3 ObjectColor(1.0f, 0.0f, 0.0f);
	GLuint ObjectColorID = glGetUniformLocation(program_id, "objectColor");
	glUniform3fv(ObjectColorID, 1, &ObjectColor[0]);

	glm::vec3 LightColor(1.0f, 1.0f, 1.0f);
	GLuint lightColorID = glGetUniformLocation(program_id, "lightColor");
	glUniform3fv(lightColorID, 1, glm::value_ptr(LightColor));
	glCheckError();

	glBindVertexArray(VAO); // seeing as we only have a single VAO there's no need to bind it every time, but we'll do so to keep things a bit more organized
}

void render::drawSphere(ply_parser *parser, glm::vec3 translation, bool lightRot) {
	// Model matrix : an identity matrix (model will be at the origin)
	glm::mat4 Model = glm::translate(glm::mat4(1.0f), translation);
	glm::vec3 scale = glm::vec3(0.006f, 0.006f, 0.006f);
	Model = glm::scale(Model, scale);
	//Model = glm::rotate(Model, glm::radians(30.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	glCheckError();

#if 1
	LightPos.x = sin(glfwGetTime()) * 3.0f;
	LightPos.z = cos(glfwGetTime()) * 2.0f;
	LightPos.y = 5.0 + cos(glfwGetTime()) * 1.0f;
	LightPos.x += translation.x;
	LightPos.y += translation.y;
	LightPos.z += translation.z;
#endif
	GLuint lightPosID = glGetUniformLocation(program_id, "lightPos");
	glUniform3fv(lightPosID, 1, &LightPos[0]);

	Model = glm::translate(Model, translation);
	GLuint ModelID = glGetUniformLocation(program_id, "model");
	glUniformMatrix4fv(ModelID, 1, GL_FALSE, &Model[0][0]);
	glDrawArrays(GL_TRIANGLES, 0, vertex.size());
}

void render::drawSpheres(ply_parser *parser, bool lightRot, bool translate) {

	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUseProgram(program_id);
	glCheckError();
	setView();
#if 1
	drawSphere(parser, glm::vec3(0.0f,  -2.0f, 0.0f), lightRot);
	drawSphere(parser, glm::vec3(0.0f,  0.0f, 0.0f), lightRot);
	drawSphere(parser, glm::vec3(0.0f,  2.0f, 0.0f), lightRot);
#endif
	drawSphere(parser, glm::vec3(-2.0f,  -2.0f, 0.0f), lightRot);
	drawSphere(parser, glm::vec3(-2.0f,  0.0f, 0.0f), lightRot);
	drawSphere(parser, glm::vec3(-2.0f,  2.0f, 0.0f), lightRot);
#if 1
	drawSphere(parser, glm::vec3(2.0f,  -2.0f, 0.0f), lightRot);
	drawSphere(parser, glm::vec3(2.0f,  0.0f, 0.0f), lightRot);
	drawSphere(parser, glm::vec3(2.0f,  2.0f, 0.0f), lightRot);
	//drawSphere(parser, glm::vec3(4.0f,  2.0f, 5.0f), lightRot);
#endif
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
