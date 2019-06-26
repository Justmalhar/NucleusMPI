#include "syscall.h"

int main()
{

int bufferid = -1;
int result = -1;
int result1 = -1;
char *message;
bufferid = WaitMessage("../test/prog3", message,bufferid);
bufferid = WaitMessage("../test/prog3", message,bufferid);

result = SendAnswer(result,"Answer from Prog4",bufferid);
result1 = SendAnswer(result1,"Answer from Prog4",bufferid);


	Exit(0);
}