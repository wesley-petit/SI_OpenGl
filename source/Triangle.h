#pragma once
#include <glm/vec3.hpp>
#include <glm/glm.hpp>
#include <vector>

struct Triangle
{
	glm::vec3 p0, p1, p2;
};

struct TriangleWithNormal
{
	glm::vec3 p0, n0, p1, n1, p2, n2;
};
