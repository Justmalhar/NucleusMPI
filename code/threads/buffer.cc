#include "buffer.h"
#include "main.h"
#include <string>
Buffer::Buffer(Thread *s, Thread *r)
{
    availableCheck = true;
    this->s = s;
    this->r = r;
    this->bufferid = kernel->mBuffers->FindAndSet();
    this->msg = new List<string>();
}

Buffer::Buffer(Thread *s, Thread *r, int bufferid)
{
    availableCheck = true;
    this->s = s;
    this->r = r;
    this->bufferid = bufferid;
    this->msg = new List<string>();
}
