#include "syscall.h"
int 
main(){
int buffer;
int buffer2;
int buffer3;

buffer = SendMessage("../test/progB", "Message 1 from A to B", -1);
buffer = SendMessage("../test/progB", "Message 2 from A to B", buffer);

WaitAnswer("progB","progA",buffer);
WaitAnswer("progB","progA",buffer);

Exit(0);
}
