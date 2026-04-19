#ifndef GRAPHICS_CORE_FRAMEBUFFER_H_
#define GRAPHICS_CORE_FRAMEBUFFER_H_

#include <memory>

#include <typedefs.h>

class Texture;

class Framebuffer {
	uint fbo;
	uint depth;
	uint width;
	uint height;
	uint format;
	std::unique_ptr<Texture> texture;
public:
	Framebuffer(uint fbo, uint depth, std::unique_ptr<Texture> texture);
	Framebuffer(uint width, uint height, bool alpha=false);
	~Framebuffer();

	void bind();
	void unbind();

	void resize(uint width, uint height);

	Texture* getTexture() const;

	uint getWidth() const;

	uint getHeight() const;
};

#endif // GRAPHICS_CORE_FRAMEBUFFER_H_
