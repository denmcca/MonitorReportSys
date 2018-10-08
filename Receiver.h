//static int globalReceiverId = 0; // receiver 1 and 2 run as separate programs.
#include "MsgPigeon.cpp"
#include <string>
#include <pthread.h>

using namespace std;

class Receiver
{
	private:
	const char* ftok_path = ".";
	const int ftok_id = 'u';
	
	void initializeQueue();
	void assignReceiverNumber();	// displays prompt for user to select from ids.
	int getReceiverNumber();	// prompts user to enter choice of ids.
	
	public:
	// fields
	const string MSG_TERM = "Terminating";	// Use when sending terminating messages or checking for term messages.
	const string MSG_ACK = "Acknowledgement";
	const string MSG_ALIVE = "I am alive";	// Use when sending acknowledgement messages.
	const int MSG_COUNT_MAX_R2 = 5000;	// 5000 for final
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
	//long current_mType = 0;
	int msgCount = 0;
	pthread_t thread_id = 0; // holds thread id of first thread 25x 
	
	// buffer object
	MsgPigeon msgr25x;	// sends and gets values from queue
	MsgPigeon msgr997;
	//MsgPigeon* msgr;
	
	// constructor
	Receiver();	// constructor with parameter for queue id
	
	// functions
	
	void getMessage(const long&);	// get message from queue
	void setMessage(const string&);	// sets message
	bool isThread25x();
	void sendMessage(const long&);	// send message to queue
	void terminateQueue();	// clean up values from queue.
	void terminateSelf();
	bool isQueueEmpty();	// checks if message queue is empty
	bool isMessageTerminate();	// checks if message received is message to terminate
	void cleanUpQueue();	// removes messages with mtypes for which receiver is responsible.
	void printMsg();	// print message from buffer	
	void printQueueNotFoundError();	// prints error when queue not found
	void sendAcknowledgement();
	void senderTerminationNotification(const long&);
	void waitForSenders();
	void sendStartMessages();
	int getQueueMessageLimit();
	int getQueueMessageCount();
	bool isMessageQueueFull();
	void doTerminateSelf();
	//void getPoll();
	//void sendPoll();
	void postCompletion();
	void doQueueDeallocation();
	bool isMessageCountMax();
	void process997();
	void process25x();
	void processMessage();
	void disconnectSender();
};
