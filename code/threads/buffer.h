#pragma once
#include "kernel.h"
#include "scheduler.h"
#include "thread.h"
class Kernel;
class Thread;
class Scheduler;
class Buffer
{
  public:
    Buffer(Thread *s, Thread *r);
    Buffer(Thread *s, Thread *r, int bufferid);
    Thread* getSender(){return this->s;}
    Thread* getReceiver(){return this->r;}
    int getBufferid(){return this->bufferid;}
    List<string>* getMsg(){return msg;}
    bool checkAvailable(){return this->availableCheck;}
    void setAvailable(bool f){this->availableCheck = f;}
  private:
    Thread *s;
    Thread *r;
    int bufferid;
    bool availableCheck;
    List<string> *msg;

  
};

