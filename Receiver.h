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

<<<<<<< HEAD
	// ErrorCode constructors
=======
>>>>>>> aee4432ff4590b2afd06e8357541868a230bbc42
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
<<<<<<< HEAD
=======
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

>>>>>>> aee4432ff4590b2afd06e8357541868a230bbc42

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
<<<<<<< HEAD

	// Flags
	bool getFrom25x, getFrom997; // If true, get message from Sender. Else, do not.
	bool isPrinting; // If true, prints received messages. Else, do not print.

	// Attributes
	const int MSG_COUNT_MAX_R2 = 5000000; // Receiver 2's max message to receive.
	int msgCount;
	int msgSize = 0; // Size of buf object / message object
	int qid = 0; // Message queue ID
	int id = 0; // Receiver ID
=======
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
>>>>>>> aee4432ff4590b2afd06e8357541868a230bbc42

	// Private functions
	void initializeQueue(); // initializes message queue.
	void assignReceiverNumber();	// displays prompt for user to select from ids.
	int promptReceiverNumber();	// prompts user to enter choice of ids.
<<<<<<< HEAD
	void sendMessage(const long&, const char*);	// send message to queue.
=======
	void sendMessage(const long&, const char*);	// send message to queue
>>>>>>> aee4432ff4590b2afd06e8357541868a230bbc42
	void terminateQueue();	// clean up values from queue.
	bool isQueueEmpty();	// checks if message queue is empty.
	bool isMessageTerminating();	// checks if message received is message to terminate.
	void disconnectSender();	// stops receiver from sending and getting messages from sender, returns sender disconnected.
	void cleanUpQueue();	// removes messages with mtypes for which receiver is responsible.
<<<<<<< HEAD
	bool isMessageCountMax(); // checks if message count has reached maximum value for receiver 2.
	void getMessage(const long&);	// get message from message queue.
	void printMessage();	// print message from received from message queue.
	bool processMessage(); // decides what to do with message from queue.
	void terminateSelf();	// function which sends messages to active senders indicating that receiver is terminating.
	void sendAcknowledgement(); // passes ack message and mtype to sendMessage.
	void static sendAcknowledgementThreaded(Receiver* r); // used to create thread specifically for threading acks (fast).
	void sendAcknowledgementForked(); // alternative method to branch acknowledgements prevent lock ups (slow).
	void doTerminateSelf(); // receiver 2's termination check.
	void doQueueDeallocation(); // removes message queue.
	void notifyStart(); // Receiver 2 notifies Receiver 1. Receiver 1 notifies Senders.
	void waitForSenders(); // Receiver 2 waits for Senders. Receiver 1 waits for Receiver 2.

public:
	// constructor
	Receiver();
	// Public functions
	void initialize(); // calls functions to select receiver number and initialize or connect to message queue.
	int getID(); // returns receiver id.
	int getQID(); // returns message queue id.
	void startReceiver(); // starts main program loop.
	int messageQueueCount(); // returns number of messages inside message queue.
	static void printError(ErrorCode, int, Receiver&); // prints error messages.
=======
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
>>>>>>> aee4432ff4590b2afd06e8357541868a230bbc42
};
