//static int globalReceiverId = 0; // receiver 1 and 2 run as separate programs.
#include "MsgPigeon.cpp"
#include <string>

using namespace std;

class Receiver
{
	public:
	// fields
	const string MSG_TERM = "Terminating";	// Use when sending terminating messages or checking for term messages.
	const string MSG_ACK = "Acknowledgement";	// Use when sending acknowledgement messages.
	const int MSG_COUNT_MAX_R2 = 10;	// 5000 for final
	const long MTYPE_EVENT_251 = 251;
	const long MTYPE_EVENT_257 = 257;
	const long MTYPE_EVENT_997 = 997;
	const long MTYPE_EVENT_997_R2 = 1097;
	const long MTYPE_ACK_997 = 998;
	const long MTYPE_ACK_997_R2 = 1098;
	const long MTYPE_POLL_257 = 256;
	const long MTYPE_POLL_997 = 998;
	
	static const int idListSize = 2;	// size of list that holds possible receiver ids.
	int idList[idListSize] = {1, 2};	// list that holds values of possible receiver ids.
	int qid;	// id of queue
	int id;		// id of receiver
	int senderBit;
	
	// buffer object
	MsgPigeon msgr;	// sends and gets values from queue
	
	// constructor
	Receiver(int);	// constructor with parameter for queue id
	
	// functions
	int assignReceiverNumber();	// displays prompt for user to select from ids.
	int getReceiverNumber();	// prompts user to enter choice of ids.
	bool getMessage(long*);	// get message from queue
	void setMessage(string*);	// sets message
	void setMessageType(long*);	// sets mType
	bool sendMessage(long*);	// send message to queue
	void terminateQueue();	// clean up values from queue.
	void terminateSelf();
	bool isQueueEmpty();	// checks if message queue is empty
	bool isTerminate();	// checks if message received is message to terminate
	void cleanUpQueue();	// removes messages with mtypes for which receiver is responsible.
	void printMsg();	// print message from buffer	
	void printQueueNotFoundError();	// prints error when queue not found
	bool sendAcknowledgement();
};
