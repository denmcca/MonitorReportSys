//static int globalReceiverId = 0; // receiver 1 and 2 run as separate programs.
#include "MsgPigeon.cpp"
#include <string>

using namespace std;

class Receiver
{
	private:
	const char* ftok_path = ".";
	const int ftok_id = 'u';
	
	void getQID();
	void assignReceiverNumber();	// displays prompt for user to select from ids.
	int getReceiverNumber();	// prompts user to enter choice of ids.
	
	public:
	// fields
	const string MSG_TERM = "Terminating";	// Use when sending terminating messages or checking for term messages.
	const string MSG_ACK = "Acknowledgement";
	const string MSG_ALIVE = "I am alive";	// Use when sending acknowledgement messages.
	const int MSG_COUNT_MAX_R2 = 100;	// 5000 for final
	const long MTYPE_EVENT_251 = 251;
	const long MTYPE_EVENT_257 = 257;
	const long MTYPE_EVENT_997 = 997;
	const long MTYPE_EVENT_997_R2 = 1097;
	const long MTYPE_ACK_997 = 998;
	const long MTYPE_ACK_997_R2 = 1098;
	const long MTYPE_POLL_251 = 250;	
	const long MTYPE_POLL_257 = 256;
	const long MTYPE_POLL_997 = 996;
	
	static const int idListSize = 2;	// size of list that holds possible receiver ids.
	int idList[idListSize] = {1, 2};	// list that holds values of possible receiver ids.
	int qid = 0;	// id of queue
	int id = 0;		// id of receiver
	int senderBit = 11;
	long current_mType = 0;
	int msgCount = 0;
	
	// buffer object
	MsgPigeon msgr;	// sends and gets values from queue
	
	// constructor
	Receiver();	// constructor with parameter for queue id
	
	// functions
	
	bool getMessage(const long&);	// get message from queue
	void setMessage(const string&);	// sets message
	void setMessageType(const long&);	// sets mType
	bool sendMessage(const long&);	// send message to queue
	void terminateQueue();	// clean up values from queue.
	void terminateSelf();
	bool isQueueEmpty();	// checks if message queue is empty
	bool isTerminate();	// checks if message received is message to terminate
	void cleanUpQueue();	// removes messages with mtypes for which receiver is responsible.
	void printMsg();	// print message from buffer	
	void printQueueNotFoundError();	// prints error when queue not found
	bool sendAcknowledgement();
	void senderTerminationNotification(const long&);
};
