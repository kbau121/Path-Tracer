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
				face next_face = face();

				for (int i = 0; i < 3; ++i) {
					pop_next(line, SPACE, token);

					try {
						next_face.v[i] = stoi(pop_next(token, FSLASH)) - 1;

						try {
							next_face.t[i] = stoi(pop_next(token, FSLASH)) - 1;
						}
						catch (exception) {}
						
						next_face.n[i] = stoi(token) - 1;
					}
					catch (exception) { }
				}

				faces.push_back(next_face);
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

RTCGeometry read_obj(const char* file_location, RTCDevice& device, glm::vec3 p_transform, glm::vec3 s_transform) {
	parse_rec* rec = parse_obj(file_location);

	RTCGeometry geometry = rtcNewGeometry(device, RTC_GEOMETRY_TYPE_TRIANGLE);
	glm::vec3* verts = (glm::vec3*)rtcSetNewGeometryBuffer(geometry, RTC_BUFFER_TYPE_VERTEX, 0, RTC_FORMAT_FLOAT3, sizeof(glm::vec3), rec->faces.size()*3);
	glm::uvec3* inds = (glm::uvec3*)rtcSetNewGeometryBuffer(geometry, RTC_BUFFER_TYPE_INDEX, 0, RTC_FORMAT_UINT3, sizeof(glm::uvec3), rec->faces.size());

	for (int i = 0; i < rec->faces.size(); i++) {
		verts[i*3] = rec->vertices[rec->faces[i].v[0]] * s_transform + p_transform;
		verts[i*3 + 1] = rec->vertices[rec->faces[i].v[1]] * s_transform + p_transform;
		verts[i*3 + 2] = rec->vertices[rec->faces[i].v[2]] * s_transform + p_transform;
	}

	int norm_size = rec->faces.size()*3;
	glm::vec3* norms = new glm::vec3[norm_size];
	for (int i = 0; i < rec->faces.size(); i++) {
		/*
		norms[i * 3] = rec->vertices[rec->faces[i].v[0]];
		norms[i * 3 + 1] = rec->vertices[rec->faces[i].v[1]];
		norms[i * 3 + 2] = rec->vertices[rec->faces[i].v[2]];
		*/

		norms[i * 3] = rec->normals[rec->faces[i].n[0]];
		norms[i * 3 + 1] = rec->normals[rec->faces[i].n[1]];
		norms[i * 3 + 2] = rec->normals[rec->faces[i].n[2]];
	}
	rtcSetGeometryVertexAttributeCount(geometry, 1);
	rtcSetSharedGeometryBuffer(geometry, RTC_BUFFER_TYPE_VERTEX_ATTRIBUTE, 0, RTC_FORMAT_FLOAT3, norms, 0, sizeof(glm::vec3), rec->normals.size());

	for (int i = 0; i < rec->faces.size(); i++) {
		inds[i] = glm::uvec3(i*3, i*3 + 1, i*3 + 2);
	}

	delete rec;

	rtcCommitGeometry(geometry);
	return geometry;
}

RTCGeometry read_obj(const char* file_location, RTCDevice& device) {
	return read_obj(file_location, device, glm::vec3(0), glm::vec3(1));
}