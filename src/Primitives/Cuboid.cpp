
#include "Primitive.cpp"

class Cuboid : public Primitive {

public:
	glm::vec3 center, size;

    Cuboid(Shader shader, Shader normalsShader, 
		glm::vec3 translation, glm::vec3 scale, glm::vec3 rotation, 
        glm::vec3 center, glm::vec3 size, 
		glm::vec3 color, bool useSolidColor, 
		const unsigned int* diffuseMap, const unsigned int* specularMap)

        : Primitive(shader, normalsShader, translation, scale, rotation, color, useSolidColor, diffuseMap, specularMap), center(center), size(size) {

        if (sizeIsWrong()) {
			std::cout << "ERROR: Both dimension of the size of CUBOID should be positive." << std::endl;
			return;
		}

        vertices = getVertices();
        calculateBoundingBox(vertices, 8);
        TransferDataToGPU(vertices, bb.vertices);
    }

private:
    bool sizeIsWrong() {
        if (size.x <= 0 || size.y <= 0 || size.z <= 0) {
			return true;
		}

		return false;
    }

    std::vector<float> getVertices() {
        glm::vec3 vertices[] = {
            glm::vec3(center.x - size.x / 2, center.y - size.y / 2, center.z + size.z / 2), // 0: front-bottom-left
            glm::vec3(center.x + size.x / 2, center.y - size.y / 2, center.z + size.z / 2), // 1: front-bottom-right
            glm::vec3(center.x + size.x / 2, center.y + size.y / 2, center.z + size.z / 2), // 2: front-top-right
            glm::vec3(center.x - size.x / 2, center.y + size.y / 2, center.z + size.z / 2), // 3: front-top-left
            glm::vec3(center.x - size.x / 2, center.y - size.y / 2, center.z - size.z / 2), // 4: back-bottom-left
            glm::vec3(center.x + size.x / 2, center.y - size.y / 2, center.z - size.z / 2), // 5: back-bottom-right
            glm::vec3(center.x + size.x / 2, center.y + size.y / 2, center.z - size.z / 2), // 6: back-top-right
            glm::vec3(center.x - size.x / 2, center.y + size.y / 2, center.z - size.z / 2)  // 7: back-top-left
        };

        auto calculateNormal = [](glm::vec3 a, glm::vec3 b, glm::vec3 c) {
            glm::vec3 edge1 = b - a;
            glm::vec3 edge2 = c - a;
            return glm::normalize(glm::cross(edge1, edge2));
        };

        glm::vec3 frontNormal = calculateNormal(vertices[0], vertices[1], vertices[2]);
        glm::vec3 backNormal = calculateNormal(vertices[5], vertices[4], vertices[7]);
        glm::vec3 topNormal = calculateNormal(vertices[3], vertices[2], vertices[6]);
        glm::vec3 bottomNormal = calculateNormal(vertices[0], vertices[4], vertices[5]);
        glm::vec3 leftNormal = calculateNormal(vertices[4], vertices[0], vertices[3]);
        glm::vec3 rightNormal = calculateNormal(vertices[1], vertices[5], vertices[6]);

        std::vector<float> v = {
            //Vertex Positions                        -                Normals                   -  Texture coords
            // Front face
            vertices[0].x, vertices[0].y, vertices[0].z, frontNormal.x, frontNormal.y, frontNormal.z, 0.0, 0.0,
            vertices[1].x, vertices[1].y, vertices[1].z, frontNormal.x, frontNormal.y, frontNormal.z, 1.0, 0.0,
            vertices[2].x, vertices[2].y, vertices[2].z, frontNormal.x, frontNormal.y, frontNormal.z, 1.0, 1.0,

            vertices[2].x, vertices[2].y, vertices[2].z, frontNormal.x, frontNormal.y, frontNormal.z, 1.0, 1.0,
            vertices[3].x, vertices[3].y, vertices[3].z, frontNormal.x, frontNormal.y, frontNormal.z, 0.0, 1.0,
            vertices[0].x, vertices[0].y, vertices[0].z, frontNormal.x, frontNormal.y, frontNormal.z, 0.0, 0.0,

            // Back face
            vertices[5].x, vertices[5].y, vertices[5].z, backNormal.x, backNormal.y, backNormal.z, 1.0, 0.0,
            vertices[4].x, vertices[4].y, vertices[4].z, backNormal.x, backNormal.y, backNormal.z, 0.0, 0.0,
            vertices[7].x, vertices[7].y, vertices[7].z, backNormal.x, backNormal.y, backNormal.z, 0.0, 1.0,

            vertices[7].x, vertices[7].y, vertices[7].z, backNormal.x, backNormal.y, backNormal.z, 0.0, 1.0,
            vertices[6].x, vertices[6].y, vertices[6].z, backNormal.x, backNormal.y, backNormal.z, 1.0, 1.0,
            vertices[5].x, vertices[5].y, vertices[5].z, backNormal.x, backNormal.y, backNormal.z, 1.0, 0.0,

            // Top face
            vertices[3].x, vertices[3].y, vertices[3].z, topNormal.x, topNormal.y, topNormal.z, 0.0, 1.0,
            vertices[2].x, vertices[2].y, vertices[2].z, topNormal.x, topNormal.y, topNormal.z, 1.0, 1.0,
            vertices[6].x, vertices[6].y, vertices[6].z, topNormal.x, topNormal.y, topNormal.z, 1.0, 0.0,

            vertices[6].x, vertices[6].y, vertices[6].z, topNormal.x, topNormal.y, topNormal.z, 1.0, 0.0,
            vertices[7].x, vertices[7].y, vertices[7].z, topNormal.x, topNormal.y, topNormal.z, 0.0, 0.0,
            vertices[3].x, vertices[3].y, vertices[3].z, topNormal.x, topNormal.y, topNormal.z, 0.0, 1.0,

            // Bottom face
            vertices[4].x, vertices[4].y, vertices[4].z, bottomNormal.x, bottomNormal.y, bottomNormal.z, 0.0, 0.0,
            vertices[5].x, vertices[5].y, vertices[5].z, bottomNormal.x, bottomNormal.y, bottomNormal.z, 1.0, 0.0,
            vertices[1].x, vertices[1].y, vertices[1].z, bottomNormal.x, bottomNormal.y, bottomNormal.z, 1.0, 1.0,

            vertices[1].x, vertices[1].y, vertices[1].z, bottomNormal.x, bottomNormal.y, bottomNormal.z, 1.0, 1.0,
            vertices[0].x, vertices[0].y, vertices[0].z, bottomNormal.x, bottomNormal.y, bottomNormal.z, 0.0, 1.0,
            vertices[4].x, vertices[4].y, vertices[4].z, bottomNormal.x, bottomNormal.y, bottomNormal.z, 0.0, 0.0,

            // Left face
            vertices[4].x, vertices[4].y, vertices[4].z, leftNormal.x, leftNormal.y, leftNormal.z, 1.0, 0.0,
            vertices[0].x, vertices[0].y, vertices[0].z, leftNormal.x, leftNormal.y, leftNormal.z, 0.0, 0.0,
            vertices[3].x, vertices[3].y, vertices[3].z, leftNormal.x, leftNormal.y, leftNormal.z, 0.0, 1.0,

            vertices[3].x, vertices[3].y, vertices[3].z, leftNormal.x, leftNormal.y, leftNormal.z, 0.0, 1.0,
            vertices[7].x, vertices[7].y, vertices[7].z, leftNormal.x, leftNormal.y, leftNormal.z, 1.0, 1.0,
            vertices[4].x, vertices[4].y, vertices[4].z, leftNormal.x, leftNormal.y, leftNormal.z, 1.0, 0.0,

            // Right face
            vertices[1].x, vertices[1].y, vertices[1].z, rightNormal.x, rightNormal.y, rightNormal.z, 0.0, 0.0,
            vertices[5].x, vertices[5].y, vertices[5].z, rightNormal.x, rightNormal.y, rightNormal.z, 1.0, 0.0,
            vertices[6].x, vertices[6].y, vertices[6].z, rightNormal.x, rightNormal.y, rightNormal.z, 1.0, 1.0,

            vertices[6].x, vertices[6].y, vertices[6].z, rightNormal.x, rightNormal.y, rightNormal.z, 1.0, 1.0,
            vertices[2].x, vertices[2].y, vertices[2].z, rightNormal.x, rightNormal.y, rightNormal.z, 0.0, 1.0,
            vertices[1].x, vertices[1].y, vertices[1].z, rightNormal.x, rightNormal.y, rightNormal.z, 0.0, 0.0
        };

        return v;
    }

};