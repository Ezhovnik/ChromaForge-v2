#include "Batch3D.h"

#include <GL/glew.h>

#include "Mesh.h"
#include "Texture.h"
#include "../typedefs.h"

#define VERTEX_SIZE 9

Batch3D::Batch3D(size_t capacity) : capacity(capacity), offset(0) {
	const vattr attrs[] = {{3}, {2}, {4}, {0}};
    
    buffer = new float[capacity * VERTEX_SIZE];
	mesh = new Mesh(buffer, 0, attrs);
	index = 0;

	ubyte pixels[] = {255, 255, 255, 255};
	blank = new Texture(pixels, 1, 1, GL_RGBA);
	_texture = nullptr;
}

Batch3D::~Batch3D(){
	delete blank;
	delete[] buffer;
	delete mesh;
}

void Batch3D::begin(){
	_texture = nullptr;
	blank->bind();
}

void Batch3D::vertex(float x, float y, float z, float u, float v, float r, float g, float b, float a) {
	buffer[index++] = x;
	buffer[index++] = y;
	buffer[index++] = z;

	buffer[index++] = u;
	buffer[index++] = v;

	buffer[index++] = r;
	buffer[index++] = g;
	buffer[index++] = b;
	buffer[index++] = a;
}

void Batch3D::vertex(glm::vec3 point, glm::vec2 uvpoint, float r, float g, float b, float a) {
	buffer[index++] = point.x;
	buffer[index++] = point.y;
	buffer[index++] = point.z;

	buffer[index++] = uvpoint.x;
	buffer[index++] = uvpoint.y;

	buffer[index++] = r;
	buffer[index++] = g;
	buffer[index++] = b;
	buffer[index++] = a;
}

void Batch3D::vertex(glm::vec3 coord, float u, float v, float r, float g, float b, float a) {
	buffer[index++] = coord.x;
	buffer[index++] = coord.y;
	buffer[index++] = coord.z;

	buffer[index++] = u;
	buffer[index++] = v;

	buffer[index++] = r;
	buffer[index++] = g;
	buffer[index++] = b;
	buffer[index++] = a;
}

void Batch3D::face(const glm::vec3& coord, float w, float h,
		const glm::vec3& axisX,
		const glm::vec3& axisY,
		const UVRegion& region,
		const glm::vec4& tint
	) {
	if (index + VERTEX_SIZE * 4 > capacity) flush();

	vertex(coord, region.u1, region.v1, tint.r, tint.g, tint.b, tint.a);
	vertex(coord + axisX * w, region.u2, region.v1, tint.r, tint.g, tint.b, tint.a);
	vertex(coord + axisX * w + axisY * h, region.u2, region.v2, tint.r, tint.g, tint.b, tint.a);

	vertex(coord, region.u1, region.v1, tint.r, tint.g, tint.b, tint.a);
	vertex(coord + axisX * w + axisY * h, region.u2, region.v2, tint.r, tint.g, tint.b, tint.a);
	vertex(coord + axisY * h, region.u1, region.v2, tint.r, tint.g, tint.b, tint.a);
}

void Batch3D::texture(Texture* new_texture){
	if (_texture == new_texture) return;
	flush();
	_texture = new_texture;
	if (new_texture == nullptr) blank->bind();
	else new_texture->bind();
}

void Batch3D::sprite(glm::vec3 pos, glm::vec3 up, glm::vec3 right, float w, float h, const UVRegion& uv, glm::vec4 color) {
	const float r = color.r;
	const float g = color.g;
	const float b = color.b;
	const float a = color.a;
	if (index + 6 * VERTEX_SIZE >= capacity) flush();

	vertex(pos.x - right.x * w - up.x * h,
			pos.y - right.y * w - up.y * h,
			pos.z - right.z * w - up.z * h,
			uv.u1, uv.v1,
			r, g, b, a
		);

	vertex(pos.x + right.x * w + up.x * h,
			pos.y + right.y * w + up.y * h,
			pos.z + right.z * w + up.z * h,
			uv.u2, uv.v2,
			r, g, b, a
		);

	vertex(pos.x - right.x * w - up.x * h,
			pos.y + right.y * w + up.y * h,
			pos.z - right.z * w - up.z * h,
			uv.u1, uv.v2,
			r, g, b, a
		);

	vertex(pos.x - right.x * w - up.x * h,
			pos.y - right.y * w - up.y * h,
			pos.z - right.z * w - up.z * h,
			uv.u1, uv.v1,
			r, g, b, a
		);

	vertex(pos.x + right.x * w + up.x * h,
			pos.y - right.y * w - up.y * h,
			pos.z + right.z * w + up.z * h,
			uv.u2, uv.v1,
			r, g, b, a
		);

	vertex(pos.x + right.x * w + up.x * h,
			pos.y + right.y * w + up.y * h,
			pos.z + right.z * w + up.z * h,
			uv.u2, uv.v2,
			r, g, b, a
		);
}

inline glm::vec4 do_tint(float value) {
	return glm::vec4(value, value, value, 1.0f);
}

void Batch3D::xSprite(float w, float h, const UVRegion& uv, const glm::vec4 tint, bool shading) {
	face(glm::vec3(-w *0.25f, 0.0f, 0.0f - w *0.25f), w, h, glm::vec3(1, 0, 0), glm::vec3(0, 1, 0), uv, (shading ? do_tint(1.0f) * tint : tint));
	face(glm::vec3(w * 0.25f, 0.0f, w * 0.5f - w *0.25f), w, h, glm::vec3(0, 0, -1), glm::vec3(0, 1, 0), uv, (shading ? do_tint(0.9f) * tint : tint));
}

void Batch3D::blockCube(const glm::vec3 size, const UVRegion(&texfaces)[6], const glm::vec4 tint, bool shading) {
	glm::vec3 coord = (1.0f - size) * -0.45f;
	face(coord + glm::vec3(0.0f, 0.0f, 0.0f), size.x, size.y, glm::vec3(1, 0, 0), glm::vec3(0, 1, 0), texfaces[5], (shading ? do_tint(0.8) * tint : tint));
	face(coord + glm::vec3(size.x, 0.0f, -size.z), size.x, size.y, glm::vec3(-1, 0, 0), glm::vec3(0, 1, 0), texfaces[4], (shading ? do_tint(0.8f) * tint : tint));
	face(coord + glm::vec3(0.0f, size.y, 0.0f), size.x, size.z, glm::vec3(1, 0, 0), glm::vec3(0, 0, -1), texfaces[3], (shading ? do_tint(1.0f) * tint : tint));
	face(coord + glm::vec3(0.0f, 0.0f, -size.z), size.x, size.z, glm::vec3(1, 0, 0), glm::vec3(0, 0, 1), texfaces[2], (shading ? do_tint(0.7f) * tint : tint));
	face(coord + glm::vec3(0.0f, 0.0f, -size.z), size.z, size.y, glm::vec3(0, 0, 1), glm::vec3(0, 1, 0), texfaces[0], (shading ? do_tint(0.9f) * tint : tint));
	face(coord + glm::vec3(size.x, 0.0f, 0.0f), size.z, size.y, glm::vec3(0, 0, -1), glm::vec3(0, 1, 0), texfaces[1], (shading ? do_tint(0.9f) * tint : tint));
}

void Batch3D::flush() {
	mesh->reload(buffer, index / VERTEX_SIZE);
	mesh->draw(GL_TRIANGLES);
	index = 0;
}
