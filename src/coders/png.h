#ifndef CODERS_PNG_H_
#define CODERS_PNG_H_

#include <string>

class Texture;
class ImageData;

namespace png {
    extern ImageData* loadImage(std::string filename);
    extern bool writeImage(std::string filename, const ImageData* image);
    extern Texture* loadTexture(std::string filename);
}

#endif // CODERS_PNG_H_
