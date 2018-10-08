#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/types.h>

struct MsgPigeon
{
	long mType; // required for queue
	char message[50];
	
	int getSize() { return sizeof(MsgPigeon) - sizeof(long); }
	int getQueueMessageLimit(int& qidIn)
	{
		struct msqid_ds buf_nfo;
		msgctl(qidIn, IPC_STAT, &buf_nfo); // buf gets queue data including number of messages
		return buf_nfo.msg_qbytes / getSize();
	}
	int getQueueMessageCount(int& qidIn)
	{
		struct msqid_ds buf_nfo;	
		msgctl(qidIn, IPC_STAT, &buf_nfo); // buf gets queue data including number of messages
		return buf_nfo.msg_qnum;
	}
	bool isMessageQueueFull(int& qidIn)
	{
		return getQueueMessageLimit(qidIn) == getQueueMessageCount(qidIn);	
	}
	
};


