#include "syscall.h"

int main()
{
int bufferid = -1;
int result = -1;
int result1 = -1;
char *answer;

bufferid = SendMessage("../test/prog6", "Hello from the sender",bufferid);
result = WaitAnswer(result,answer,bufferid);

bufferid = SendMessage("../test/prog6", "Hello from the sender",bufferid);
result1 = WaitAnswer(result1,answer,bufferid);


	Exit(0);
}