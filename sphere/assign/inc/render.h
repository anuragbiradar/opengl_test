#ifndef _RENDER_
#define _RENDER_

#include "ply_parser.h"
#include "FastTrackball.h"

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

class render {
	private:
		Trackball wire_mesh_q;
		Trackball filled_mesh_q;
		Trackball cube_mesh_q;
		uint32_t SCR_WIDTH;
		uint32_t SCR_HEIGHT;
		GLuint program_id;
		unsigned int VBO, VAO;
		vector<point> points;
		uint32_t prev_f;
		vector<GLfloat> vertex;
		glm::mat4 Projection;
		glm::mat4 View;
	public:
		render();
		~render();
		void init_render(int width, int height, ply_parser *parser);
		GLuint LoadShaders(const char *vert_file, const char *frag_file);
		void setView();
		void drawSphere(ply_parser *parser, glm::vec3 translation, bool lightRot);
		void drawSpheres(ply_parser *parser, bool lightRot, bool translate); 
};
#endif
