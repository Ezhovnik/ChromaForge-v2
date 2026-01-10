#ifndef GRAPHICS_FRAMEBUFFER_H_
#define GRAPHICS_FRAMEBUFFER_H_

#include "../typedefs.h"

class Texture;

class Framebuffer {
	uint fbo;
	uint depth;

    void cleanup();
public:
	int width;
	int height;

	Texture* texture;

	Framebuffer(int width, int height);
	~Framebuffer();

	void bind();
	void unbind();
};

#endif // GRAPHICS_FRAMEBUFFER_H_
