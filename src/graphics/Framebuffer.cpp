#include "Framebuffer.h"

#include <GL/glew.h>

#include "Texture.h"
#include "../logger/Logger.h"

Framebuffer::Framebuffer(int width, int height) : width(width), height(height) {
	// glGenFramebuffers(1, &fbo);
	// bind();

	// GLuint tex;
	// glGenTextures(1, &tex);
	// glBindTexture(GL_TEXTURE_2D, tex);

	// glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);

    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex, 0);

    // texture = new Texture(tex, width, height);

    // glGenRenderbuffers(1, &depth);
    // glBindRenderbuffer(GL_RENDERBUFFER, depth);
    // glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, width, height);
    // glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depth);

    // GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    // if (status != GL_FRAMEBUFFER_COMPLETE) {
    //     LOG_CRITICAL("Framebuffer is incomplete! Status: 0x{:X}", status);
        
    //     switch (status) {
    //         case GL_FRAMEBUFFER_UNDEFINED:
    //             LOG_CRITICAL("Framebuffer undefined (default framebuffer does not exist)");
    //             break;
    //         case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
    //             LOG_CRITICAL("Incomplete attachment");
    //             break;
    //         case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
    //             LOG_CRITICAL("No attachments");
    //             break;
    //         case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
    //             LOG_CRITICAL("Incomplete draw buffer");
    //             break;
    //         case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
    //             LOG_CRITICAL("Incomplete read buffer");
    //             break;
    //         case GL_FRAMEBUFFER_UNSUPPORTED:
    //             LOG_CRITICAL("Unsupported format combination");
    //             break;
    //         case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:
    //             LOG_CRITICAL("Incomplete multisample");
    //             break;
    //         case GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS:
    //             LOG_CRITICAL("Incomplete layer targets");
    //             break;
    //         default:
    //             LOG_CRITICAL("Unknown error");
    //     }

    //     cleanup();
    //     throw std::runtime_error("Framebuffer creation failed");
    // }

    // unbind();
}

Framebuffer::~Framebuffer() {
	cleanup();
}

void Framebuffer::cleanup() {
    if (texture) {
        delete texture;
        texture = nullptr;
    }
    if (fbo) {
        glDeleteFramebuffers(1, &fbo);
        fbo = 0;
    }
    if (depth) {
        glDeleteRenderbuffers(1, &depth);
        depth = 0;
    }
}

void Framebuffer::bind() {
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
}

void Framebuffer::unbind() {
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
