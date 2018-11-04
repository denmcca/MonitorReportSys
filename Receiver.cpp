#include "Receiver.h"
#include <thread>

// debugging: keeps track of highest number of messages inside queue.
int MAX_QUEUE_COUNT = 0;

/**
	Constructor for Receiver class.
*/
Receiver::Receiver()
{
	// keeping track of senders. 1x represents 997, x1 represents 251 or 257
	getFrom25x = true;
	getFrom997 = true;
	isAccepting = true;
	msgCount = 0;
	msgSize = msgr.getSize();
}

/**
	Function which calls functions to assign id and allocate or connect to message
	queue.
*/
void Receiver::initialize()
{
	assignReceiverNumber();
	initializeQueue();
}

/**
	Function which allocates queue or connects to initialized message queue.
*/
void Receiver::initializeQueue()
{
	if (id == 1) qid = msgget(ftok(ftok_path, ftok_id), IPC_CREAT | 0600);
	else if (id == 2)
	{
		printf("Waiting for Receiver 1 to allocate message queue (Control-C to abort)");
		while (true)
		{
			qid = msgget(ftok(ftok_path, ftok_id), 0);
			if (qid != -1) break;

			std::cout << '.' << std::flush; sleep(1); // Animation
		}
		printf("\nQueue %d found!\n", qid);
	}
	else	throw ErrorCode(3, id); // Unexpected receiver ID
}

/**
	Prompts user with receiver choices then calls function to get input.
*/
void Receiver::assignReceiverNumber()
{
	printf("Which receiver do you want to activate?\n");

	// Printing choices
	for (int i = 0; i < 2; i++) printf("%d: Receiver %d\n", i + 1, i + 1);

	// Gets choice
	id = promptReceiverNumber();
}

/**
	Prompts user for receiver selection input.
	@return integer which represents user's choice.
*/
int Receiver::promptReceiverNumber() // gets user's choice
{
	char choice;

	while(true)
	{
		printf("Enter selection: ");
		std::cin >> choice;
		if (choice - '0' < REC1 | choice - '0' > REC2)
		{
			printf("%c is an invalid selection!\n", choice);
			if (!std::cin.good())
			{
				std::cin.clear();
				std::cin.ignore(INT8_MAX, '\n');
			}
		}
		else return choice - '0';
	}
}

/**
	Gets receiver's ID number.
	@return integer value that indicates receiver number.
*/
inline int Receiver::getID() { return id; }

/**
	Receives message object from message queue.
	@param id Message type number that is used to retrieve specific message mTypes
	from message queue.
*/
inline void Receiver::getMessage(const long &mTypeIn)
{
	if (msgrcv(qid, (struct msgbuf *)&msgr, msgSize, mTypeIn, 0) == -1)
		throw ErrorCode(-11, mTypeIn);
}

/**
	Sends message object to message queue.
	@param mTypeIn Message type that sets the message's mtype value.
*/
inline void Receiver::sendMessage(const long &mTypeIn, const char *msgIn)
{
	msgr.message.srcID = id;
	msgr.mType = mTypeIn;
	strcpy(msgr.message.message, msgIn);

	if(msgsnd(qid, (struct msgbuf *)&msgr, msgSize, 0) == -1)
		throw ErrorCode(-10, (int)mTypeIn);
}

/**
	Creates a thread for acknowledgements to prevent system from locking up
	@param r Receiver pointer required to pass values to static function.
*/
inline void Receiver::sendAcknowledgementThreaded(Receiver* r)
{
	r->sendAcknowledgement();
}

/*
	Sets and sents a message object to sendMessage function to send an
	acknowledgement to Sender 997 after it sends an event.
*/
inline void Receiver::sendAcknowledgement()
{
	if (id == REC1)
	{
		sendMessage(REC1_ACK, MSG_ACK);
		return;
	}
	sendMessage(REC2_ACK, MSG_ACK);
}

/**
	Establishes syncronization between programs, sets main loop into action. After
	main loop is completed functions to deallocate message queue are called.
*/
void Receiver::startReceiver()
{
	if (id == REC1) printf("Message Queue Limit: %d messages\n",
		msgr.getMessageQueueLimit(qid));

	waitForSenders();
	notifyStart();

	while (getFrom25x | getFrom997)
	{
		if (id == REC1) getMessage(REC1_GET);
		else if (id == REC2) getMessage(REC2_GET);

		if (!processMessage()) continue;
		doTerminateSelf();
	}
	doQueueDeallocation();
}

