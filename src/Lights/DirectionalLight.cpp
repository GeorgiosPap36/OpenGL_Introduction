#ifndef DIRECTIONALLIGHT_H
#define DIRECTIONALLIGHT_H

#include "Light.cpp"

#include <iostream>
#include <vector>

class DirectionalLight : public Light {

public:
    glm::vec3 direction;

    DirectionalLight() : Light(1, glm::vec3(1.0f), glm::vec3(1.0f), glm::vec3(1.0f), glm::vec3(1.0f)) {

    }

    DirectionalLight(float intensity, glm::vec3 color, glm::vec3 ambient, glm::vec3 diffuse, glm::vec3 specular, glm::vec3 direction) 
    : Light(intensity, color, ambient, diffuse, specular), direction(direction) {
        
    }

};

#endif