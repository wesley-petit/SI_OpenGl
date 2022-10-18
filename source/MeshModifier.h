#pragma once
void CreateTriangleWithNormals(const std::vector<Triangle>& triangles, std::vector<TriangleWithNormal>& outTrianglesWithNormals)
{
	for (size_t i = 0; i < triangles.size(); i++)
	{
		auto& t = triangles[i];

		glm::vec3 a = t.p0 - t.p1;
		glm::vec3 b = t.p0 - t.p2;
		glm::vec3 n = glm::normalize(glm::cross(a, b));

		TriangleWithNormal trisWithNormal({ t.p0, n, t.p1, n, t.p2, n });
		outTrianglesWithNormals.push_back(trisWithNormal);
	}
}

void CenterAllVertex(std::vector<Triangle>& outTriangles)
{
    // Calcul du centre de l'objet
    glm::vec3 gravityCenter(0.0f);
	for (auto&& tris : outTriangles)
	{
		gravityCenter += tris.p0;
		gravityCenter += tris.p1;
		gravityCenter += tris.p2;
	}

    gravityCenter /= outTriangles.size() * 3;

    // Recentre les vertices en enlevant le centre de gravité
	for (auto&& tris : outTriangles)
	{
		tris.p0 -= gravityCenter;
		tris.p1 -= gravityCenter;
		tris.p2 -= gravityCenter;
	}
}