#include "syscall.h"


int
main()
{
     char *answerPtr1;
char *answerPtr2;
int bufferid1 = -1;
int bufferid2 = -1;
int result1 = -1;
int result2 = -1;
     bufferid1 =  SendMessage("../test/prog12","Message1 from sender",-1);
     bufferid2 =  SendMessage("../test/prog13","Message2 from sender",-1);
     result1  =  WaitAnswer(-1,answerPtr1,bufferid1);
     result2  =  WaitAnswer(-1,answerPtr2,bufferid2);
     Exit(0);
}
