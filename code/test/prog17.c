#include "syscall.h"

int main()

{
	int bufferid1 = -1;
int bufferid2 = -1;
int result = -1;
int result2 = -1;

	bufferid1 = SendMessage("../test/prog18","Hello from the sender", bufferid1);
	bufferid2 = SendMessage("../test/prog19","Hello from the sender", bufferid2);

	if(bufferid1 == bufferid2)
	Exit(0);
} 