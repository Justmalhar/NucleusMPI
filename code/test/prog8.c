#include "syscall.h"

int main()
{
int bufferid = -1;
int result = -1;
char *message;

bufferid = WaitMessage("../test/prog7", message,bufferid);

	Exit(0);
}