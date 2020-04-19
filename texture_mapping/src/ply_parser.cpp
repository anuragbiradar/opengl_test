
#include <bits/stdc++.h>
#include <iostream>
#include <stdint.h>
#include <vector>
#include <fstream>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "ply_parser.h"
using namespace std;

ply_parser::ply_parser() {
}

ply_parser::~ply_parser() {
}

void ply_parser::cross_product(point *a, point *b, point *result)
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

void ply_parser::calculateNormal(point *coord1, point *coord2, point *coord3, point *result)
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


void ply_parser::load_ply(const char *file_path) {

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
	}
	for (i = 0; i < total_faces; i++) {
		getline(file, str);
		const char *line = str.c_str();
		float num, v1,v2, v3,v4;
		if (line[0] == '4')
			sscanf(line, "%f %f %f %f %f", &num, &v1, &v2, &v3, &v4);
		else
			sscanf(line, "%f %f %f %f", &num, &v1, &v2, &v3);
		point normalv;
		calculateNormal(&vertices[v1], &vertices[v2], &vertices[v3], &normalv);
		//cout << "Normal " << normalv.x << " " << normalv.y << " " << normalv.z << endl;
		this->normal.push_back(normalv);
		vertices[v1].set_vertex_normal(normalv.x, normalv.y, normalv.z);
		vertices[v2].set_vertex_normal(normalv.x, normalv.y, normalv.z);
		vertices[v3].set_vertex_normal(normalv.x, normalv.y, normalv.z);
		points.push_back(vertices[v1]);
		points.push_back(vertices[v2]);
		points.push_back(vertices[v3]);
		if (line[0] == '4') {
			vertices[v4].set_vertex_normal(normalv.x, normalv.y, normalv.z);
			points.push_back(vertices[v4]);
		}
	}
}

uint32_t ply_parser::get_element_face_size() {
	return this->total_faces;
}

vector<point> ply_parser::get_element_face_points() {
	return this->points;
}

vector<point> ply_parser::get_normal_vector() {
	return this->normal;
}
