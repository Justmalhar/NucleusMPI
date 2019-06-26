#include "syscall.h"

int main()
{
int bufferid = -1;
int result = -1;
int result1 = -1;
char *message;

bufferid = WaitMessage("../test/prog5", message,bufferid);
result = SendAnswer(result,"Answer from Prog6",bufferid);

bufferid = WaitMessage("../test/prog5", message,bufferid);
result1 = SendAnswer(result1,"Answer from Prog6",bufferid);


	Exit(0);
}