#ifndef _RENDER_
#define _RENDER_

#include "ply_parser.h"
#include "FastTrackball.h"

#include <GL/glew.h>
#include <GLFW/glfw3.h>

class render {
	private:
		Trackball wire_mesh_q;
		Trackball filled_mesh_q;
		Trackball cube_mesh_q;
		uint32_t SCR_WIDTH;
		uint32_t SCR_HEIGHT;
		GLuint vertexbuffer;
		GLuint program_id;
		GLuint VertexArrayID;
		vector<point> points;
		uint32_t prev_f;
		vector<GLfloat> vertex;
	public:
		render();
		~render();
		void init_render(int width, int height, ply_parser *parser);
		GLuint LoadShaders(const char *vert_file, const char *frag_file);
		void buildData(ply_parser *parser, int f);
		void buildRecursive(GLfloat *fa, GLfloat *fb, GLfloat *fc, uint32_t f);
		void drawTetra(ply_parser *parser, int f);
};
#endif
