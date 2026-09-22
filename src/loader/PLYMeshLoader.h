#pragma once

#include <cstdint>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

#include <glm/glm.hpp>

using namespace std;
using glm::vec2;
using glm::vec3;
using glm::vec4;

struct PLYMeshPart {
	vector<vec3> positions;
	vector<vec2> uvs;

	vec4 color = vec4(1.0f);
	uint32_t colorKey = 0xffffffff;
};

struct PLYMesh {
	vector<PLYMeshPart> parts;
	size_t triangleCount = 0;
};

struct PLYMeshLoader {

	static uint32_t packColor(
		uint32_t r,
		uint32_t g,
		uint32_t b,
		uint32_t a = 255)
	{
		return
			(r & 0xffu) |
			((g & 0xffu) << 8) |
			((b & 0xffu) << 16) |
			((a & 0xffu) << 24);
	}

	static vec4 unpackColor(uint32_t color)
	{
		float r = float((color >> 0) & 0xffu) / 255.0f;
		float g = float((color >> 8) & 0xffu) / 255.0f;
		float b = float((color >> 16) & 0xffu) / 255.0f;
		float a = float((color >> 24) & 0xffu) / 255.0f;

		return vec4(r, g, b, a);
	}

	static PLYMesh load(const string& path)
	{
		ifstream file(path);

		if (!file.is_open()) {
			throw runtime_error(
				"Could not open PLY file: " + path
			);
		}

		string line;

		size_t numVertices = 0;
		size_t numFaces = 0;

		bool ascii = false;
		bool hasAlpha = false;
		bool headerEnded = false;

		// ============================================================
		// HEADER
		// ============================================================

		while (getline(file, line)) {

			if (line == "format ascii 1.0") {
				ascii = true;
			}
			else if (line.rfind("element vertex ", 0) == 0) {

				stringstream ss(line);

				string elementString;
				string vertexString;

				ss >> elementString
					>> vertexString
					>> numVertices;
			}
			else if (line.rfind("element face ", 0) == 0) {

				stringstream ss(line);

				string elementString;
				string faceString;

				ss >> elementString
					>> faceString
					>> numFaces;
			}
			else if (line == "property uchar alpha") {
				hasAlpha = true;
			}
			else if (line == "end_header") {
				headerEnded = true;
				break;
			}
		}

		if (!headerEnded) {
			throw runtime_error(
				"Invalid PLY: missing end_header"
			);
		}

		if (!ascii) {
			throw runtime_error(
				"PLYMeshLoader supports ASCII PLY only"
			);
		}

		if (numVertices == 0) {
			throw runtime_error(
				"PLY contains no vertices"
			);
		}

		// ============================================================
		// VERTICES
		// ============================================================

		vector<vec3> sourcePositions;
		vector<uint32_t> sourceColors;

		sourcePositions.reserve(numVertices);
		sourceColors.reserve(numVertices);

		for (size_t i = 0; i < numVertices; i++) {

			if (!getline(file, line)) {
				throw runtime_error(
					"Unexpected end of PLY vertex data"
				);
			}

			stringstream ss(line);

			float x = 0.0f;
			float y = 0.0f;
			float z = 0.0f;

			uint32_t r = 255;
			uint32_t g = 255;
			uint32_t b = 255;
			uint32_t a = 255;

			ss >> x >> y >> z >> r >> g >> b;

			if (hasAlpha) {
				ss >> a;
			}

			if (ss.fail()) {
				throw runtime_error(
					"Invalid vertex data at vertex " +
					to_string(i)
				);
			}

			sourcePositions.push_back(
				vec3(x, y, z)
			);

			sourceColors.push_back(
				packColor(r, g, b, a)
			);
		}

		// ============================================================
		// FACES
		// ============================================================

		PLYMesh result;

		unordered_map<uint32_t, size_t> colorToPart;

		auto addTriangle = [&](PLYMeshPart& part, const vec3& p0, const vec3& p1, const vec3& p2) {

			part.positions.push_back(p0);
			part.positions.push_back(p1);
			part.positions.push_back(p2);

			part.uvs.push_back(vec2(0.5f, 0.5f));
			part.uvs.push_back(vec2(0.5f, 0.5f));
			part.uvs.push_back(vec2(0.5f, 0.5f));
			};

		auto addSubdividedTriangle = [&](PLYMeshPart& part, const vec3& p0, const vec3& p1, const vec3& p2) {

			constexpr int subdivisionLevel = 4;

			auto subdivide = [&](auto&& self, const vec3& a, const vec3& b, const vec3& c, int level) -> void {

				if (level == 0) {
					addTriangle(part, a, b, c);
					return;
				}

				const vec3 center = (a + b + c) / 3.0f;

				self(self, a, b, center, level - 1);
				self(self, b, c, center, level - 1);
				self(self, c, a, center, level - 1);
				};

			subdivide(subdivide, p0, p1, p2, subdivisionLevel);
			};

		auto getPart = [&](uint32_t colorKey)
			-> PLYMeshPart&
			{
				auto it = colorToPart.find(colorKey);

				if (it != colorToPart.end()) {
					return result.parts[it->second];
				}

				size_t index = result.parts.size();

				PLYMeshPart part;
				part.colorKey = colorKey;
				part.color = unpackColor(colorKey);

				result.parts.push_back(part);

				colorToPart[colorKey] = index;

				return result.parts[index];
			};

		for (size_t faceIndex = 0; faceIndex < numFaces; faceIndex++) {

			if (!getline(file, line)) {
				throw runtime_error(
					"Unexpected end of PLY face data"
				);
			}

			stringstream ss(line);

			uint32_t vertexCount = 0;

			ss >> vertexCount;

			if (vertexCount < 3) {
				continue;
			}

			vector<uint32_t> indices(vertexCount);

			for (uint32_t i = 0; i < vertexCount; i++) {

				ss >> indices[i];

				if (indices[i] >= sourcePositions.size()) {
					throw runtime_error(
						"PLY face contains invalid vertex index"
					);
				}
			}

			// Fan triangulation:
			//
			// 0 1 2
			// 0 2 3
			// 0 3 4
			// ...

			for (uint32_t i = 1; i + 1 < vertexCount; i++) {
				uint32_t i0 = indices[0];
				uint32_t i1 = indices[i];
				uint32_t i2 = indices[i + 1];

				// Our avatar uses one material/color per triangle.
				uint32_t colorKey = sourceColors[i0];

				PLYMeshPart& part = getPart(colorKey);
				const vec3& p0 = sourcePositions[i0];
				const vec3& p1 = sourcePositions[i1];
				const vec3& p2 = sourcePositions[i2];

				addSubdividedTriangle(part, p0, p1, p2);
				//part.positions.push_back(sourcePositions[i0]);
				//part.positions.push_back(sourcePositions[i1]);
				//part.positions.push_back(sourcePositions[i2]);

				//// No texture is used.
				//part.uvs.push_back(vec2(0.5f, 0.5f));
				//part.uvs.push_back(vec2(0.5f, 0.5f));
				//part.uvs.push_back(vec2(0.5f, 0.5f));

				result.triangleCount++;
			}
		}

		if (result.triangleCount == 0) {
			throw runtime_error(
				"PLY contains no valid triangles"
			);
		}

		return result;
	}
};