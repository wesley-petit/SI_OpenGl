#include "stl.h"

#include <fstream>

std::vector<Triangle> ReadStl(const char * filename)
{
	std::ifstream file(filename, std::ios::in | std::ios::binary);
	if (file.is_open())
	{
		// skip header
		file.seekg(80);

		unsigned triCount;
		file.read((char*) &triCount, 4);

		std::vector<Triangle> tris;
		tris.reserve(triCount);

		for(unsigned i = 0; i < triCount; ++i)
		{
			glm::vec3 p0, p1, p2;

			// skip normal
			file.seekg(3 * 4, std::ios_base::cur);

			file.read((char*) &p0, sizeof(glm::vec3));
			file.read((char*) &p1, sizeof(glm::vec3));
			file.read((char*) &p2, sizeof(glm::vec3));

			// skip attribute
			file.seekg(2, std::ios_base::cur);

			tris.push_back({p0, p1, p2});
		}

		return tris;
	}
	else
	{
		throw std::runtime_error(std::string("Cannot open file: ") + filename);
		return {};
	}
}
