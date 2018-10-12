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
	bool sendTo25x, sendTo997;
	int qid = 0;	// id of queue
	int id = 0;		// id of receiver
	int msgCount = 0;
	static const int idListSize = 2;	// size of list that holds possible receiver ids.
	int idList[idListSize] = {1, 2};	// list that holds values of possible receiver ids.
	
	const int MSG_COUNT_MAX_R2 = 5000;	// 5000 for final	

	const string MSG_TERM = "Terminating";	// Use when sending terminating messages or checking for term messages.
	const string MSG_ACK = "Acknowledgement";
	const string MSG_ALIVE = "I am alive";	// Use when sending acknowledgement messages.
	
	// mTypes
	// Events
	const long MTYPE_EVENT_251 = 251;
	const long MTYPE_EVENT_257 = 257;
	const long MTYPE_EVENT_997 = 997;
	const long MTYPE_EVENT_997_R2 = 1097;
	// ACKS
	const long MTYPE_ACK_997 = 998;
	const long MTYPE_ACK_997_R2 = 1098;
	// Polls
	const long MTYPE_POLL_257 = 256;
	const long MTYPE_POLL_997 = 996;
	// Handshaking
	const long MTYPE_SHAKE_251 = 253;
	const long MTYPE_SHAKE_251_ACK = 254;
	const long MTYPE_SHAKE_257 = 259;
	const long MTYPE_SHAKE_257_ACK = 260;
	const long MTYPE_SHAKE_997 = 999;
	const long MTYPE_SHAKE_997_ACK = 1000;
	const long MTYPE_SHAKE_997_R2 = 1099;
	const long MTYPE_SHAKE_997_R2_ACK = 1100;
	const long MTYPE_R2_READY = 4;
	// Shutdown
	const long MTYPE_SHUTDOWN_251 = 255;
	const long MTYPE_SHUTDOWN_257 = 261;
	const long MTYPE_SHUTDOWN_997 = 1001;
	const long MTYPE_SHUTDOWN_R2 = 6;
	// Queue cleaning
	const long MTYPE_QUEUE_CLEAN = -9999;
	

	// functions
	void initializeQueue();
	void assignReceiverNumber();	// displays prompt for user to select from ids.
	int getReceiverNumber();	// prompts user to enter choice of ids.
	void getMessage(const long&);	// get message from queue
	void setMessage(const string&);	// sets message
	void processMessage();
	bool isThread25x();
	void sendMessage(const long&);	// send message to queue
	void terminateQueue();	// clean up values from queue.
	void terminateSelf();
	bool isQueueEmpty();	// checks if message queue is empty
	bool isMessageTerminate();	// checks if message received is message to terminate
	void disconnectSender();	// stops receiver from sending and getting messages from sender
	void cleanUpQueue();	// removes messages with mtypes for which receiver is responsible.
	void printMsg();	// print message from buffer	
	void printQueueNotFoundError();	// prints error when queue not found
	void senderTerminationNotification(const long&);
	int getQueueMessageLimit();
	int getQueueMessageCount();
	bool isMessageQueueFull();
	bool isMessageCountMax();
		
	public:	
	// buffer objects
	MsgPigeon msgr25x;	// sends and gets values from queue
	MsgPigeon msgr997;
	
	// fields
	pthread_t thread_id = 0; // holds thread id of first thread 25x 
	
	// constructor
	Receiver();
	
	// functions
	void waitForSenders();	
	void sendStartMessages();	
	void process997();
	void process25x();
	void sendAcknowledgement();
	void doTerminateSelf();	
	bool isSendTo25x();
	bool isSendTo997();
	void doQueueDeallocation();
	void postCompletion();
	void waitForShutdownConfirmations();
};
