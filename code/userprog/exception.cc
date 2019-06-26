// exception.cc
//	Entry point into the Nachos kernel from user programs.
//	There are two kinds of things that can cause control to
//	transfer back to here from user code:
//
//	syscall -- The user code explicitly requests to call a procedure
//	in the Nachos kernel.  Right now, the only function we support is
//	"Halt".
//
//	exceptions -- The user code does something that the CPU can't handle.
//	For instance, accessing memory that doesn't exist, arithmetic errors,
//	etc.
//
//	Interrupts (which can also cause control to transfer from user
//	code into the Nachos kernel) are handled elsewhere.
//
// For now, this only handles the Halt() system call.
// Everything else core dumps.
//
// Copyright (c) 1992-1996 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation
// of liability and disclaimer of warranty provisions.

#include "../lib/copyright.h"
#include "../threads/main.h"
#include "syscall.h"
#include "ksyscall.h"
#include "../threads/thread.h"
#include "buffer.h"
#include <vector>
#include "sstream"
//----------------------------------------------------------------------
// ExceptionHandler
// 	Entry point into the Nachos kernel.  Called when a user program
//	is executing, and either does a syscall, or generates an addressing
//	or arithmetic exception.
//
// 	For system calls, the following is the calling convention:
//
// 	system call code -- r2
//		arg1 -- r4
//		arg2 -- r5
//		arg3 -- r6
//		arg4 -- r7
//
//	The result of the system call, if any, must be put back into r2.
//
// If you are handling a system call, don't forget to increment the pc
// before returning. (Or else you'll loop making the same system call forever!)
//
//	"which" is the kind of exception.  The list of possible exceptions
//	is in machine.h.
//----------------------------------------------------------------------

char *DUMMY_MESSAGE = "Dummy Message in case of exception!";
// Incase of any errors( or abnormal communication ) we need the dummy message
void cleanBuffer(int bufferID)
{
if (bufferID >= 0)
kernel->clearMB(bufferID);
kernel->currentThread->Finish();
}
// This is for reusage of the buffer.
// Clearing the buffer once the job is done and then terminating the currentThread.

string getStringPTR(int begin)
{
string str = "";
int value;
while (true)
{
kernel->machine->ReadMem(begin, 1, &value);
if ((char)value != '\0')
{
	str += (char)value;
	begin++;
}
else
{
	break;
}
}

return str;
}

//This function is used for getting the int values from reading the registers and converting the register values which are integers to strings.

void setStringPTR(string m, int begin)
{
while (true)
{
if (m != "\0")
{
int msg;
stringstream(m) >> msg;
kernel->machine->WriteMem(begin, 1, msg);
begin++;
}
else
break;
}
}

// This function sets or writes the string into memory.

Buffer *communication_val(int id, Thread *receiverProcess)
{
Buffer *msg_buff = kernel->msg_buff_Q[id]->Front();
if (msg_buff != NULL)
{
if (msg_buff->getReceiver() == receiverProcess)
{
return msg_buff;
}
}
}

//This function takes the first available buffer and then 
//the receiver process is assigned to it.

Buffer *fetch_buff(int id, string sender)
{
std::map<int, Buffer *> msg_q = kernel->currentThread->pMQueue; //Creating a map of messages queue which has int and Buffer* as parameters
if (!msg_q.empty()) 
{
if (msg_q.find(id) != msg_q.end())
{
return msg_q[id]; // if the id we are looking for is not at the end of the queue return the id.
}
else if (sender != "") // if sender name is not empty
{
std::vector<Buffer *> v; //creating a vector of buffers
for (map<int, Buffer *>::iterator it = msg_q.begin(); it != msg_q.end(); ++it) // for all the messages
{
	v.push_back(it->second); //add it to the vector
}
for (vector<Buffer *>::iterator it = v.begin(); it != v.end(); it++) // for all the elements of the vectors
{
	if ((*it)->getSender()->getName() == sender) // if the sender name matches return the vector iterator details at that point
	{
		return (*it);
	}
}
}
}
return NULL;
}

