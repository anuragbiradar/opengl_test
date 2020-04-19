#ifndef _RENDER_
#define _RENDER_

#include "ply_parser.h"

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

typedef enum _objectDirection {
	MOVE_INVALID = 0xff,
	MOVE_LEFT = 0x00,
	MOVE_RIGHT = 0x01,
	MOVE_UP = 0x03,
	MOVE_DOWN = 0x04
} objectDirection;

typedef enum _rotationAxis {
	ROTATE_INVALID = 0xff,
	ROTATE_X = 0x00,
	ROTATE_Y = 0x01,
	ROTATE_Z = 0x02,
} rotationAxis;

typedef struct _objectAttributes {
	float diffuseStrength;
	float specularStrength;
	rotationAxis axis;
	objectDirection translate;
	int lightPositionIndex;
} objectAttributes;

class render {
	private:
		uint32_t SCR_WIDTH;
		uint32_t SCR_HEIGHT;
		GLuint sphereProgramId;
		unsigned int sphereVertexBufferObject, sphereVertexArrayObject;
		vector<point> points;
		vector<GLfloat> vertex;
		glm::mat4 projectionMatrix1;
		glm::mat4 viewMatrix1;
		float moveObject;
		rotationAxis prev_axis;
		unsigned int diffuseMap, diffuseMapSphere;
		GLuint LoadShaders(const char *vert_file, const char *frag_file);
	public:
		render();
		~render();
		void initRender(int width, int height, ply_parser *parser);
		void setView1();
		void drawSphere(int sphereIndex, objectAttributes attr);
		void drawSpheres(rotationAxis axis, objectDirection translate);
		unsigned int loadTexture(char const * path);
};
#endif
