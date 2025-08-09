#ifndef BOUNDINGBOX_H
#define BOUNDINGBOX_H

#include <iostream>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <limits>

class BoundingBox {
public:
    std::vector<float> vertices;
    glm::vec3 minVert;
    glm::vec3 maxVert;
    glm::mat4 transformation;

    BoundingBox() : transformation(glm::mat4(1.0f)) {}

    BoundingBox(std::vector<float> vertices, glm::vec3 minVert, glm::vec3 maxVert)
        : vertices(vertices), minVert(minVert), maxVert(maxVert), transformation(glm::mat4(1.0f)) {}

    // FIND OUT HOW IT ACTUALLY WORKS
    float projectOntoAxis(const glm::vec3& axis) {
        glm::vec3 center = (minVert + maxVert) * 0.5f;
        glm::vec3 extent = (maxVert - minVert) * 0.5f;

        glm::vec4 worldCenter = transformation * glm::vec4(center, 1.0f);

        glm::vec3 axes[3] = {
            glm::vec3(transformation[0]), // X-axis
            glm::vec3(transformation[1]), // Y-axis
            glm::vec3(transformation[2])  // Z-axis
        };

        float radius = 0.0f;
        for (int i = 0; i < 3; ++i) {
            radius += fabs(glm::dot(extent[i] * axes[i], axis));
        }

        float centerProj = glm::dot(glm::vec3(worldCenter), axis);

        return radius;
    }

    bool isIntersectingOtherBB(BoundingBox otherBB, glm::vec3 positionDelta) {
        glm::mat4 thisTransform = glm::translate(glm::mat4(1.0f), positionDelta) * transformation;

        glm::vec3 axes[15];
        axes[0] = glm::vec3(thisTransform[0]); // X-axis of this box
        axes[1] = glm::vec3(thisTransform[1]); // Y-axis of this box
        axes[2] = glm::vec3(thisTransform[2]); // Z-axis of this box
        axes[3] = glm::vec3(otherBB.transformation[0]); // X-axis of other box
        axes[4] = glm::vec3(otherBB.transformation[1]); // Y-axis of other box
        axes[5] = glm::vec3(otherBB.transformation[2]); // Z-axis of other box

        int index = 6;
        for (int i = 0; i < 3; ++i) {
            for (int j = 0; j < 3; ++j) {
                axes[index] = glm::cross(axes[i], axes[3 + j]);
                if (glm::length(axes[index]) > 0.0001f) {
                    axes[index] = glm::normalize(axes[index]);
                } else {
                    axes[index] = glm::vec3(0.0f);
                }
                ++index;
            }
        }

        glm::mat4 originalTransform = transformation;
        transformation = thisTransform;

        for (int i = 0; i < 15; ++i) {
            if (glm::length(axes[i]) < 0.0001f) {
                continue;
            }

            float r1 = projectOntoAxis(axes[i]);
            float r2 = otherBB.projectOntoAxis(axes[i]);

            glm::vec3 c1 = glm::vec3(thisTransform * glm::vec4((minVert + maxVert) * 0.5f, 1.0f));
            glm::vec3 c2 = glm::vec3(otherBB.transformation * glm::vec4((otherBB.minVert + otherBB.maxVert) * 0.5f, 1.0f));
            float dist = fabs(glm::dot(c1 - c2, axes[i]));

            if (dist > (r1 + r2)) {
                transformation = originalTransform;
                return false; // Separating axis found
            }
        }

        transformation = originalTransform;

        // No separating axis found
        return true;
    }

    void setTransformation(glm::mat4 model) {
        transformation = model;
    }
};

#endif