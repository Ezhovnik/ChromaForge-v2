#ifndef INTERFACES_TASK_H_
#define INTERFACES_TASK_H_

#include "../typedefs.h"

class Task {
public:
    virtual ~Task() {}

    virtual bool isActive() const = 0;
    virtual uint getWorkTotal() const = 0;
    virtual uint getWorkDone() const = 0;
    virtual void update() = 0;
    virtual void waitForEnd() = 0;
    virtual void terminate() = 0;
};

#endif // INTERFACES_TASK_H_
