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
#include <thread>

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
	const char* MSG_TERM = "Terminating";
	// Sent to sender program that indicates when receiver has received message.
	const char* MSG_ACK = "Acknowledgement";
	// Sent to sender for polling
	const char* MSG_ALIVE = "Alive";

	// mTypes
	const int REC1 = 1;
	const int REC1_GET = -10;
	const int REC1_ACK = 11;
	const int REC2 = 2;
	const int REC2_GET = 20;
	const int REC2_ACK = 21;
	const int REC2_TERM = 12;
	const int MSG_COUNT_MAX_R2 = 5000;
	const int S251 = 251;
	const int S257 = 257;
	const int S997 = 997;
	const int S257_POLL = 256;
	const int QUEUE_CLEAN = 0;


	// buffer objects
	MsgPigeon msgr;
	bool getFrom25x, getFrom997;
	// Message count
	int msgCount = 0;
	int msgSize = 0;
	std::thread t_ack;
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
	void sendMessage(const long&, const char*);	// send message to queue
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
	void static sendAcknowledgementThreaded(Receiver* r);
	void sendAcknowledgementForked();
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
