#ifndef PRIMITIVE_H
#define PRIMITIVE_H

#include "../Shader.cpp"
#include "../BoundingBox.cpp"

#include "../../dependencies/stb_image.h"

#define _USE_MATH_DEFINES
#include <math.h>

#include <iostream>
#include <vector>

class Primitive {

public:
	unsigned int VBO, VAO, bbVBO, bbVAO;

    Shader shader;
	Shader normalsShader;

	std::vector<float> vertices;

	BoundingBox bb;

	glm::vec3 translation;
	glm::vec3 scale;
	glm::vec3 rotation;

	glm::vec3 color;
	bool useSolidColor;

    const unsigned int* diffuseMap;
    const unsigned int* specularMap;

    Primitive(Shader shader, Shader normalsShader, 
		glm::vec3 translation, glm::vec3 scale, glm::vec3 rotation, 
		glm::vec3 color, bool useSolidColor, 
		const unsigned int* diffuseMap = nullptr, const unsigned int* specularMap = nullptr)

	: shader(shader), normalsShader(normalsShader), 
	translation(translation), scale(scale), rotation(rotation), 
	color(color), useSolidColor(useSolidColor), 
	diffuseMap(diffuseMap), specularMap(specularMap) {

    }

	void draw() {
		glBindVertexArray(VAO);
		glDrawArrays(GL_TRIANGLES, 0, vertices.size() / 8);
		glBindVertexArray(0);
	}

	void drawBB() {
		static const unsigned int indices[] = {
			0,1, 1,2, 2,3, 3,0, // front face
			4,5, 5,6, 6,7, 7,4, // back face
			0,4, 1,5, 2,6, 3,7  // connecting edges
		};

		static unsigned int bbEBO = 0;
		if (bbEBO == 0) {
			glGenBuffers(1, &bbEBO);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bbEBO);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
		}

		glBindBuffer(GL_ARRAY_BUFFER, bbVBO);
		glBufferSubData(GL_ARRAY_BUFFER, 0, bb.vertices.size() * sizeof(float), bb.vertices.data());

		glBindVertexArray(bbVAO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bbEBO);
		glDrawElements(GL_LINES, 24, GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);
	}

	void calculateBoundingBox(std::vector<float> vertices, int vertexSize) {
		float minX = INFINITY, minY = INFINITY, minZ = INFINITY;
		float maxX = -INFINITY, maxY = -INFINITY, maxZ = -INFINITY;

		for (int i = 0; i < vertices.size(); i += 8) {
			if (vertices[i] > maxX) {
				maxX = vertices[i];
			}

			if (vertices[i] < minX) {
				minX = vertices[i];
			}

			if (vertices[i + 1] > maxY) {
				maxY = vertices[i + 1];
			}

			if (vertices[i + 1] < minY) {
				minY = vertices[i + 1];
			}

			if (vertices[i + 2] > maxZ) {
				maxZ = vertices[i + 2];
			}

			if (vertices[i + 2] < minZ) {
				minZ = vertices[i + 2];
			}
		}

		if (minX == maxX) {
			minX = minX - 0.01f;
			maxX = maxX + 0.01f;
		}

		if (minY == maxY) {
			minY = minY - 0.01f;
			maxY = maxY + 0.01f;
		}

		if (minZ == maxZ) {
			minZ = minZ - 0.01f;
			maxZ = maxZ + 0.01f;
		}

		glm::vec3 minVert = glm::vec3(minX, minY, minZ);
		glm::vec3 maxVert = glm::vec3(maxX, maxY, maxZ);

		std::vector<float> bbVertices = {
			minX, minY, maxZ, // 0: front-bottom-left
			maxX, minY, maxZ, // 1: front-bottom-right
			maxX, maxY, maxZ, // 2: front-top-right
			minX, maxY, maxZ, // 3: front-top-left
			minX, minY, minZ, // 4: back-bottom-left
			maxX, minY, minZ, // 5: back-bottom-right
			maxX, maxY, minZ, // 6: back-top-right
			minX, maxY, minZ  // 7: back-top-left
		};
		
		bb = BoundingBox(bbVertices, minVert, maxVert);
	}

protected:
	void TransferDataToGPU(std::vector<float> vertices, std::vector<float> bbVertices) {
		glGenVertexArrays(1, &VAO);
		glGenBuffers(1, &VBO);

		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

		glBindVertexArray(VAO);

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);

		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
		glEnableVertexAttribArray(1);

		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
		glEnableVertexAttribArray(2);

		glBindVertexArray(0);

		glGenVertexArrays(1, &bbVAO);
		glGenBuffers(1, &bbVBO);

		glBindBuffer(GL_ARRAY_BUFFER, bbVBO);
		glBufferData(GL_ARRAY_BUFFER, bbVertices.size() * sizeof(float), bbVertices.data(), GL_STATIC_DRAW);

		glBindVertexArray(bbVAO);

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);

		glBindVertexArray(0);
	}

};

#endif