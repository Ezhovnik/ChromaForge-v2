#ifndef CODERS_PNG_H_
#define CODERS_PNG_H_

#include <string>

class Texture;

namespace png {
    extern Texture* loadTexture(std::string filename);
}

#endif // CODERS_PNG_H_
