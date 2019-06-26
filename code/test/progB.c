#include "syscall.h"
int
main(){
int buffer;
buffer = WaitMessage("../test/progA","../test/progB",-1);
buffer = WaitMessage("../test/progA","../test/progB",buffer);

SendAnswer("Answer 1 from prog B","progA",buffer);
SendAnswer("Answer 2 from progB","progA",buffer);
}
