
#include "Primitive.cpp"


class Circle : public Primitive {

public:
	glm::vec2 center;
	float r;
	int steps;

	Circle(Shader shader, Shader normalsShader, 
		glm::vec3 translation, glm::vec3 scale, glm::vec3 rotation,
        glm::vec2 center, float r, int steps, 
		glm::vec3 color, bool useSolidColor, 
		const unsigned int* diffuseMap = nullptr, const unsigned int* specularMap = nullptr)

		: Primitive(shader, normalsShader, translation, scale, rotation, color, useSolidColor, diffuseMap, specularMap), center(center), r(r), steps(steps) {

		if (r < 0 || steps < 4) {
			std::cout << "ERROR: Range should be positive and steps larger than 4 in order to create CIRCLE" << std::endl;
			return;
		}

		vertices = getVertices();
        calculateBoundingBox(vertices, 8);
        TransferDataToGPU(vertices, bb.vertices);
	}

private:
    std::vector<float> getVertices() {
        std::vector<float> vertices;

        float angleStep = 2.0f * M_PI / steps;

        for (int i = 0; i < steps; ++i) {
            float theta1 = i * angleStep;
            float theta2 = (i + 1) * angleStep;

            vertices.push_back(center.x);
            vertices.push_back(center.y);
            vertices.push_back(0);
            // normals
            vertices.push_back(0.0f);
            vertices.push_back(0.0);
            vertices.push_back(1.0f);
            // texture coordinates
            vertices.push_back(0.5f);
            vertices.push_back(0.5f);

            vertices.push_back(center.x + r * cos(theta1));
            vertices.push_back(center.y + r * sin(theta1));
            vertices.push_back(0);
            // normals
            vertices.push_back(0.0f);
            vertices.push_back(0.0);
            vertices.push_back(1.0f);
            // texture coordinates
            vertices.push_back(0.5f + 0.5f * cos(theta1));
            vertices.push_back(0.5f + 0.5f * sin(theta1));

            vertices.push_back(center.x + r * cos(theta2));
            vertices.push_back(center.y + r * sin(theta2));
            vertices.push_back(0);
            // normals
            vertices.push_back(0.0f);
            vertices.push_back(0.0);
            vertices.push_back(1.0f);
            // texture coordinates
            vertices.push_back(0.5f + 0.5f * cos(theta2));
            vertices.push_back(0.5f + 0.5f * sin(theta2));
        }

        return vertices;
    }


};