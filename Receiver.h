//static int globalReceiverId = 0; // receiver 1 and 2 run as separate programs.
#include "MsgPigeon.cpp"
#include <string>

using namespace std;

class Receiver
{
	public:
	// fields
	static const int idListSize = 2;	// size of list that holds possible receiver ids.
	int idList[idListSize] = {1, 2};	// list that holds values of possible receiver ids.
	int qid;	// id of queue
	int id;		// id of receiver
	MsgPigeon msgr;	// sends and gets values from queue
	
	string MSG_TERM;
	string MSG_ACK;
	int MSG_COUNT_MAX_R2;	
	int senderBit;
	
	// constructor
	Receiver(int);	// constructor with parameter for queue id
	
	// functions
	int assignReceiverNumber();	// displays prompt for user to select from ids.
	int getReceiverNumber();	// prompts user to enter choice of ids.
	bool getMessage(long);	// get message from queue
	void setMessage(string);	// sets message
	void setMessageType(long);	// sets mType
	bool sendMessage(long);	// send message to queue
	void terminateQueue();	// clean up values from queue.
	void terminateSelf();
	bool isQueueEmpty();	// checks if message queue is empty
	bool isTerminate();	// checks if message received is message to terminate
	void cleanUpQueue();	// removes messages with mtypes for which receiver is responsible.
	void printMsg();	// print message from buffer	
	void printQueueNotFoundError();	// prints error when queue not found
	bool sendAcknowledgement();
};
