#include "syscall.h"

int main()
{
int bufferid = -1;
int result = -1;
char *message;

bufferid = WaitMessage("../test/prog1", message,bufferid);
result = SendAnswer(result,"Answer from Prog2",bufferid);

	Exit(0);
}