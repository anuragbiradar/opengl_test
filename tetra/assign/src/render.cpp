#include <iostream>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <fstream>
#include <sstream>

#include "render.h"
#include "FastTrackball.h"
using namespace std;

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
	prev_f = UINT_MAX;
}

render::~render() {
	glDeleteBuffers(1, &VertexArrayID);
	glDeleteBuffers(1, &vertexbuffer);
	glDeleteProgram(program_id);
}

void render::init_render(int width, int height, ply_parser *parser) {
	program_id = LoadShaders("data/SimpleVertexShader.vertexshader", "data/SimpleFragmentShader.fragmentshader");
	SCR_WIDTH = width;
	SCR_HEIGHT = height;
	return;
}

void normalize(GLfloat *p)
{
	double d=0.0;
	int i;
	for(i=0;i<3;i++)
		d+=p[i]*p[i];
	d=sqrt(d);
	if(d>0.0) for(i=0;i<3;i++) p[i]/=d;
}

void render::buildRecursive(GLfloat *fa, GLfloat *fb, GLfloat *fc, uint32_t f) {

	GLfloat mid[3][3];
	int j;

	if (f > 0) {
		for(j=0;j<3;j++) mid[0][j]=(fa[j]+fb[j])/2;
		normalize(mid[0]);
		for(j=0;j<3;j++) mid[1][j]=(fa[j]+fc[j])/2;
		normalize(mid[1]);
		for(j=0;j<3;j++) mid[2][j]=(fc[j]+fb[j])/2;
		normalize(mid[2]);

 		buildRecursive(fa, mid[0], mid[1], f-1);
		buildRecursive(fc, mid[1], mid[2], f-1);
		buildRecursive(fb, mid[2], mid[1], f-1);
		buildRecursive(mid[0], mid[2], mid[1], f-1);
	}
	else {
		//cout << "Adding to vector " << endl;
		for (j = 0; j < 3; j++) {
		//	cout << "ADD " << fa[j] << "\n";
			vertex.push_back(fa[j]);
		}
		for (j = 0; j < 3; j++)
			vertex.push_back(fb[j]);
		for (j = 0; j < 3; j++)
			vertex.push_back(fc[j]);
	}
}

void render::buildData(ply_parser *parser, int f) {
	vertex.clear();
	for (int i = 0; i < points.size(); i = i + 3) {
		// get one triangle
		GLfloat faces[3][3];
		faces[0][0] = points[i].x; faces[0][1] = points[i].y; faces[0][2] = points[i].z;
		faces[1][0] = points[i + 1].x; faces[1][1] = points[i + 1].y; faces[1][2] = points[i + 1].z;
		faces[2][0] = points[i + 2].x; faces[2][1] = points[i + 2].y; faces[2][2] = points[i + 2].z;
		buildRecursive(faces[0], faces[1], faces[2], f);
	}
	glCheckError();
	glGenVertexArrays(1, &VertexArrayID);
	glCheckError();
	glBindVertexArray(VertexArrayID);
	glCheckError();
	glGenBuffers(1, &vertexbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	glCheckError();
	GLfloat arr[vertex.size()];
	copy(vertex.begin(), vertex.end(), arr);
	//glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_STATIC_DRAW);
	glBufferData(GL_ARRAY_BUFFER, vertex.size() * sizeof(GLfloat), arr, GL_STATIC_DRAW);

	//glBufferData(GL_ARRAY_BUFFER, vertex.size() * sizeof(GL_FLOAT), &vertex[0], GL_STATIC_DRAW);
	glCheckError();
}

void render::drawTetra(ply_parser *parser, int f) {

	points = parser->get_element_face_points();
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	if (f != prev_f) {
		buildData(parser, f);
		f = prev_f;
	}

	glUseProgram(program_id);
	// 1st attribute buffer : vertices
	// Projection matrix : 45° Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
	glm::mat4 Projection = glm::perspective(glm::radians(45.0f), (float) SCR_WIDTH/ (float) SCR_HEIGHT, 0.1f, 100.0f);

	// Or, for an ortho camera :
	//glm::mat4 Projection = glm::ortho(-10.0f,10.0f,-10.0f,10.0f,0.0f,100.0f); // In world coordinates

	// Camera matrix
	glm::mat4 View = glm::lookAt(
			glm::vec3(4,3,5), // Camera is at (4,3,3), in World Space
			glm::vec3(0,0,0), // and looks at the origin
			glm::vec3(0,1,0)  // Head is up (set to 0,-1,0 to look upside-down)
			);
	// Model matrix : an identity matrix (model will be at the origin)
	glm::mat4 Model = glm::mat4(1.0f);
	static GLfloat sc = 1.0f;
	glm::vec3 scale = glm::vec3(sc, sc, sc);
	//sc = sc + 0.001f;
//	Model = glm::scale(Model, scale);
//	static GLfloat rot = 30.0f;
	Model = glm::rotate(Model, glm::radians(45.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	Model = glm::rotate(Model, glm::radians(60.0f), glm::vec3(0.0f, 0.0f, 1.0f));
#if 0
	Model = glm::rotate(Model, glm::radians(rot), glm::vec3(0.0f, 1.0f, 0.0f));
	if (rot >=360)
		rot = 30;
	rot = rot+2;
#endif
	// Our ModelViewProjection : multiplication of our 3 matrices
	glm::mat4 mvp = Projection * View * Model; // Remember, matrix multiplication is the other way around
	//glm::mat4 mvp = Model; // Remember, matrix multiplication is the other way around

	GLuint MatrixID = glGetUniformLocation(program_id, "MVP");
	glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &mvp[0][0]);

	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);
	//glDrawArrays(GL_TRIANGLES, 0, 12); // Starting from vertex 0; 3 vertices total -> 1 triangle
	glDrawArrays(GL_TRIANGLES, 0, vertex.size()); // Starting from vertex 0; 3 vertices total -> 1 triangle
	glDisableVertexAttribArray(0);
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
