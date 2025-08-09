
#include "Primitive.cpp"


class Sphere : public Primitive {

public:
    glm::vec3 center;
	float r;
	int steps;

    Sphere(Shader shader, Shader normalsShader, 
		glm::vec3 translation, glm::vec3 scale, glm::vec3 rotation,
        glm::vec3 center, float r, int steps, 
		glm::vec3 color, bool useSolidColor, 
		const unsigned int* diffuseMap = nullptr, const unsigned int* specularMap = nullptr)

        : Primitive(shader, normalsShader, translation, scale, rotation, color, useSolidColor, diffuseMap, specularMap), center(center), r(r), steps(steps) {

        if (r < 0 || steps < 4) {
            std::cout << "ERROR: Range should be positive and steps larger than 4 in order to create SPHERE" << std::endl;
            return;
        }

        vertices = getVertices();
        calculateBoundingBox(vertices, 8);
        TransferDataToGPU(vertices, bb.vertices);	
    }

private:
    std::vector<float> getVertices() {
        std::vector<float> vertices;

        float phiStep = 2.0f * M_PI / steps;
        float thetaStep = M_PI / steps;

        // Generate vertices and triangles for the sphere
        for (int i = 0; i < steps; ++i) {
            float theta1 = i * thetaStep;
            float theta2 = (i + 1) * thetaStep;

            for (int j = 0; j < steps; ++j) {
                float phi1 = j * phiStep;
                float phi2 = (j + 1) * phiStep;

                // Calculate the 4 vertices of the current patch
                glm::vec3 v1 = sphericalToCartesian(theta1, phi1);
                glm::vec3 v2 = sphericalToCartesian(theta1, phi2);
                glm::vec3 v3 = sphericalToCartesian(theta2, phi2);
                glm::vec3 v4 = sphericalToCartesian(theta2, phi1);

                if (i == 0) {
                    // North pole needs only one triangle
                    addVertex(vertices, v4, theta1, phi1);
                    addVertex(vertices, v3, theta2, phi2);
                    addVertex(vertices, v1, theta2, phi1);
                }
                else if (i == steps - 1) {
                    // South pole needs only one triangle
                    addVertex(vertices, v3, theta1, phi1);
                    addVertex(vertices, v2, theta1, phi2);
                    addVertex(vertices, v1, theta2, phi2);
                }
                else {
                    // Middle needs two triangles
                    addVertex(vertices, v3, theta1, phi1);
                    addVertex(vertices, v2, theta1, phi2);
                    addVertex(vertices, v1, theta2, phi2);

                    addVertex(vertices, v3, theta1, phi1);
                    addVertex(vertices, v1, theta2, phi2);
                    addVertex(vertices, v4, theta2, phi1);
                }
            }
        }

        return vertices;
    }

    glm::vec3 sphericalToCartesian(float theta, float phi) {
        float x = r * sin(theta) * cos(phi) + center.x;
        float y = r * sin(theta) * sin(phi) + center.y;
        float z = r * cos(theta) + center.z;
        return glm::vec3(x, y, z);
    }

    void addVertex(std::vector<float>& vertices, const glm::vec3& vertex, float theta, float phi) {
        vertices.push_back(vertex.x);
        vertices.push_back(vertex.y);
        vertices.push_back(vertex.z);
        // normals
        glm::vec3 normal = glm::normalize(vertex - center);
        vertices.push_back(normal.x);
        vertices.push_back(normal.y);
        vertices.push_back(normal.z);
        // texture coordinates
        float u = phi / (2.0f * M_PI);
        float v = theta / M_PI;
        vertices.push_back(u);
        vertices.push_back(v);
    }

};