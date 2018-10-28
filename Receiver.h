#define REC1 1
#define REC1_GET -10
#define REC1_ACK 7
#define REC2 2
#define REC2_GET 20 // Cannot use negative for R2 since R1 is using negative value
#define REC2_ACK 17
#define REC2_TERM 12
#define MSG_COUNT_MAX_R2 5000
#define S251 251
#define S257 257
#define S997 997
#define S257_POLL 256
#define QUEUE_CLEAN 0

#include "MsgPigeon.cpp"
#include <string>
#include <iostream>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/types.h>
#include <unistd.h>
#include <cstring>
#include <sys/wait.h>
#include <stdio.h>

struct ErrorCode
{
	int errorCode = 0;
	int auxCode = 0;
	
	ErrorCode(int errorCodeIn, long auxCodeIn) { errorCode = errorCodeIn; auxCode = auxCodeIn; };
	ErrorCode(int errorCodeIn) { errorCode = errorCodeIn; auxCode = 0; };
};

class Receiver
{
	private:
	const char* ftok_path = ".";
	const int ftok_id = 'u';
	// Indicates that program is terminating
	const std::string MSG_TERM = "Terminating";
	// Sent to sender program that indicates when receiver has received message.
	const std::string MSG_ACK = "Acknowledgement";
	// Sent to sender for polling
	const std::string MSG_ALIVE = "Alive";	

	// buffer objects
	MsgPigeon msgr;
	bool getFrom25x, getFrom997;	
	// Message count
	int msgCount = 0;	
	// Queue ID
	int qid = 0;
	// Receiver ID
	int id = 0;
	// Receiver can still receiver but will not print messages. Keep buffer clear.
	bool isPrinting;

	// Private functions
	void initializeQueue();
	void assignReceiverNumber();	// displays prompt for user to select from ids.
	int promptReceiverNumber();	// prompts user to enter choice of ids.
	void sendMessage(const long&, const std::string&);	// send message to queue
	void terminateQueue();	// clean up values from queue.
	bool isQueueEmpty();	// checks if message queue is empty
	bool isMessageTerminating();	// checks if message received is message to terminate
	void disconnectSender();	// stops receiver from sending and getting messages from sender, returns sender disconnected
	void cleanUpQueue();	// removes messages with mtypes for which receiver is responsible.
	bool isMessageCountMax();
	void getMessage(const long&);	// get message from queue
	void printMessage();	// print message from buffer		
	bool processMessage();
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
	void initialize();
	int getQID();
	void startReceiver();
	int messageQueueCount();
	static void printError(ErrorCode, int, Receiver&);	
};
