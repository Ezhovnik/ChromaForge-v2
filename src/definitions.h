#ifndef SRC_DECLARATIONS_H_
#define SRC_DECLARATIONS_H_

#include "typedefs.h"

#define DEFAULT_BLOCK_NAMESPACE "chromaforge"

class ContentBuilder;

void setup_definitions(ContentBuilder* builder);
void setup_bindings();

#endif // SRC_DECLARATIONS_H_
