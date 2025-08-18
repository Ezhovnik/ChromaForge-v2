#include "../Header Files/SkyboxParams.h"

const Vertex skyboxVertices[] =
{
	Vertex{glm::vec3(-1.0f, -1.0f,  1.0f)},
	Vertex{glm::vec3(-1.0f, -1.0f, -1.0f)},
	Vertex{glm::vec3(1.0f, -1.0f, -1.0f)},
	Vertex{glm::vec3(1.0f, -1.0f,  1.0f)},
	Vertex{glm::vec3(-1.0f,  1.0f,  1.0f)},
	Vertex{glm::vec3(-1.0f,  1.0f, -1.0f)},
	Vertex{glm::vec3(1.0f,  1.0f, -1.0f)},
	Vertex{glm::vec3(1.0f,  1.0f,  1.0f)}
};

const GLuint skyboxIndices[] =
{
	0, 2, 1,
	0, 3, 2,
	0, 4, 7,
	0, 7, 3,
	3, 7, 6,
	3, 6, 2,
	2, 6, 5,
	2, 5, 1,
	1, 5, 4,
	1, 4, 0,
	4, 5, 6,
	4, 6, 7
};

const size_t skyboxVerticesCount = sizeof(skyboxVertices)/sizeof(Vertex);
const size_t skyboxIndicesCount = sizeof(skyboxIndices)/sizeof(GLuint);