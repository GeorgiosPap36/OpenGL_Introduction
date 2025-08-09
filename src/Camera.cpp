#ifndef CAMERA_H
#define CAMERA_H

#include "BoundingBox.cpp"
#include "Primitives/Primitive.cpp"

#include "../dependencies/glad.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

enum Camera_Movement {
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT
};

// Default camera values
const float YAW = -90.0f;
const float PITCH = 0.0f;
const float SPEED = 2.5f;
const float SENSITIVITY = 0.1f;
const float ZOOM = 45.0f;


class Camera {
public:
    // camera Attributes
    glm::vec3 Position;
    glm::vec3 Front;
    glm::vec3 Up;
    glm::vec3 Right;
    glm::vec3 WorldUp;
    // euler Angles
    float Yaw;
    float Pitch;
    // camera options
    float MovementSpeed;
    float MouseSensitivity;
    float Zoom;

    BoundingBox bb;

    Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), float yaw = YAW, float pitch = PITCH) : Front(glm::vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(SPEED), MouseSensitivity(SENSITIVITY), Zoom(ZOOM) {
        Position = position;
        WorldUp = up;
        Yaw = yaw;
        Pitch = pitch;
        updateCameraVectors();

        float minX = -0.5f;
        float maxX =  0.5f;
        float minY = -0.5f;
        float maxY =  0.5f;
        float minZ = -0.5f;
        float maxZ =  0.5f;


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

        glm::vec3 minVert = glm::vec3(minX, minY, minZ);
		glm::vec3 maxVert = glm::vec3(maxX, maxY, maxZ);
		
		bb = BoundingBox(bbVertices, minVert, maxVert);
        updateBoundingBox(Position);
    }

    Camera(float posX, float posY, float posZ, float upX, float upY, float upZ, float yaw, float pitch) : Front(glm::vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(SPEED), MouseSensitivity(SENSITIVITY), Zoom(ZOOM) {
        Position = glm::vec3(posX, posY, posZ);
        WorldUp = glm::vec3(upX, upY, upZ);
        Yaw = yaw;
        Pitch = pitch;
        updateCameraVectors();

        float minX = -0.5f;
        float maxX =  0.5f;
        float minY = -0.5f;
        float maxY =  0.5f;
        float minZ = -0.5f;
        float maxZ =  0.5f;


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

        glm::vec3 minVert = glm::vec3(minX, minY, minZ);
		glm::vec3 maxVert = glm::vec3(maxX, maxY, maxZ);
		
		bb = BoundingBox(bbVertices, minVert, maxVert);
        updateBoundingBox(Position);
    }

    glm::mat4 GetViewMatrix() {
        return glm::lookAt(Position, Position + Front, Up);
    }

    void ProcessKeyboard(Camera_Movement direction, float deltaTime, std::vector<Primitive> objects) {
        float velocity = MovementSpeed * deltaTime;
        glm::vec3 positionDelta = glm::vec3(0, 0, 0);

        bool intersects = false;

        if (direction == FORWARD) {
            positionDelta = Front * velocity;
        }
        if (direction == BACKWARD) {
            positionDelta = -Front * velocity;
        }
        if (direction == LEFT) {
            positionDelta = -Right * velocity;
        }
        if (direction == RIGHT) {
            positionDelta = Right * velocity;
        }

        // std::cout << "PositionDelta: " << positionDelta.x << ", " << positionDelta.y << ", " << positionDelta.z << std::endl;

        for (int i = 0; i < objects.size(); i++) {
            if (bb.isIntersectingOtherBB(objects[i].bb, positionDelta)) {
                intersects = true;
                // std::cout << "COLLISION" << " with " << i << " object!" << std::endl;
                break; 
            }
        }

        if (!intersects) {
            Position += positionDelta;
            updateBoundingBox(positionDelta);
        }
    }

    void ProcessMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch = true) {
        xoffset *= MouseSensitivity;
        yoffset *= MouseSensitivity;

        Yaw += xoffset;
        Pitch += yoffset;

        // make sure that when pitch is out of bounds, screen doesn't get flipped
        if (constrainPitch) {
            if (Pitch > 89.0f)
                Pitch = 89.0f;
            if (Pitch < -89.0f)
                Pitch = -89.0f;
        }

        updateCameraVectors();
    }

private:
    void updateCameraVectors() {
        glm::vec3 front;
        front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
        front.y = sin(glm::radians(Pitch));
        front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
        Front = glm::normalize(front);

        Right = glm::normalize(glm::cross(Front, WorldUp));  // normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
        Up = glm::normalize(glm::cross(Right, Front));
    }

    void updateBoundingBox(glm::vec3 positionDelta) {
        bb.minVert += positionDelta;
        bb.maxVert += positionDelta;
    }
};
#endif