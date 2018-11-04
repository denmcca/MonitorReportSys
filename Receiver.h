#include "MsgPigeon.cpp"
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/types.h>
#include <unistd.h>

/**
	Struct object used to throw error codes from Receiver class to an awaiting
	catch block.
	errorCode is set depending on where the throw has occurred and the auxCode
	is any value of interest from within the function.
*/
struct ErrorCode
{
	int errorCode = 0;
	int auxCode = 0;

	// ErrorCode constructors
	ErrorCode(int errorCodeIn, long auxCodeIn) { errorCode = errorCodeIn; auxCode = auxCodeIn; };
	ErrorCode(int errorCodeIn) { errorCode = errorCodeIn; auxCode = 0; };
};

/**
	Class which is designed to receives messages from a message queue.
*/
class Receiver
{
private:
	// Message queue initialization values.
	const char* ftok_path = ".";
	const int ftok_id = 'u';

	// Source ID
	const int REC1 = 1; // Message from Receiver 1.
	const int REC2 = 2; // Message from Receiver 2.
	const int S251 = 251; // Message from Sender 251.
	const int S257 = 257; // Message from Sender 257.
	const int S997 = 997; // Message from Sender 997.

	// mTypes
	const int REC1_GET = -2; // Gets messages for receiver 1.
	const int REC1_ACK = 3; // ACKs sent from receiver 1.
	const int REC2_GET = 20; // Gets message for receiver 2.
	const int REC2_ACK = 30; // ACKs sent from receiver 2.
	const int REC2_TERM = 12; // Message sent from R2 to R1 to notify termination.
	const int S257_POLL = 256; // Sending terminating message using 257 polling
	const int QUEUE_CLEAN = 0; // Used to clear out message queue

	// Messages
	const char* MSG_TERM = "Terminating"; // Message indicating termination.
	const char* MSG_ACK = "Acknowledgement"; // Message sent to Sender 997.

	// Buffer objects
	MsgPigeon msgr;

	// Flags
	bool getFrom25x, getFrom997; // If true, get message from Sender. Else, do not.
	bool isAccepting; // If true, completely processes received messages. Else, do not process.

	// Attributes
	const int MSG_COUNT_MAX_R2 = 5000; // Receiver 2's max message to receive.
	int msgCount;
	int msgSize = 0; // Size of buf object / message object
	int qid = 0; // Message queue ID
	int id = 0; // Receiver ID

	// Private functions
	void initializeQueue(); // initializes message queue.
	void assignReceiverNumber();	// displays prompt for user to select from ids.
	int promptReceiverNumber();	// prompts user to enter choice of ids.
	void getMessage(const long&);	// get message from message queue.
	void sendMessage(const long&, const char*);	// send message to queue.
	void sendAcknowledgement(); // passes ack message and mtype to sendMessage.
	void printMessage();	// print message from received from message queue.
	bool processMessage(); // decides what to do with message from queue.
	bool isMessageCountMax(); // checks if message count has reached maximum value for receiver 2.
	void terminateSelf();	// function which sends messages to active senders indicating that receiver is terminating.
	void doTerminateSelf(); // receiver 2's termination check.
	void doQueueDeallocation(); // removes message queue.
	bool isQueueEmpty();	// checks if message queue is empty.
	bool isMessageTerminating();	// checks if message received is message to terminate.
	void disconnectSender();	// stops receiver from sending and getting messages from sender, returns sender disconnected.
	void cleanUpQueue();	// removes messages with mtypes for which receiver is responsible.
	void notifyStart(); // Receiver 2 notifies Receiver 1. Receiver 1 notifies Senders.
	void waitForSenders(); // Receiver 2 waits for Senders. Receiver 1 waits for Receiver 2.
	void terminateQueue();	// clean up values from queue.

public:
	// constructor
	Receiver();
	// Public functions
	void initialize(); // calls functions to select receiver number and initialize or connect to message queue.
	int getID(); // returns receiver id.
	int getQID(); // returns message queue id.
	void startReceiver(); // starts main program loop.
	int messageQueueCount(); // returns number of messages inside message queue.
	void printError(ErrorCode); // prints error messages.
};
