#pragma once

#include <glm/vec3.hpp>
#include <vector>

#include "Triangle.h"

std::vector<Triangle> ReadStl(const char * filename);
