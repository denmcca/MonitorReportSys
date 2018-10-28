//static int globalReceiverId = 0; // receiver 1 and 2 run as separate programs.
#include "MsgPigeon.cpp"
#include <string>

using namespace std;

struct ErrorCode
{
	int errorCode = 0;
	int auxCode = 0;
	
	ErrorCode(int errorCodeIn, long auxCodeIn) { errorCode = errorCodeIn; auxCode = auxCodeIn; };
	ErrorCode(int errorCodeIn) { errorCode = errorCodeIn; auxCode = 0; };
};

class Receiver
{
	public:
	const char* ftok_path = ".";
	const int ftok_id = 'u';

	// buffer objects
	MsgPigeon msgr;
	bool getFrom25x, getFrom997;	
	// Message count
	int msgCount = 0;	
	// Queue ID
	int qid = 0;
	// Receiver ID
	int id = 0;
	// Receiver 2 message count max
	const int MSG_COUNT_MAX_R2 = 5000;
	// Predefined functional queue messages
	// Indicates that program is terminating
	const string MSG_TERM = "Terminating";
	// Sent to sender program that indicates when receiver has received message.
	const string MSG_ACK = "Acknowledgement";
	// Sent to sender for polling
	const string MSG_ALIVE = "Alive";
	// mTypes events
	const long MTYPE_251 = 251;
	const long MTYPE_257 = 257;
	const long MTYPE_997 = 997;
	// ACK also for termination
	const long MTYPE_997_ACK = 998;
	// Polls
	const long MTYPE_POLL_257 = 256;
	// Queue cleaning: Used to pop all unprocessed messages from queue before deallocation
	const long MTYPE_QUEUE_CLEAN = 0;
	// Private functions
	void initializeQueue();
	void assignReceiverNumber();	// displays prompt for user to select from ids.
	int promptReceiverNumber();	// prompts user to enter choice of ids.
	void sendMessage(const long&, const string&);	// send message to queue
	void terminateQueue();	// clean up values from queue.
	bool isQueueEmpty();	// checks if message queue is empty
	bool isMessageTerminating();	// checks if message received is message to terminate
	void disconnectSender();	// stops receiver from sending and getting messages from sender, returns sender disconnected
	void cleanUpQueue();	// removes messages with mtypes for which receiver is responsible.
	bool isMessageCountMax();
	void getMessage(const long&);	// get message from queue
	void printMessage();	// print message from buffer		
	bool processMessage();
	int getQueueMessageCount();	
	void terminateSelf();	
	void sendAcknowledgement();
	void doTerminateSelf();	
	void doQueueDeallocation();
	void notifyStart();
	void waitForSenders();

	public:			
	// constructor
	Receiver();
	// Public functions
	int getQID();
	void startReceiver();	
};
