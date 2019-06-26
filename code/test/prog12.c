#include "syscall.h"


int
main()
{
     char *messagePtr;
int bufferid = -1;
int result = -1;
     bufferid =  WaitMessage("../test/prog11",messagePtr,bufferid);
     result  =  SendAnswer(-1,"Answer from prog12",bufferid);
     Exit(0);

}
