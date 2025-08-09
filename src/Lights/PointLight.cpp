#ifndef POINTLIGHT_H
#define POINTLIGHT_H

#include "Light.cpp"

#include <iostream>
#include <vector>

class PointLight : public Light {

public:
    glm::vec3 position;
    float constant, linear, quadratic;

    PointLight(float intensity, glm::vec3 color, glm::vec3 position, glm::vec3 ambient, glm::vec3 diffuse, glm::vec3 specular, float constant, float linear, float quadratic) 
    : Light(intensity, color, ambient, diffuse, specular), position(position), constant(constant), linear(linear), quadratic(quadratic) {

    }

};

#endif