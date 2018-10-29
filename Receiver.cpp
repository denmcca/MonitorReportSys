#include "Receiver.h"

Receiver::Receiver()
{
	// keeping track of senders. 1x represents 997, x1 represents 251 or 257	
	getFrom25x = true;
	getFrom997 = true;
	isPrinting = true;
}

void Receiver::initialize()
{
	assignReceiverNumber();
	initializeQueue();
}

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

// Prompts user for choice
void Receiver::assignReceiverNumber() 
{
	printf("Which receiver do you want to activate?\n");
	
	// Printing choices
	for (int i = 0; i < 2; i++) printf("%d: Receiver %d\n", i + 1, i + 1);
	
	// Gets choice
	id = promptReceiverNumber();
}

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
		else	return choice - '0';
	}
}

void Receiver::getMessage(const long& mTypeIn)
{
	if (msgrcv(qid, (struct msgbuf*)&msgr, msgr.getSize(), mTypeIn, 0) == -1) 
		throw ErrorCode(-11, mTypeIn);
}

void Receiver::sendMessage(const long& mTypeIn, const std::string& msgIn)
{
	msgr.message.srcID = id;
	msgr.mType = mTypeIn;
	strcpy(msgr.message.message, msgIn.c_str());
	
	if(msgsnd(qid, (struct msgbuf *)&msgr, msgr.getSize(), 0) == -1)
		throw ErrorCode(-10, (int)mTypeIn);
}

void Receiver::sendAcknowledgement()
{
	wait(0);

	int fork_id;
	fork_id = fork();
	
	if (fork_id == 0)
	{
		if (id == REC1) sendMessage(REC1_ACK, MSG_ACK);
		else sendMessage(REC2_ACK, MSG_ACK);
		exit(0);
	}
	else if (fork_id < 0) throw (ErrorCode(-8, fork_id));
}

bool Receiver::processMessage()
{		
	// if 997 or 251 is terminating
	if (isMessageTerminating()) 
	{
		disconnectSender();
		return true;
	}
	
	if (isPrinting)
	{
		if (msgr.message.srcID == S251 | msgr.message.srcID == S257)
		{
			if (!getFrom25x) return false; // junk message from Sender 251 or 257; continue
		}
		else if (msgr.message.srcID == S997)
		{
			if (!getFrom997) return false; // junk message from Sender 997; continue
		}
		printMessage();
		
		if (msgr.message.srcID == S997) sendAcknowledgement();
		
		// Receiver 2 terminates at message count max
		msgCount++;
		return true;
	}
	return false; 
}

bool Receiver::isMessageTerminating()
{
	return strcmp(msgr.message.message, MSG_TERM.c_str()) == 0;
}

// Use after confirming sender terminated
void Receiver::disconnectSender() 
{
	if (msgr.message.srcID == (int)S251 | msgr.message.srcID == (int)S257)
	{	
		// Disconnects Sender 251 or 257
		getFrom25x = false; 
	}
	else if (msgr.message.srcID == (int)S997)
	{
		// Disconnects Sender 997
		getFrom997 = false; 	
	}
	 // disconnectSender received unexcepted srcID
	else throw ErrorCode(-9, msgr.message.srcID);
	
	printf("Disconnected Sender %d\n", (int)msgr.message.srcID);	
}

void Receiver::printMessage()
{
	printf("Sender %d sent: %s\n", (int)msgr.message.srcID, msgr.message.message);	
}

void Receiver::doQueueDeallocation()
{
	if (id == REC1) 
	{
		getMessage(REC2_TERM);		
		terminateQueue();
	}
	else if (id == REC2)
	{
		sendMessage(REC2_TERM, "Receiver 2 Terminated!");
	}
	
	//while (wait(0) > 0);
}

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

void Receiver::cleanUpQueue()
{	
	//while (wait(0) > 0);
	
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


bool Receiver::isQueueEmpty()
{
	return messageQueueCount() == 0;
}

void Receiver::doTerminateSelf()
{
	if (id == REC2 && isPrinting)
	{
		if (isMessageCountMax())
		{
			terminateSelf();
		}
	}	
}

bool Receiver::isMessageCountMax()
{
	if (msgCount >= MSG_COUNT_MAX_R2) return true;
	return false;
}

void Receiver::terminateSelf()
{
	if (getFrom25x)
	{
		getMessage(S257_POLL);
		sendMessage(S257_POLL, MSG_TERM);
		printf("Sent Sender 257 termination notification.\n");					
	}
	if (getFrom997)
	{			
		sendMessage(REC2_ACK, MSG_TERM);
		printf("Sent Sender 997 termination notification.\n");			
	}
	
	isPrinting = false;
}

//Handshaking: To sync all processes
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
		sendMessage(REC1, "All Processes Ready! Start!");
	}
}

int Receiver::messageQueueCount()
{
	return msgr.getMessageQueueCount(qid);
}

// Prints error message when called
void Receiver::printError(ErrorCode err, int QID, Receiver &r)
{
	// disconnectSender error
	if (err.errorCode == -9)
	{
		printf("Error (%d): disconnectSender method received unexpected srcID: %d\n", (int)err.errorCode, (int)err.auxCode); 
	}
	
	else if (err.errorCode == 3)
	{
		printf("Error (%d): initializeQueue method received unexpected receiver ID: %d\n", (int)err.errorCode, (int)err.auxCode);
	}
	
	else if (err.errorCode == -8)
	{
		printf("Error (%d): Acknowledgement Fork failed: %d\n", (int)err.errorCode, (int)err.auxCode); 
	}
	// Message Queue not found error
	else if (err.errorCode == -10)
	{
		printf("Error: sendMessage Queue Not Found. (%d)\n", (int)err.errorCode);
	}
	else if (err.errorCode == -11)
	{
		printf("Error (%d): getMessage Queue Not Found. (%d)\n", (int)err.errorCode, (int)err.auxCode);
	}
	
	printf("Failed with %d messages in message queue...\n", r.messageQueueCount());	
	
	msgctl(QID, IPC_RMID, NULL);
	
	printf("Press enter to continue... \n\n");
}

int Receiver::getQID() { return qid; }

void Receiver::startReceiver()
{	
	waitForSenders();
	notifyStart();
	
	while (getFrom25x | getFrom997)
	{	
		if (id == REC1) getMessage(-10);
		else if (id == REC2) getMessage(20);
		
		if (!processMessage()) continue;
		doTerminateSelf();
	}
	
	printf("Exiting after receiving %d events.\n", msgCount);
	doQueueDeallocation();		
}

// Clears input stream buffer
void clearCinBuffer()
{
	std::cin.clear();
	std::cin.ignore(INT8_MAX, '\n');
}

// Prints exit message when called
void printExitMessage()
{
	// Notify user that Reciever program has completed
	printf("Program has completed... \n");
}


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
	catch (int qError)
	{		
		Receiver::printError(qError, QID, r);
		return qError;
	}
	catch (ErrorCode err)
	{
		Receiver::printError(err, QID, r);
		return err.errorCode;
	}
	
	//clearCinBuffer();
	printExitMessage();
	
	return 0;
}
