#include "syscall.h"


int
main()
{
     char *messagePtr;
int bufferid = -1;
int result = -1;
     bufferid =  WaitMessage("../test/prog14",messagePtr,bufferid);
     result  =  SendAnswer(result,"Answer from Prog15",bufferid);
     Exit(0);
}

