#ifndef LIGHT_H
#define LIGHT_H

#include "glm/glm.hpp"

#include <iostream>
#include <vector>

class Light {

public:
	float intensity;
    glm::vec3 color, ambient, diffuse, specular;

    Light(float intensity, glm::vec3 color, glm::vec3 ambient, glm::vec3 diffuse, glm::vec3 specular) 
    : intensity(intensity), color(color), ambient(ambient), diffuse(diffuse), specular(specular) {

    }


};

#endif