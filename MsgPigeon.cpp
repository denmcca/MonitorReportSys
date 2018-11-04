#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/types.h>

/**
	Struct used to create object to pass through message queue. Contains message
	type value which is used to identify which process is to receive it. Also
	contains a struct object which holds message content and ID of sending process.
*/
struct MsgPigeon
{
	long mType; // required for queue
	struct InnerMsg { int srcID; char message[20]; } message;

	/**
		Gets the size of this structure.
	*/
	int getSize() { return sizeof(MsgPigeon) - sizeof(long); }

	/**
		Gets message capacity of message queue.
		@return The size as an integer value.
	*/
	int getMessageQueueLimit(int &qidIn)
	{
		struct msqid_ds buf_nfo;
		msgctl(qidIn, IPC_STAT, &buf_nfo); // buf gets queue data including number of messages
		return buf_nfo.msg_qbytes / getSize();
	}

	/**
		Gets the current number of messages inside message queue.
		@return Integer value of message count inside message queue.
	*/
	int getMessageQueueCount(int &qidIn)
	{
		struct msqid_ds buf_nfo;
		msgctl(qidIn, IPC_STAT, &buf_nfo); // buf gets queue data including number of messages
		return buf_nfo.msg_qnum;
	}

	/**
		Checks if message queue is full.
		@param qidIn Integer value of message queue ID used to query message queue.
		@return True if message queue limit is equal to message queue count. If not
		False.
	*/
	bool isMessageQueueFull(int &qidIn)
	{
		return getMessageQueueLimit(qidIn) == getMessageQueueCount(qidIn);
	}
};