/**
	Determines what to do with received message. If message is to terminate, Receiver
	will disassociate itself from sender. If message is an event, count will be
	incremented. If message is from Sender 997 then ACK will be sent.
	@return False if message was not an event, or if receiver is no long receiving
	events from sender. True if message is an event from a sender with which it is still
	assoicated.
*/
inline bool Receiver::processMessage()
{
	if (MAX_QUEUE_COUNT < msgr.getMessageQueueCount(qid))
		MAX_QUEUE_COUNT = msgr.getMessageQueueCount(qid);

	// if 997 or 251 is terminating
	if (isMessageTerminating())
	{
		disconnectSender();
		return true;
	}

	if (isAccepting)
	{
		if (msgr.message.srcID == S251 | msgr.message.srcID == S257)
		{
			if (!getFrom25x) return false; // junk message from Sender 251 or 257; continue
		}
		else if (msgr.message.srcID == S997)
		{
			if (!getFrom997) return false; // junk message from Sender 997; continue
			sendAcknowledgement();
		}
		else throw(ErrorCode(-7, msgr.mType));

		// Receiver 2 terminates at message count max
		msgCount++;
		return true;
	}
	return false;
}

/**
	Checks message received has terminating message.
	@return True if contains terminating message. False if does not.
*/
inline bool Receiver::isMessageTerminating()
{
	return strcmp(msgr.message.message, MSG_TERM) == 0;
}

/**
	Disassociates sender from receiver.
*/
void Receiver::disconnectSender()
{
	if (msgr.message.srcID == S251 | msgr.message.srcID == S257)
	{
		// Disconnects Sender 251 or 257
		getFrom25x = false;
	}
	else if (msgr.message.srcID == S997)
	{
		// Disconnects Sender 997
		getFrom997 = false;
	}
	 // disconnectSender received unexcepted srcID
	else throw ErrorCode(-9, msgr.message.srcID);
}

/**
	Prints message content from message object. No longer used to speed up
	processing speed.
*/
void Receiver::printMessage()
{
	printf("Sender %d sent: %s\n", (int)msgr.message.srcID, msgr.message.message);
}

/**
	Determines if receiver is Receiver 2, still accepting messages, and	has reached
	its maximum message count to call function which starts termination process.
*/
inline void Receiver::doTerminateSelf()
{
	if (id == REC2 && isAccepting)
	{
		if (isMessageCountMax())
		{
			terminateSelf();
		}
	}
}

/**
	Evalulates Receiver 2's message count with maximum message count.
	@return True if message count has been reached. False if count has not been
	reached.
*/
inline bool Receiver::isMessageCountMax()
{
	if (msgCount >= MSG_COUNT_MAX_R2) return true;
	return false;
}

/**
	Function called when Receiver 2 must terminate itself. If still receiving
	messages either sender, it will send terminating message through specified
	process according to each sender. Sets isAccepting attribute to false.
*/
void Receiver::terminateSelf()
{
	if (getFrom25x)
	{
		getMessage(S257_POLL);
		sendMessage(S257_POLL, MSG_TERM);
	}
	if (getFrom997)
	{
		sendMessage(REC2_ACK, MSG_TERM); // 17 and Terminating
	}

	isAccepting = false;
}

/**
	Checks if receiver is Receiver 1 then causes receiver to wait for termination
	message from Receiver 2 before calling function to remove message queue.
*/
void Receiver::doQueueDeallocation()
{
	if (id == REC1)
	{
		printf("Waiting for Receiver 2 to finish.\n");
		getMessage(REC2_TERM);
		terminateQueue();
	}
	else if (id == REC2)
	{
		sendMessage(REC2_TERM, "Receiver 2 Terminated!");
	}
}

/**
	Calls function to remove any messages left over in message queue. Confirms
	message queue is empty, then uses system call to deallocate message queue from
	memory.
*/
void Receiver::terminateQueue()
{
	cleanUpQueue();

	// if isQueueEmpty is 0 then queue is empty
	if (isQueueEmpty())
	{
		// if msgctl returns 0 then queue successfully deallocated
		if (msgctl(qid, IPC_RMID, NULL) == 0)
		{
			printf("Queue %d is now deallocated!\n", qid);
			return;
		}

		printf("Warning: unable to deallocate queue %d!\n", qid);
	}
}

