#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/types.h>
#include <iostream>
#include <cstring>
	
struct InnerMsg
{
	long srcID;
	char message[50];
};

struct MsgPigeon
{
	long mType; // required for queue
	InnerMsg message;
	
	int getSize() { return sizeof(MsgPigeon) - sizeof(long); }
	int getMessageQueueLimit(int &qidIn)
	{
		struct msqid_ds buf_nfo;
		msgctl(qidIn, IPC_STAT, &buf_nfo); // buf gets queue data including number of messages
		return buf_nfo.msg_qbytes / getSize();
	}
	int getMessageQueueCount(int &qidIn)
	{
		struct msqid_ds buf_nfo;	
		msgctl(qidIn, IPC_STAT, &buf_nfo); // buf gets queue data including number of messages
		return buf_nfo.msg_qnum;
	}
	bool isMessageQueueFull(int &qidIn)
	{
		return getMessageQueueLimit(qidIn) == getMessageQueueCount(qidIn);	
	}
	
		
};

/*//
int main()
{

	MsgPigeon pigTest;
	
	InnerMsg innerTest;
	
	innerTest.srcID = 99;
	strcpy(innerTest.message, "testing inside message");
	
	pigTest.mType = 22;
	pigTest.message = innerTest;
	

	std::cout << pigTest.mType << std::endl;
	std::cout << pigTest.message.srcID << std::endl;
	std::cout << pigTest.message.message << std::endl;
	
	std::cout << "end test... ";
	std::cin.get();
};
//*/
