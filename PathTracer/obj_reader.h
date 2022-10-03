#pragma once

#include "glm/glm.hpp"
#include "hittable_list.h"
#include "material.h"
#include "triangle.h"

#include <embree3/rtcore.h>
#include <fstream>
#include <string>
#include <vector>

using namespace std;

struct face {
	int v[3];
	int t[3];
	int n[3];
};

struct parse_rec {
	vector<vec3> vertices;
	vector<vec3> normals;
	vector<face> faces;
	bool isSmooth;
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

parse_rec* parse_obj(const char* file_location) {
	const string VERT = "v";
	const string NORM = "vn";
	//const string TEXTC = "vt";
	const string SMOOTH = "s";
	const string FACE = "f";

	const string SPACE = " ";
	const string FSLASH = "/";

	vector<vec3> vertices;
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
				float e[3];

				for (int i = 0; i < 3; ++i) {
					e[i] = stof(pop_next(line, SPACE, token));
				}

				vertices.push_back(vec3(e[0], e[1], e[2]));
			}
			else if (token.compare(NORM) == 0) {
				float e[3];

				for (int i = 0; i < 3; ++i) {
					e[i] = stof(pop_next(line, SPACE, token));
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
	}
	else {
		printf("Unable to open file: %s", file_location);
	}

	return new parse_rec { vertices, normals, faces, isSmooth };
}

void read_obj(const char* file_location, hittable_list& objects) {
	parse_rec* rec = parse_obj(file_location);

	auto test_mat = make_shared<normal>();

	for (int i = 0; i < rec->faces.size(); ++i) {
		if (rec->isSmooth) {
			objects.add(make_shared<triangle>(
				rec->vertices[rec->faces[i].v[0]],
				rec->vertices[rec->faces[i].v[1]],
				rec->vertices[rec->faces[i].v[2]],
				rec->normals[rec->faces[i].n[0]],
				rec->normals[rec->faces[i].n[1]],
				rec->normals[rec->faces[i].n[2]],
				test_mat
				));
		}
		else {
			objects.add(make_shared<triangle>(
				rec->vertices[rec->faces[i].v[0]],
				rec->vertices[rec->faces[i].v[1]],
				rec->vertices[rec->faces[i].v[2]],
				test_mat
				));
		}
	}

	delete rec;
}

RTCGeometry read_obj(const char* file_location, RTCDevice &device) {
	parse_rec* rec = parse_obj(file_location);

	RTCGeometry geometry = rtcNewGeometry(device, RTC_GEOMETRY_TYPE_TRIANGLE);
	glm::vec3* verts = (glm::vec3*)rtcSetNewGeometryBuffer(geometry, RTC_BUFFER_TYPE_VERTEX, 0, RTC_FORMAT_FLOAT3, sizeof(glm::vec3), rec->vertices.size());
	glm::uvec3* inds = (glm::uvec3*)rtcSetNewGeometryBuffer(geometry, RTC_BUFFER_TYPE_INDEX, 0, RTC_FORMAT_UINT3, sizeof(glm::uvec3), rec->faces.size());

	for (int i = 0; i < rec->vertices.size(); ++i) {
		verts[i] = rec->vertices[i];
	}

	for (int i = 0; i < rec->faces.size(); ++i) {
		inds[i] = glm::uvec3(rec->faces[i].v[0], rec->faces[i].v[1], rec->faces[i].v[2]);
	}

	delete rec;

	rtcCommitGeometry(geometry);
	return geometry;
}