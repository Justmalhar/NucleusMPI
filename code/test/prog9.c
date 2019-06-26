#include "syscall.h"

int main()
{
int bufferid = -1;
int result = -1;
char *answer;

bufferid = SendMessage("../test/prog10", "Hello from the sender",bufferid);

	Exit(0);
}