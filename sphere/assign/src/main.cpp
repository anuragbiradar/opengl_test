#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "ply_parser.h"
#include "render.h"

#include <iostream>
using namespace std;

typedef enum _mobject_ {
	MESH_OBJECT_1,
	MESH_OBJECT_2,
	MESH_OBJECT_3,
} mesh_object_type;

void handleKeys(GLFWwindow* window, int key, int code, int action, int mode);

class window_mgmt {

	private:
		int translate, lighttranslate0, lighttranslate1;
		mesh_object_type mesh_object;
		GLFWwindow *mainWindow;
		uint8_t parse_top;
		ply_parser *parser[10];
		render render_obj;
	public:
		window_mgmt();
		void window_set_ply_object(ply_parser *parser);
		int window_mainloop_init();
		void window_handle_event(int key, int code, int action, int mode);
		void window_mainloop_run();
};

window_mgmt window_mgmr;

window_mgmt::window_mgmt() {
	translate = lighttranslate0 = lighttranslate1 = 0;
	parse_top = 0;
	mesh_object = MESH_OBJECT_1;
}

void window_mgmt::window_set_ply_object(ply_parser *parser_obj) {
	parser[parse_top++] = parser_obj;
}

int window_mgmt::window_mainloop_init() {
	const GLint WIDTH = 800, HEIGHT = 600;

	// Initializing GLFW
	if(!glfwInit()) {
		cout <<"GLFW initialization failed.";
		glfwTerminate();
		return 0;
	}

	// Setup GLFW window properties
	// OpenGL version
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR,3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR,3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	mainWindow = glfwCreateWindow(WIDTH, HEIGHT, "AssignMent", NULL, NULL);
	if(!mainWindow) {
		cout << "GLFW window creation failed.";
		glfwTerminate();
		return 0;
	}

	int bufferWidth, bufferHeight;
	glfwGetFramebufferSize(mainWindow, &bufferWidth, &bufferHeight);

	// Set context for GLEW to use
	glfwMakeContextCurrent(mainWindow);

	// Allow modern extension features
	glewExperimental = GL_TRUE;

	if(glewInit() != GLEW_OK) {
		cout << "GLEW initialization failed.";
		glfwDestroyWindow(mainWindow);
		glfwTerminate();
		return 0;
	}
	render_obj.init_render(bufferWidth, bufferHeight, parser[0]);
}

void window_mgmt::window_mainloop_run() {
	while(!glfwWindowShouldClose(mainWindow))
  	{
		glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		render_obj.drawSpheres(parser[0], lighttranslate0, lighttranslate1, translate);
		lighttranslate0 = lighttranslate1 = translate = 0;
		glfwSwapBuffers(mainWindow);
		glfwPollEvents();
		glfwSetKeyCallback(mainWindow, handleKeys);
	}
}

void window_mgmt::window_handle_event(int key, int code, int action, int mode) {
	if (key == GLFW_KEY_LEFT && action == GLFW_PRESS) {
		if (mesh_object == MESH_OBJECT_1) translate = -1;
		if (mesh_object == MESH_OBJECT_2) lighttranslate0 = -1;
		if (mesh_object == MESH_OBJECT_3) lighttranslate1 = -1;
	}
  	if (key == GLFW_KEY_RIGHT && action == GLFW_PRESS) {
		if (mesh_object == MESH_OBJECT_1) translate = 1;
		if (mesh_object == MESH_OBJECT_2) lighttranslate0 = 1;
		if (mesh_object == MESH_OBJECT_3) lighttranslate1 = 1;
	}
	if (key == '1' && action == GLFW_PRESS)
		mesh_object = MESH_OBJECT_1;
	if (key == '2' && action == GLFW_PRESS)
		mesh_object = MESH_OBJECT_2;
	if (key == '3' && action == GLFW_PRESS)
		mesh_object = MESH_OBJECT_3;
}

void handleKeys(GLFWwindow* window, int key, int code, int action, int mode)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
	   glfwSetWindowShouldClose(window, GL_TRUE);
  	else
		window_mgmr.window_handle_event(key, code, action, mode);
}

int main(int argc, char *argv[])
{
	ply_parser parser_obj;
	parser_obj.load_ply("data/sphere.ply");

	window_mgmr.window_set_ply_object(&parser_obj);
	window_mgmr.window_mainloop_init();
	window_mgmr.window_mainloop_run();
	return 0;
}
