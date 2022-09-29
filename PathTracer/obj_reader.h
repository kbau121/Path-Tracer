#pragma once

#include "hittable_list.h"
#include "material.h"
#include "triangle.h"

#include <fstream>
#include <string>
#include <vector>

using namespace std;

struct face {
	int v[3];
	int t[3];
	int n[3];
};

inline string pop_next(string& str, const string delimiter, string& out) {
	size_t next_token = str.find(delimiter);
	out = str.substr(0, next_token);
	str.erase(0, next_token + delimiter.size());

	return out;
}

inline string pop_next(string& str, const string delimiter) {
	size_t next_token = str.find(delimiter);
	string out = str.substr(0, next_token);
	str.erase(0, next_token + delimiter.size());

	return out;
}

void read_obj(const char* file_location, hittable_list& objects) {
	const string VERT = "v";
	const string NORM = "vn";
	//const string TEXTC = "vt";
	const string SMOOTH = "s";
	const string FACE = "f";

	const string SPACE = " ";
	const string FSLASH = "/";

	vector<point3> vertices;
	vector<vec3> normals;
	//std::vector<vec3> textureCoords;
	vector<face> faces;
	bool isSmooth = false;

	ifstream obj_file(file_location, ios::in);

	if (obj_file) {
		string line;
		while (obj_file) {
			getline(obj_file, line);
			
			string token;
			pop_next(line, SPACE, token);

			if (token.compare(VERT) == 0) {
				double e[3];

				for (int i = 0; i < 3; ++i) {
					e[i] = stod(pop_next(line, SPACE, token));
				}

				vertices.push_back(point3(e[0], e[1], e[2]));
			}
			else if (token.compare(NORM) == 0) {
				double e[3];

				for (int i = 0; i < 3; ++i) {
					e[i] = stod(pop_next(line, SPACE, token));
				}

				normals.push_back(vec3(e[0], e[1], e[2]));
			}
			else if (token.compare(SMOOTH) == 0) {
				isSmooth = (stoi(pop_next(line, SPACE)) == 1);
			}
			else if (token.compare(FACE) == 0) {
				faces.push_back(face());

				for (int i = 0; i < 3; ++i) {
					pop_next(line, SPACE, token);

					faces[faces.size() - 1].v[i] = stoi(pop_next(token, FSLASH)) - 1;
					faces[faces.size() - 1].t[i] = stoi(pop_next(token, FSLASH)) - 1;
					faces[faces.size() - 1].n[i] = stoi(token) - 1;
				}
			}
		}
		
		obj_file.close();

		auto test_mat = make_shared<normal>();

		for (int i = 0; i < faces.size(); ++i) {
			if (isSmooth) {
				objects.add(make_shared<triangle>(
					vertices[faces[i].v[0]],
					vertices[faces[i].v[1]],
					vertices[faces[i].v[2]],
					normals[faces[i].n[0]],
					normals[faces[i].n[1]],
					normals[faces[i].n[2]],
					test_mat
					));
			}
			else {
				objects.add(make_shared<triangle>(
					vertices[faces[i].v[0]],
					vertices[faces[i].v[1]],
					vertices[faces[i].v[2]],
					test_mat
					));
			}
		}
	}
	else {
		printf("Unable to open file: %s", file_location);
	}
}