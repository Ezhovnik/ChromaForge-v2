#ifndef GRAPHICS_FRAMEBUFFER_H_
#define GRAPHICS_FRAMEBUFFER_H_

#include "../typedefs.h"

class Texture;

class Framebuffer {
	uint fbo;
	uint depth;

    void cleanup();
public:
	uint width;
	uint height;

	Texture* texture;

	Framebuffer(uint width, uint height);
	~Framebuffer();

	void bind();
	void unbind();
};

#endif // GRAPHICS_FRAMEBUFFER_H_