/**
	Loops to pop any messages remaining in the message queue. Addition feature which
	keeps track of message types left in queue. If system works as expected, no messages
	should not have been left over.
*/
void Receiver::cleanUpQueue()
{
	int leftOverMessages = 0;
	int leftOver251 = 0;
	int leftOver257 = 0;
	int leftOver997 = 0;
	int leftOver997R2 = 0;

	while (!isQueueEmpty())
	{
		getMessage(QUEUE_CLEAN);

		printMessage();

		if (msgr.message.srcID == S251)
			leftOver251++;
		else if (msgr.message.srcID == S257)
			leftOver257++;
		else if (msgr.message.srcID == S997 && msgr.mType == REC1)
			leftOver997++;
		else if (msgr.message.srcID == S997 && msgr.mType == REC2)
			leftOver997R2++;

		++leftOverMessages;
	}

	printf("Messages removed before queue termination count: %d\n",
		leftOverMessages);
	printf("251: %d, 257: %d, 997_R1: %d, 997_R2: %d\n",
		leftOver251, leftOver257, leftOver997, leftOver997R2);
}

/**
	Checks if message queue is currently empty.
	@return True if called function returns zero. False if it does not return zero.
*/
bool Receiver::isQueueEmpty()
{
	return messageQueueCount() == 0;
}

/**
	Function used to syncronize sender and receiver program starts. If Receiver 1
	the receiver will wait for message from receiver 2. If Receiver 2 the receiver
	will wait for initialization messages from all senders.
*/
void Receiver::waitForSenders()
{
	if (id == REC1)
	{
		getMessage(REC1_GET);
	}
	if (id == REC2)
	{
		if (getFrom25x)
		{
			getMessage(REC2_GET);
			getMessage(REC2_GET);
		}
		if (getFrom997) getMessage(REC2_GET);
	}
}

/**
	Second function to syncronize start up. If Receiver 1 then it will send messages
	to all senders. If Receiver 2 the receiver will send notification to start to
	Receiver 1.
*/
void Receiver::notifyStart()
{
	if (id == REC1)
	{
		if (getFrom25x)
		{
			sendMessage(S251, "Sender 251 Start!");
			sendMessage(S257, "Sender 257 Start!");
		}
		if (getFrom997)
		{
			sendMessage(S997, "Sender 997 Start!");
		}
	}
	if (id == REC2)
	{
		sendMessage(REC1, "All Senders Ready!");
	}
}

/**
	Gets the number of message currently inside message queue.
	@return Integer value which represents the number of messages.
*/
int Receiver::messageQueueCount()
{
	return msgr.getMessageQueueCount(qid);
}

/**
	Prints error message for each ErrorCode object received.
	@param err ErrorCode that contains error information.
	@param QID Integer used to
*/
void Receiver::printError(ErrorCode err)
{
	if (err.errorCode == -9)
	{
		printf("Error (%d): disconnectSender method received unexpected srcID: %d\n",
			(int)err.errorCode, (int)err.auxCode);
	}

	else if (err.errorCode == 3)
	{
		printf("Error (%d): initializeQueue method received unexpected receiver ID: %d\n",
			(int)err.errorCode, (int)err.auxCode);
	}

	else if (err.errorCode == -8)
	{
		printf("Error (%d): Acknowledgement Fork failed: %d\n", (int)err.errorCode,
			(int)err.auxCode);
	}
	// Message Queue not found error
	else if (err.errorCode == -10)
	{
		printf("Error: sendMessage Queue Not Found. (%d)\n", (int)err.errorCode);
	}
	else if (err.errorCode == -11)
	{
		printf("Error (%d): getMessage Queue Not Found. (%d)\n", (int)err.errorCode,
			(int)err.auxCode);
	}
	else
	{
		printf("Error (%d): Something went wrong! (%d)\n", (int)err.errorCode,
			(int)err.auxCode);
	}

	printf("Failed with %d messages in message queue...\n", messageQueueCount());
	terminateQueue();
}

/**
	Gets message queue ID from message queue.
	@param Message queue ID as in integer value.
*/
int Receiver::getQID() { return qid; }

/**
	Prints exit message when called.
	@param id Receiver's identification number.
*/
void printExitMessage(int id)
{
	printf("Max Number of Messages inside Queue: %d\n", MAX_QUEUE_COUNT);
	// Notify user that Reciever program has completed
	printf("Receiver %d has completed... \n", id);
}

/**
	Program's main point of entry. Instantiates receiver object, initializes, and
	starts main function. Try-Catch block used to catch errors thrown by process.
*/
int main()
{
	int QID;
	Receiver r;

	try
	{
		// Instantiates Receiver object
		r.initialize();
		QID = r.getQID();
		r.startReceiver();
	}
	catch (ErrorCode err) { r.printError(err); }
	catch (int err) {	printf("Attention: Something went wrong!!! (%d)\n", err); }

	printExitMessage(r.getID());
	return 0;
}
