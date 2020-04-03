#ifndef _PLY_PARSER_
#define _PLY_PARSER_

#include <vector>
#include <numeric>
#include <stdint.h>
#include <stdbool.h>
#include <GL/gl.h>
#include <GL/glu.h>

using namespace std;

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

class ply_parser {
	private:
		vector<point> vertices;
		vector<point> points;
		vector<point> normal;
		GLfloat *element_face_points;
		uint32_t total_vertices;
		uint32_t total_faces;
		void cross_product(point *a, point *b, point *result);
		void calculateNormal(point *coord1, point *coord2, point *coord3, point *result);

	public:
		ply_parser();
		~ply_parser();
		void load_ply(const char *file_path);
		uint32_t get_element_face_size();
		vector<point> get_element_face_points();
		vector<point> get_normal_vector();
};
#endif
