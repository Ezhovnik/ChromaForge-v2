#include "Batch2D.h"
#include "Mesh.h"

const int BATCH_2D_ATTRS[] = {
    2, 2, 4, 0
};

Batch2D::Batch2D(size_t capacity) : capacity(capacity), offset(0), color(1.0f, 1.0f, 1.0f, 1.0f){
	buffer = new float[capacity];
	mesh = new Mesh(nullptr, 0, BATCH_2D_ATTRS);
}

Batch2D::~Batch2D(){
	delete buffer;
	delete mesh;
}
