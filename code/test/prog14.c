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
     bufferid1 =  SendMessage("../test/prog15","Message1 from sender",bufferid1);
     bufferid2 =  SendMessage("../test/prog16","Message2 from sender",bufferid1);
	
if(bufferid1 != -1){
result1  =  WaitAnswer(-1,answerPtr1,bufferid1);
}

if(bufferid2 != -1){
result2  =  WaitAnswer(-1,answerPtr2,bufferid2);
}     

     Exit(0);

}
