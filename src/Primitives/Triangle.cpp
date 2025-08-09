
#include "Primitive.cpp"


class Triangle : public Primitive {

public:
	glm::vec2 v1, v2, v3;

	Triangle(Shader shader, Shader normalsShader, 
		glm::vec3 translation, glm::vec3 scale, glm::vec3 rotation,
		glm::vec2 v1, glm::vec2 v2, glm::vec2 v3, 
		glm::vec3 color, bool useSolidColor, 
		const unsigned int* diffuseMap = nullptr, const unsigned int* specularMap = nullptr)

		: Primitive(shader, normalsShader, translation, scale, rotation, color, useSolidColor, diffuseMap, specularMap), v1(v1), v2(v2), v3(v3) {
		
		if (collinear()) {
			std::cout << "ERROR: The 3 vertices are collinear, no such TRIANGLE exists" << std::endl;
			return;
		}

		vertices = getVertices();
		calculateBoundingBox(vertices, 8);
        TransferDataToGPU(vertices, bb.vertices);
	}

private:
	bool collinear() {
		float area = v1.x * (v2.y - v3.y) + v2.x * (v3.y - v1.y) + v3.x * (v1.y - v2.y);
		if (area == 0) {
			return true;
		}
		return false;
	}

	std::vector<float> getVertices() {
		std::vector<float> vertices = {
		  //Vertex Positions   -    Normals       -  Texture coords
			v1.x, v1.y, 0.0,     0.0, 0.0, 1.0,       0.0, 0.0,
			v2.x, v2.y, 0.0,     0.0, 0.0, 1.0,       1.0, 0.0,
			v3.x, v3.y, 0.0,     0.0, 0.0, 1.0,       0.5, 1.0
		};

		return vertices;
	}
};