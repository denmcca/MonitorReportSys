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
	// Queue ID
	int qid = 0;
	// Receiver ID
	int id = 0;
	// Message count
	int msgCount = 0;
	// Receiver 2 message count max
	const int MSG_COUNT_MAX_R2 = 5000;	
	
	// Predefined functional queue messages
	// Indicates that program is terminating
	const string MSG_TERM = "Terminating";
	// Sent to sender program that indicates when receiver has received message.
	const string MSG_ACK = "Acknowledgement";
	// Sent to sender for polling
	const string MSG_ALIVE = "Alive";
	
	// mTypes
	// Events
	const long MTYPE_EVENT_251 = 251;
	const long MTYPE_EVENT_257 = 257;
	const long MTYPE_EVENT_997 = 997;
	// Event message from Sender 997 to Receiver 2
	const long MTYPE_EVENT_997_R2 = 1097;
	// ACKS
	const long MTYPE_ACK_997 = 998;
	// Ack message from Receiver 2 to Sender 997
	const long MTYPE_ACK_997_R2 = 1098;
	// Polls
	const long MTYPE_POLL_257 = 256;
	//const long MTYPE_POLL_997 = 996;
	// Handshake
	const long MTYPE_SHAKE_251 = 253;
	const long MTYPE_SHAKE_257 = 259;	
	const long MTYPE_SHAKE_997 = 999;
	const long MTYPE_SHAKE_997_R2 = 1099;
	const long MTYPE_SHAKE_251_ACK = 254;
	const long MTYPE_SHAKE_257_ACK = 260;
	const long MTYPE_SHAKE_997_ACK = 1000;	
	const long MTYPE_SHAKE_997_R2_ACK = 1100;
	const long MTYPE_R2_READY = 4;
	// Shutdown: Messages received by Receiver 1 before initiating shutdown process
	const long MTYPE_SHUTDOWN_257 = 261;
	const long MTYPE_SHUTDOWN_997 = 1001;
	const long MTYPE_SHUTDOWN_R2 = 6;
	// Queue cleaning: Used to pop all unprocessed messages from queue before deallocation
	const long MTYPE_QUEUE_CLEAN = -9999;
	
	// functions
	void initializeQueue();
	void assignReceiverNumber();	// displays prompt for user to select from ids.
	int getReceiverNumber();	// prompts user to enter choice of ids.
	
	void processMessage();
	bool isThread25x();
	void sendMessage(const long&);	// send message to queue
	void terminateQueue();	// clean up values from queue.
	void terminateSelf();
	bool isQueueEmpty();	// checks if message queue is empty
	bool isMessageTerminate();	// checks if message received is message to terminate
	void disconnectSender();	// stops receiver from sending and getting messages from sender
	void cleanUpQueue();	// removes messages with mtypes for which receiver is responsible.
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
	
		void getMessage(const long&);	// get message from queue
		void setMessage(const string&);	// sets message
		void printMsg();	// print message from buffer		
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
	int getMessageCount();
};
