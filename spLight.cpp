// Include standard headers
#include <stdio.h>
#include <stdlib.h>
// Include GLEW. Always include it before gl.h and glfw3.h, since it's a bit magic.
#include <GL/glew.h>
// Include GLFW
#include <GLFW/glfw3.h>
// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <numeric>
using namespace glm;
using namespace std;

#define SCR_WIDTH 800
#define SCR_HEIGHT 600

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

class point {
	public:
		GLfloat x, y, z;
		vector<GLfloat> nx;
		vector<GLfloat> ny;
		vector<GLfloat> nz;
		inline void set_vertex_normal(GLfloat x, GLfloat y, GLfloat z) {
			nx.push_back(x);
			ny.push_back(y);
			nz.push_back(z);
		}
		inline void get_avg_vertex_normal(GLfloat *x, GLfloat *y, GLfloat *z) {
			*x = *y = *z = 0.0f;
			int n = nx.size();
			if (n != 0)
				*x = accumulate(nx.begin(), nx.end(), 0.0) / n;
			n = ny.size();
			if (n != 0)
				*y = accumulate(ny.begin(), ny.end(), 0.0) / n;
			n = nz.size();
			if (n != 0)
				*z = accumulate(nz.begin(), nz.end(), 0.0) / n;
		}
};

class ply_reader {
	private:
		vector<GLfloat> vertex;
		vector<GLushort> indices;
		vector<point> vertices;
		vector<point> points;
		vector<point> normal;
		GLfloat *element_face_points;
		uint32_t total_vertices;
		uint32_t total_faces;
		void cross_product(point *a, point *b, point *result);
		void calculateNormal(point *coord1, point *coord2, point *coord3, point *result);

	public:
		void ply_load(const char *path);
		vector<point> get_element_face_points();
		vector<GLfloat> get_vertex();
		vector<GLushort> get_indices();
		vector<point> get_normal_vector();
};

vector<point> ply_reader::get_element_face_points() {
	return points;
}

void ply_reader::cross_product(point *a, point *b, point *result)
{
	result->x = a->y * b->z - a->z * b->y;
	result->y = -(a->x * b->z - a->z * b->x);
	result->z = a->x * b->y - a->y * b->x;
	GLfloat length = sqrt(result->x * result->x + result->y * result->y + result->z * result->z);
#ifndef NO_NORMALIZE
	result->x = result->x / length;
	result->y = result->y / length;
	result->z = result->z / length;
#endif
}

void ply_reader::calculateNormal(point *coord1, point *coord2, point *coord3, point *result)
{
	/* calculate Vector1 and Vector2 */
	point va, vb, vr;
	float val;

	va.x = (coord2->x - coord1->x);
	va.y = (coord2->y - coord1->y);
	va.z = (coord2->z - coord1->z);

	vb.x = (coord3->x - coord1->x);
	vb.y = (coord3->y - coord1->y);
	vb.z = (coord3->z - coord1->z);
	cross_product(&va, &vb, result);
}

vector<GLfloat> ply_reader::get_vertex()
{
	return vertex;
}

vector<GLushort> ply_reader::get_indices()
{
	return indices;
}

vector<point> ply_reader::get_normal_vector() {
	return normal;
}

void ply_reader::ply_load(const char *file_path)
{
	if (file_path == NULL)
		return;
	std::ifstream file(file_path);
	string str;
	/* Parse Header */
	while (getline(file, str)) {
		cout << "Line: " << str << endl;
		cout << str.find("element vertex") << endl;
		if (str.find("element vertex") != string::npos) {
			const char *line = str.c_str();
			char ig1[20], ig2[20];
			sscanf(line, "%s %s %d", ig1, ig2, &this->total_vertices);
			cout << "Total Vertex " << this->total_vertices << endl; 
		}
		if (str.find("element face") != string::npos) {
			const char *line = str.c_str();
			char ig1[20], ig2[20];
			sscanf(line, "%s %s %d", ig1, ig2, &this->total_faces);
			cout << "Total faces " << this->total_faces << endl; 
		}
		if (str.find("end_header") != string::npos) {
			break;
		}
	}
	int i = 0;
	for (i = 0; i < this->total_vertices; i++) {
		getline(file, str);
		const char *line = str.c_str();
		point p;
		sscanf(line, "%f %f %f", &p.x, &p.y, &p.z);
		vertices.push_back(p);
		vertex.push_back(p.x);
		vertex.push_back(p.y);
		vertex.push_back(p.z);
	}
	for (i = 0; i < total_faces; i++) {
		getline(file, str);
		const char *line = str.c_str();
		int num, v1,v2, v3,v4;
		if (line[0] == '4')
			sscanf(line, "%d %d %d %d %d", &num, &v1, &v2, &v3, &v4);
		else
			sscanf(line, "%d %d %d %d", &num, &v1, &v2, &v3);
		point normalv;
		calculateNormal(&vertices[v1], &vertices[v2], &vertices[v3], &normalv);
		this->normal.push_back(normalv);
		vertices[v1].set_vertex_normal(normalv.x, normalv.y, normalv.z);
		vertices[v2].set_vertex_normal(normalv.x, normalv.y, normalv.z);
		vertices[v3].set_vertex_normal(normalv.x, normalv.y, normalv.z);
		points.push_back(vertices[v1]);
		points.push_back(vertices[v2]);
		points.push_back(vertices[v3]);
		indices.push_back(v1);
		indices.push_back(v2);
		indices.push_back(v3);
		if (line[0] == '4') {
			vertices[v4].set_vertex_normal(normalv.x, normalv.y, normalv.z);
			points.push_back(vertices[v4]);
			indices.push_back(v4);
		}
	}
}

