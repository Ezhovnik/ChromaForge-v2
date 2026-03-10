#ifndef INTERFACES_OBJECT_H_
#define INTERFACES_OBJECT_H_

#include <stdlib.h>
#include <stdint.h>

class Level;

class Object {
private:

public:
    uint64_t objectUID;    
    bool shouldUpdate = true;

public:
    ~Object() { destroyed(); }

public:
    virtual void spawned() {  }
    virtual void update(float delta) { }
    virtual void destroyed() {  }
};

#endif // INTERFACES_OBJECT_H_
