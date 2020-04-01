#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <learnopengl/filesystem.h>
#include <learnopengl/shader_m.h>
#include <learnopengl/camera.h>

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <numeric>

using namespace std;

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow *window);

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// camera
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// lighting
glm::vec3 lightPos(1.2f, 1.0f, 2.0f);

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
		cout << "Normal " << normalv.x << " " << normalv.y << " " << normalv.z << endl;
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
		cout << "indices " << v1 << " " << v2 << " " << v3 <<" " << v4 << endl;
		if (line[0] == '4') {
			vertices[v4].set_vertex_normal(normalv.x, normalv.y, normalv.z);
			points.push_back(vertices[v4]);
			indices.push_back(v4);
		}
		cout << points[0].x << points[1].y << points[2].z << endl;
	}
}

int main()
{
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // uncomment this statement to fix compilation on OS X
#endif

    // glfw window creation
    // --------------------
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    // tell GLFW to capture our mouse
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // configure global opengl state
    // -----------------------------
    glEnable(GL_DEPTH_TEST);

    // build and compile our shader zprogram
    // ------------------------------------
    Shader lightingShader("3.2.materials.vs", "3.2.materials.fs");
    Shader lampShader("3.2.lamp.vs", "3.2.lamp.fs");

	ply_reader ply;
	ply.ply_load("/home/anurag/Downloads/opengl/sphere.ply");

    // set up vertex data (and buffer(s)) and configure vertex attributes
    // ------------------------------------------------------------------
#if 0
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
	vector<point> points = ply.get_element_face_points();

	//vector<GLfloat> vertex = ply.get_vertex();
	vector<float> vertex;
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
    // first, configure the cube's VAO (and VBO)
    unsigned int VBO, cubeVAO;
    glGenVertexArrays(1, &cubeVAO);
    glGenBuffers(1, &VBO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    //glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	cout << "VERT SIZE " << vertex.size() << endl;
    glBufferData(GL_ARRAY_BUFFER, vertex.size() * sizeof(float), &vertex[0], GL_STATIC_DRAW);

    glBindVertexArray(cubeVAO);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // normal attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);


    // second, configure the light's VAO (VBO stays the same; the vertices are the same for the light object which is also a 3D cube)
    unsigned int lightVAO;
    glGenVertexArrays(1, &lightVAO);
    glBindVertexArray(lightVAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    // note that we update the lamp's position attribute's stride to reflect the updated buffer data
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);


    // render loop
    // -----------
    while (!glfwWindowShouldClose(window))
    {
        // per-frame time logic
        // --------------------
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // input
        // -----
        processInput(window);

        // render
        // ------
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // be sure to activate shader when setting uniforms/drawing objects
        lightingShader.use();
        lightingShader.setVec3("light.position", lightPos);
        lightingShader.setVec3("viewPos", camera.Position);

        // light properties
        lightingShader.setVec3("light.ambient", 1.0f, 1.0f, 1.0f); // note that all light colors are set at full intensity
        lightingShader.setVec3("light.diffuse", 1.0f, 1.0f, 1.0f);
        lightingShader.setVec3("light.specular", 1.0f, 1.0f, 1.0f);

        // material properties
        lightingShader.setVec3("material.ambient", 0.0f, 0.1f, 0.06f);
        lightingShader.setVec3("material.diffuse", 0.0f, 0.50980392f, 0.50980392f);
        lightingShader.setVec3("material.specular", 0.50196078f, 0.50196078f, 0.50196078f);
        lightingShader.setFloat("material.shininess", 32.0f);

        // view/projection transformations
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = camera.GetViewMatrix();
        lightingShader.setMat4("projection", projection);
        lightingShader.setMat4("view", view);

        // world transformation
        glm::mat4 model = glm::mat4(1.0f);
        lightingShader.setMat4("model", model);

        // render the cube
        glBindVertexArray(cubeVAO);
        glDrawArrays(GL_TRIANGLES, 0, vertex.size());


        // also draw the lamp object
        lampShader.use();
        lampShader.setMat4("projection", projection);
        lampShader.setMat4("view", view);
        model = glm::mat4(1.0f);
        model = glm::translate(model, lightPos);
        model = glm::scale(model, glm::vec3(0.05f)); // a smaller cube
        lampShader.setMat4("model", model);

        glBindVertexArray(lightVAO);
        glDrawArrays(GL_TRIANGLES, 0, vertex.size()/3);


        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // optional: de-allocate all resources once they've outlived their purpose:
    // ------------------------------------------------------------------------
    glDeleteVertexArrays(1, &cubeVAO);
    glDeleteVertexArrays(1, &lightVAO);
    glDeleteBuffers(1, &VBO);

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}


// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(yoffset);
}
