#include "syscall.h"

int main()
{

int bufferid = -1;
int result = -1;
char *answer;
bufferid = SendMessage("../test/prog8", "Hello from the sender",bufferid);
result = WaitAnswer(result,answer,bufferid);

	Exit(0);
}