GLuint LoadShaders(const char * vertex_file_path,const char * fragment_file_path){

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

int main(){
	// Initialise GLFW
	glewExperimental = true; // Needed for core profile
	if( !glfwInit() )
	{
		fprintf( stderr, "Failed to initialize GLFW\n" );
		return -1;
	}
	glfwWindowHint(GLFW_SAMPLES, 4); // 4x antialiasing
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3); // We want OpenGL 3.3
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy; should not be needed
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // We don't want the old OpenGL 

	// Open a window and create its OpenGL context
	GLFWwindow* window; // (In the accompanying source code, this variable is global for simplicity)
	window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Tutorial 01", NULL, NULL);
	if( window == NULL ){
		fprintf( stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n" );
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window); // Initialize GLEW
	glewExperimental=true; // Needed in core profile
	if (glewInit() != GLEW_OK) {
		fprintf(stderr, "Failed to initialize GLEW\n");
		return -1;
	}
	glCheckError();
	// Ensure we can capture the escape key being pressed below
	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
	glEnable(GL_DEPTH_TEST);
	glCheckError();
	// Create and compile our GLSL program from the shaders
	GLuint programID = LoadShaders( "SimpleVertexShader.vertexshader", "SimpleFragmentShader.fragmentshader" );
	GLuint programIDLight = LoadShaders( "lamp.vs", "lamp.fs" );

	glCheckError();
	ply_reader ply;
	//ply.ply_load("tetra.ply");
	ply.ply_load("sphere.ply");

	vector<point> points = ply.get_element_face_points();
#if 1
float vertices[] = {
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
         0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,

        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
         0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,

        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
        -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
        -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,

         0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
         0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
         0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
         0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
         0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
         0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,

        -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
         0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
         0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
         0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,

        -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f
    };
#endif
	vector<GLfloat> vertex;
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

	unsigned int VBO, VAO, EBO;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glBindVertexArray(VAO);
	glCheckError();

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, vertex.size() * sizeof(GLfloat), &vertex[0], GL_STATIC_DRAW);
	//glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	glCheckError();

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glCheckError();
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_TRUE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
	glCheckError();
	glEnableVertexAttribArray(0);
	glCheckError();
	glEnableVertexAttribArray(1);
	glCheckError();

	// note that this is allowed, the call to glVertexAttribPointer registered VBO as the vertex attribute's bound vertex buffer object so afterwards we can safely unbind
	glBindBuffer(GL_ARRAY_BUFFER, 0); 

	// remember: do NOT unbind the EBO while a VAO is active as the bound element buffer object IS stored in the VAO; keep the EBO bound.
	//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	// You can unbind the VAO afterwards so other VAO calls won't accidentally modify this VAO, but this rarely happens. Modifying other
	// VAOs requires a call to glBindVertexArray anyways so we generally don't unbind VAOs (nor VBOs) when it's not directly necessary.
	glBindVertexArray(0);

	// second, configure the light's VAO (VBO stays the same; the vertices are the same for the light object which is also a 3D cube)
	unsigned int lightVAO;
	glGenVertexArrays(1, &lightVAO);
	glBindVertexArray(lightVAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	// note that we update the lamp's position attribute's stride to reflect the updated buffer data
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glCheckError();
	GLfloat rot = 45.0f;
	glm::vec3 LightPos(10.2f, -10.0f, -10.0f);
	do {
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glUseProgram(programID);

		glCheckError();
		// Projection matrix : 45° Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
		glm::mat4 Projection = glm::perspective(glm::radians(45.0f), (float) SCR_WIDTH/ (float) SCR_HEIGHT, 0.1f, 100.0f);

		glCheckError();
		// Or, for an ortho camera :
		//glm::mat4 Projection = glm::ortho(-10.0f,10.0f,-10.0f,10.0f,0.0f,100.0f); // In world coordinates

		// Camera matrix
			
		glm::mat4 View = glm::lookAt(
				glm::vec3(0,0,3), // Camera is at (4,3,3), in World Space
				glm::vec3(0,0,0), // and looks at the origin
				glm::vec3(0,1,0)  // Head is up (set to 0,-1,0 to look upside-down)
				);
		glCheckError();
		// Model matrix : an identity matrix (model will be at the origin)
		glm::mat4 Model = glm::mat4(1.0f);
		glm::vec3 scale = glm::vec3(0.005f, 0.005f, 0.005f);
		Model = glm::scale(Model, scale);
		//Model = glm::rotate(Model, glm::radians(45.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		//Model = glm::rotate(Model, glm::radians(rot), glm::vec3(0.0f, 0.0f, 1.0f));
		glCheckError();
		rot = rot + 1;
		if (rot > 360)
			rot = 45.0f;
		// Our ModelViewProjection : multiplication of our 3 matrices
		glm::mat4 mvp = Projection * View * Model; // Remember, matrix multiplication is the other way around
		//glm::mat4 mvp = Model; // Remember, matrix multiplication is the other way around

		glCheckError();
		GLuint MatrixID = glGetUniformLocation(programID, "MVP");
		glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &mvp[0][0]);
		GLuint ModelID = glGetUniformLocation(programID, "model");
		glUniformMatrix4fv(ModelID, 1, GL_FALSE, &Model[0][0]);
		GLuint ViewID = glGetUniformLocation(programID, "view");
		glUniformMatrix4fv(ViewID, 1, GL_FALSE, &View[0][0]);
		GLuint ProjectionID = glGetUniformLocation(programID, "projection");
		glUniformMatrix4fv(ProjectionID, 1, GL_FALSE, &Projection[0][0]);
		glCheckError();
		glm::vec3 ObjectColor(1.0f, 0.0f, 0.0f);
		GLuint ObjectColorID = glGetUniformLocation(programID, "objectColor");
		glUniform3fv(ObjectColorID, 1, &ObjectColor[0]);
		glm::vec3 LightColor(1.0f, 1.0f, 1.0f);
		GLuint lightColorID = glGetUniformLocation(programID, "lightColor");
		glUniform3fv(lightColorID, 1, glm::value_ptr(LightColor));
		glCheckError();
#if 1
		LightPos.x = sin(glfwGetTime()) * 3.0f;
        	LightPos.z = cos(glfwGetTime()) * 2.0f;
		LightPos.y = 5.0 + cos(glfwGetTime()) * 1.0f;
#endif
		GLuint lightPosID = glGetUniformLocation(programID, "lightPos");
		glUniform3fv(lightPosID, 1, &LightPos[0]);

		glCheckError();
		glBindVertexArray(VAO); // seeing as we only have a single VAO there's no need to bind it every time, but we'll do so to keep things a bit more organized
		glCheckError();
		glDrawArrays(GL_TRIANGLES, 0, vertex.size());
		//glDrawArrays(GL_TRIANGLES, 0, 36);
		glCheckError();
#if 0
		glUseProgram(programIDLight);
		Model = glm::mat4(1.0f);
       		Model = glm::translate(Model, LightPos);
        	Model = glm::scale(Model, glm::vec3(0.9f)); // a smaller cube
        	//lampShader.setMat4("model", model);
		GLuint ModelID1 = glGetUniformLocation(programID, "model");
		glUniformMatrix4fv(ModelID1, 1, GL_FALSE, &Model[0][0]);
		GLuint ViewID1 = glGetUniformLocation(programID, "view");
		glUniformMatrix4fv(ViewID1, 1, GL_FALSE, &View[0][0]);
		GLuint ProjectionID1 = glGetUniformLocation(programIDLight, "projection");
		glUniformMatrix4fv(ProjectionID1, 1, GL_FALSE, &Projection[0][0]);


        	glBindVertexArray(lightVAO);
        	//glDrawArrays(GL_TRIANGLES, 0, vertex.size());	
        	glDrawArrays(GL_TRIANGLES, 0, 36);	
#endif
		glfwSwapBuffers(window);
		glfwPollEvents();
		//break;

	} // Check if the ESC key was pressed or the window was closed
	while( glfwGetKey(window, GLFW_KEY_ESCAPE ) != GLFW_PRESS &&
			glfwWindowShouldClose(window) == 0 );

	glDeleteProgram(programID);

	glfwTerminate();
	return 0;
}

