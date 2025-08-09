
#include "Primitive.cpp"

class Quad : public Primitive {

public:
	glm::vec2 center, size;

	Quad(Shader shader, Shader normalsShader, 
		glm::vec3 translation, glm::vec3 scale, glm::vec3 rotation,
		glm::vec2 center, glm::vec2 size, 
		glm::vec3 color, bool useSolidColor, 
		const unsigned int* diffuseMap = nullptr, const unsigned int* specularMap = nullptr)

		: Primitive(shader, normalsShader, translation, scale, rotation, color, useSolidColor, diffuseMap, specularMap), center(center), size(size) {

		if (sizeIsWrong()) {
			std::cout << "ERROR: Both dimension of the size of QUAD should be positive." << std::endl;
			return;
		}

		vertices = getVertices();
		calculateBoundingBox(vertices, 8);
		TransferDataToGPU(vertices, bb.vertices);	
	}

private:
	bool sizeIsWrong() {
		if (size.x <= 0 || size.y <= 0) {
			return true;
		}

		return false;
	}

	std::vector<float> getVertices() {
		glm::vec3 v1(center.x - size.x / 2, center.y + size.y / 2, 0.0);
		glm::vec3 v2(center.x - size.x / 2, center.y - size.y / 2, 0.0);
		glm::vec3 v3(center.x + size.x / 2, center.y + size.y / 2, 0.0);
		glm::vec3 v4(center.x + size.x / 2, center.y - size.y / 2, 0.0);

		std::vector<float> v = {
			//Vertex Positions -      Normals     -   Texture coords
			  v1.x, v1.y, 0.0,      0.0, 0.0, 1.0,      0.0, 1.0,
			  v2.x, v2.y, 0.0,      0.0, 0.0, 1.0,      0.0, 0.0,
			  v3.x, v3.y, 0.0,      0.0, 0.0, 1.0,      1.0, 1.0,
          
			  v4.x, v4.y, 0.0,      0.0, 0.0, 1.0,      1.0, 0.0,
			  v3.x, v3.y, 0.0,      0.0, 0.0, 1.0,      1.0, 1.0,
			  v2.x, v2.y, 0.0,      0.0, 0.0, 1.0,      0.0, 0.0
		};

		return v;
	}

};