void ExceptionHandler(ExceptionType which)
{
int type = kernel->machine->ReadRegister(2);

DEBUG(dbgSys, "Received Exception " << which << " type: " << type << "\n");

switch (which)
{
case SyscallException:
switch (type)
{
case SC_Halt:
DEBUG(dbgSys, "Shutdown, initiated by user program.\n");
cout<<"Shutdown, initiated by user program.\n";
SysHalt();

ASSERTNOTREACHED();
break;

case SC_Add:
DEBUG(dbgSys, "Add " << kernel->machine->ReadRegister(4) << " + " << kernel->machine->ReadRegister(5) << "\n");

/* Process SysAdd Systemcall*/
int result;
result = SysAdd(/* int op1 */ (int)kernel->machine->ReadRegister(4),
				/* int op2 */ (int)kernel->machine->ReadRegister(5));

DEBUG(dbgSys, "Add returning with " << result << "\n");
/* Prepare Result */
kernel->machine->WriteRegister(2, (int)result);

/* Modify return point */
{
	/* set previous programm counter (debugging only)*/
	kernel->machine->WriteRegister(PrevPCReg, kernel->machine->ReadRegister(PCReg));

	/* set programm counter to next instruction (all Instructions are 4 byte wide)*/
	kernel->machine->WriteRegister(PCReg, kernel->machine->ReadRegister(PCReg) + 4);

	/* set next programm counter for brach execution */
	kernel->machine->WriteRegister(NextPCReg, kernel->machine->ReadRegister(PCReg) + 4);
}

return;

ASSERTNOTREACHED();

break;
case SC_Exit:
{
IntStatus oldLevel = kernel->interrupt->SetLevel(IntOff);
kernel->currentThread->Finish();  
(void) kernel->interrupt->SetLevel(oldLevel);       
return;
}
/*

The send message system call.
Here, the message is copied into the first available buffer within the 
pool and then delivered to a queue of a named receiver at which point the
receiver is activated if it waiting for a message. The sender continues after 
it is told of the message buffer’s identity.

*/
case SC_SendMessage:
{
cout << "\nSC_SendMessage System Call\n";
IntStatus oldLevel = kernel->interrupt->SetLevel(IntOff);
int r = kernel->machine->ReadRegister(4);  //reading first argument i.e. receiver
int message = kernel->machine->ReadRegister(5); // message is the second argument
int id_buff = kernel->machine->ReadRegister(6); // buffer id is the third argument
string receiver = getStringPTR(r); // string conversion from int
string msg = getStringPTR(message); // string conversion from int

Thread *t = Thread::getTByName(receiver);  // creating a new thread (reciever thread)

if (t->getName() == receiver) // if correct receiver
{
	if (kernel->currentThread->getmsgCount() <= kernel->getMessageLimit()) // checking for the max limit of messages
	{
		if (id_buff == -1) // initial communication
		{
			Buffer *buffer1 = new Buffer(kernel->currentThread, t); // creating a new buffer
			if (buffer1->getBufferid() == -1)	// bufferid is -1. This is abnormal communication. The buffer is not available
			{
				cout << "Buffer not available. " << std::endl;
				kernel->machine->WriteRegister(2, buffer1->getBufferid());
				/* set previous programm counter (debugging only)*/
				kernel->machine->WriteRegister(PrevPCReg, kernel->machine->ReadRegister(PCReg));

				/* set programm counter to next instruction (all Instructions are 4 byte wide)*/
				kernel->machine->WriteRegister(PCReg, kernel->machine->ReadRegister(PCReg) + 4);

				/* set next programm counter for brach execution */
				kernel->machine->WriteRegister(NextPCReg, kernel->machine->ReadRegister(PCReg) + 4);
				(void)kernel->interrupt->SetLevel(oldLevel);
				return;
				break;
			}
		std::cout << "\nBuffer : " << buffer1->getBufferid() << "\nP1= " << kernel->currentThread->getName() << "\nP2= " << receiver << std::endl;
		// Printing the bufferid and the two communicating processes
		kernel->currentThread->bufferr.insert(std::pair<int, string> (buffer1->getBufferid(), receiver));
		// buffer and the thread or processes communication being established
			buffer1->getMsg()->Append(msg);
			//append the message to the buffer
			List<Buffer *> *messageBufferList = new List<Buffer *>();
			// creating a new buffer list
			messageBufferList->Append(buffer1);
			// adding buffer1 to that list
			kernel->msg_buff_Q[buffer1->getBufferid()] = messageBufferList;
			//passing the updated list values
			t->pMQueue[buffer1->getBufferid()] = buffer1;

			//return value to user program
			kernel->machine->WriteRegister(2, buffer1->getBufferid());

						std::cout << "\nMessage from "<< kernel->currentThread->getName() <<" to Process " << receiver << "\n Message : " << msg <<endl;
						//printing out the message
		}
		else
		{
			//check for valid sender and receiver pair
			Buffer *msg_buff = communication_val(id_buff, t);
			if (msg_buff != NULL)
			{
				msg_buff->getMsg()->Append(msg);
				t->pMQueue[id_buff] = msg_buff;

				//return value to user program
				kernel->machine->WriteRegister(2, id_buff);
				cout << "\nMessage from "<< kernel->currentThread->getName() <<" to Process " << receiver << "\n Message : " << msg <<endl;
				// This future normal communications or executions, after we have established the connection.
			}
			else
			{
				//Pass some error message
				kernel->machine->WriteRegister(2, -1);
							std::cout << "SysCall Error ";
			}
		}
	}
	else
	{
		// This loop gets executed when we exceed the capacity of the message limit
		Buffer *msg_buff = communication_val(id_buff, t);
		if (msg_buff != NULL)
		{
			msg_buff->getMsg()->Append(DUMMY_MESSAGE); // Appending the dummy or erroe message into the buffer
			t->pMQueue[id_buff] = msg_buff;

			//return value to user program
			kernel->machine->WriteRegister(2, id_buff);
		}

					std::cout << "Buffer Limit exceeded" << kernel->currentThread->getName() << std::endl;
					std::cout << "Dummy Response"<<std::endl;
					// Printing the error messages
	}
}
else
{
	cleanBuffer(id_buff);	// cleaning up the buffer for future use
	std::cout << "Dummy Response" << std::endl; // Case of abnormal communication
}
/* set previous programm counter (debugging only)*/
kernel->machine->WriteRegister(PrevPCReg, kernel->machine->ReadRegister(PCReg));

/* set programm counter to next instruction (all Instructions are 4 byte wide)*/
kernel->machine->WriteRegister(PCReg, kernel->machine->ReadRegister(PCReg) + 4);

/* set next programm counter for brach execution */
kernel->machine->WriteRegister(NextPCReg, kernel->machine->ReadRegister(PCReg) + 4);
(void)kernel->interrupt->SetLevel(oldLevel);
return;
break;
}

/*

The wait message system call.
This primitive delays the requesting process until a message arrives in the queue of the process.
Now when the process is allowed to proceed, it is given name of sender, contents o message and 
identity of buffer after which the buffer is removed from the queue and it is made ready for transmitting the answer.

*/

case SC_WaitMessage:
{
cout<<"**********"<<endl;
cout << "\nSC_WaitMessage\n";IntStatus oldLevel = kernel->interrupt->SetLevel(IntOff);
int sendP = kernel->machine->ReadRegister(4); // reading the first argument (sender)
string sender = getStringPTR(sendP);	// string conversion
int id_buff = kernel->machine->ReadRegister(6);	// bufferid collection from register 6

if (Thread::getTByName(sender) != NULL) // if name is not NULL
{
	Thread *sndr = Thread::getTByName(sender); // Sending name to the thread sndr
	std::map<int, string>::iterator loop;	// creating a map for the iterator
	// this for loop keeps track of the life of current thread
	for (loop = sndr->bufferr.begin(); loop != sndr->bufferr.end(); ++loop)
	{
		if (loop->second == kernel->currentThread->getName())
		{
			id_buff = loop->first;
			break;
		}
	}
	Buffer *msg_buff = fetch_buff(id_buff, sender); // call the fetch function for the sender
	if (msg_buff != NULL && !msg_buff->getMsg()->IsEmpty())
	{
		cout<<"**********"<<endl;
		cout<<"\nBuffer ID is :"<<id_buff<<endl;
		string msg = msg_buff->getMsg()->RemoveFront(); //getting the message as a string
		kernel->machine->WriteRegister(2, msg_buff->getBufferid()); // writing it to the register
		cout << "\nMessage recieved from "<< kernel->currentThread->getName() << "\n Message : " << msg <<endl;
		//printing the message information
		kernel->currentThread->msg_count++; // increase message count
		/* set previous programm counter (debugging only)*/
		kernel->machine->WriteRegister(PrevPCReg, kernel->machine->ReadRegister(PCReg));

		/* set programm counter to next instruction (all Instructions are 4 byte wide)*/
		kernel->machine->WriteRegister(PCReg, kernel->machine->ReadRegister(PCReg) + 4);

		/* set next programm counter for brach execution */
		kernel->machine->WriteRegister(NextPCReg, kernel->machine->ReadRegister(PCReg) + 4);
		(void)kernel->interrupt->SetLevel(oldLevel);
		return;
		break;
	}
	else
	{
		kernel->currentThread->Yield();
	}
}
else
{
	Buffer *msg_buff = fetch_buff(id_buff, sender);
	// This loop accounts for the normal communication and proper SC_WaitMessage syscall
	if (msg_buff != NULL && !msg_buff->getMsg()->IsEmpty()){
		string msg = msg_buff->getMsg()->RemoveFront(); // getting the message
		kernel->machine->WriteRegister(2, msg_buff->getBufferid()); // getting bufferid and writing it in the register
		std::cout << "\nMessage received from process " << sender << " by process " << kernel->currentThread->getName() << "\n Message:" << ":" << msg << std::endl;
		kernel->currentThread->msg_count++;
		//increasing the message count
	}
	else
	{
		cleanBuffer(id_buff); // clearing the buffer
		kernel->machine->WriteRegister(2, -1);
		std::cout << "Process: " << sender << " exited" <<std::endl;
		std::cout << "Dummy Message " << kernel->currentThread->getName() << std::endl;	} //abnormal execution
	/* set previous programm counter (debugging only)*/
	kernel->machine->WriteRegister(PrevPCReg, kernel->machine->ReadRegister(PCReg));

	/* set programm counter to next instruction (all Instructions are 4 byte wide)*/
	kernel->machine->WriteRegister(PCReg, kernel->machine->ReadRegister(PCReg) + 4);

	/* set next programm counter for brach execution */
	kernel->machine->WriteRegister(NextPCReg, kernel->machine->ReadRegister(PCReg) + 4);
	(void)kernel->interrupt->SetLevel(oldLevel);
	return;
	break;
}

(void)kernel->interrupt->SetLevel(oldLevel);
return;
break;
}
/*

the send answer system call
Copies answer onto the buffer where the message is received and delivers the message 
in the queue of the original sender and now the sender of the message gets activated 
if waiting for the answer. And now the answering process continues right after.


*/
case SC_SendAnswer:
{
cout<<"**********"<<endl;
cout<<"\nSC_SendAnswer\n";
IntStatus oldLevel = kernel->interrupt->SetLevel(IntOff);

int tempPtr = kernel->machine->ReadRegister(5); // reading the result or the reply as an int from register 5
string reply = getStringPTR(tempPtr); // string conversion
int id_buff = kernel->machine->ReadRegister(6); // buffer id is collected from register 6

Buffer *msg_buff = kernel->currentThread->pMQueue[id_buff]; // assigning buffer
if (msg_buff != NULL) // if not empty
{
	Thread *rev = msg_buff->getSender(); // getting the sender information and giving it to rev thread
	if (rev != NULL && Thread::IsThreadAvailable(rev->getThreadId())) // if it is available (normal execution)
	{
		if (rev->pMQueue[id_buff] == NULL)
		{
			Buffer *buffer1 = new Buffer(kernel->currentThread, rev, id_buff); //creating a new buffer
			// appending the reply to buffer1
			buffer1->getMsg()->Append(reply);
			kernel->msg_buff_Q[id_buff]->Append(buffer1);
			rev->pMQueue[id_buff] = buffer1;
		}
		else
		{
			rev->pMQueue[id_buff]->getMsg()->Append(reply);
		}
		kernel->machine->WriteRegister(2, kernel->currentThread->getThreadId());
		// normal execution. printing the message
					std::cout << "\nAnswer by process " << kernel->currentThread->getName() << " Process " <<  "sent to " << rev->getName() << " with message : " << reply << std::endl;
		kernel->currentThread->msg_count--;
		// decreasing the count, once the answer is received.
	}
	else
	{
		cleanBuffer(id_buff); //cleaning the buffer
		//abnormal execution receiver is not available
		kernel->machine->WriteRegister(2, -1);
					std::cout << "Receiver not available" << std::endl;
	}
}

kernel->currentThread->Yield();
/* set previous programm counter (debugging only)*/
kernel->machine->WriteRegister(PrevPCReg, kernel->machine->ReadRegister(PCReg));

/* set programm counter to next instruction (all Instructions are 4 byte wide)*/
kernel->machine->WriteRegister(PCReg, kernel->machine->ReadRegister(PCReg) + 4);

/* set next programm counter for brach execution */
kernel->machine->WriteRegister(NextPCReg, kernel->machine->ReadRegister(PCReg) + 4);
(void)kernel->interrupt->SetLevel(oldLevel);
return;
break;
}
/*

the wait answer system call.
Delays requesting process until an answer arrives in buffer after which this answer
is copied into the process and the buffer would be returned to the pool. The result
would specify if an answer is a response from another process or if it is a dummy answer
which maybe generated by the system nucleus which is a response to a message addressed to 
a non-existing process.

*/
case SC_WaitAnswer:
{

cout<<"**********"<<endl;
cout<<"\nSC_WaitAnswer\n";
IntStatus oldLevel = kernel->interrupt->SetLevel(IntOff);
int answerPtr = kernel->machine->ReadRegister(5); // getting the answer from register 5 as an int
int id_buff = kernel->machine->ReadRegister(6); // getting the buffer id form register 6
string sender = kernel->currentThread->getName();	//string conversion
Buffer *msg_buff = fetch_buff(id_buff, sender); // fetching the buffer by passing sender information
if(msg_buff == NULL)
{
	kernel->currentThread->Yield(); // stopping incase of NULL buffer
}
else if(msg_buff->getMsg()->IsEmpty())
{
	kernel->currentThread->Yield(); // stopping incase of message being empty
}

sender = "";
if (kernel->currentThread->pMQueue[id_buff] != NULL)
	sender = kernel->currentThread->pMQueue[id_buff]->getSender()->getName(); // getting the sender name
else
{
	sender = kernel->msg_buff_Q[id_buff]->Front()->getReceiver()->getName(); //sender is message buffer's front
}


if (Thread::getTByName(sender) != NULL) // if sender is not NULL
{
	msg_buff = fetch_buff(id_buff, sender);
	if (msg_buff != NULL && !msg_buff->getMsg()->IsEmpty()) // if the message isn't empty
	{
		string msg = msg_buff->getMsg()->RemoveFront();
		kernel->machine->WriteRegister(2, kernel->currentThread->getThreadId());
						std::cout << "\nAnswer Received " <<"from process " << sender<<" by process " << kernel->currentThread->getName() << "\nMessage:" << msg << std::endl << std::endl;
		// normal execution of the wait answer syscall. The details are being printed
	}
	else
	{
		kernel->currentThread->Yield();
	}
}
else
{
	Buffer *msg_buff = fetch_buff(id_buff, sender);
	if (msg_buff != NULL && !msg_buff->getMsg()->IsEmpty()) // incase the message isn't empty, execute this if loop
	{
		string msg = msg_buff->getMsg()->RemoveFront();
		kernel->machine->WriteRegister(2, kernel->currentThread->getThreadId());
		std::cout << "\nAnswer Received " <<"from process " << sender<<" by process " << kernel->currentThread->getName() << "\nMessage:" << msg << std::endl << std::endl;
		//printing out the answer received
	}
	else
	{
		cleanBuffer(id_buff); // clear the buffer
		setStringPTR(DUMMY_MESSAGE, 0);
		kernel->machine->WriteRegister(2, 0);
		std::cout << "Dummy Response "<< std::endl;
		//abnormal execution. Sending dummy response
	}
	break;
}

/* set previous programm counter (debugging only)*/
kernel->machine->WriteRegister(PrevPCReg, kernel->machine->ReadRegister(PCReg));

/* set programm counter to next instruction (all Instructions are 4 byte wide)*/
kernel->machine->WriteRegister(PCReg, kernel->machine->ReadRegister(PCReg) + 4);

/* set next programm counter for brach execution */
kernel->machine->WriteRegister(NextPCReg, kernel->machine->ReadRegister(PCReg) + 4);
(void)kernel->interrupt->SetLevel(oldLevel);
return;
break;
}

default:
cerr << "Unexpected system call " << type << "\n";

break;
}

break;
default:
cerr << "Unexpected user mode exception" << (int)which << "\n";

break;
}
ASSERTNOTREACHED();
}
