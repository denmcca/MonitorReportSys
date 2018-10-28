#define REC1 1
#define REC2 2
#include "Receiver.h"
#include <iostream>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/types.h>
#include <unistd.h>
#include <string>
#include <cstring>
#include <sys/wait.h>
#include <stdio.h>

Receiver::Receiver()
{
	// keeping track of senders. 1x represents 997, x1 represents 251 or 257	
	getFrom25x = true;
	getFrom997 = true;
	assignReceiverNumber();
	initializeQueue();
}

void Receiver::initializeQueue()
{
	if (id == 1) qid = msgget(ftok(ftok_path, ftok_id), IPC_CREAT | 0600);
	else if (id == 2)
	{	
		std::cout << "Waiting for Receiver 1 to allocate message queue (Control-C to abort)";
		while (true)
		{
			qid = msgget(ftok(ftok_path, ftok_id), 0);
			if (qid != -1) break;
			
			std::cout << '.';
			std::cout.flush();
			// Sleep only used for aesthetical purpose
			sleep(1);
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
	int choice;
	
	while(true)
	{
		try
		{		
			printf("Enter selection: ");
			std::cin >> choice;
			if (choice < 1 | choice > 2) throw int(choice);
			return choice;
		} 
		catch (int e) { printf("%d is an invalid selection!\n", e); }
	}
}

void Receiver::getMessage(const long& mTypeIn)
{
	if (msgrcv(qid, (struct msgbuf*)&msgr, msgr.getSize(), mTypeIn, 0) == -1) 
		throw ErrorCode(-11, mTypeIn);
}

void Receiver::sendMessage(const long& mTypeIn, const string& msgIn)
{
	MsgPigeon msg;
	msg.message.srcID = id;
	msg.mType = mTypeIn;
	strcpy(msg.message.message, msgIn.c_str());
	
	if(msgsnd(qid, (struct msgbuf *)&msg, msg.getSize(), 0) == -1)
		throw ErrorCode(-10);
}

void Receiver::sendAcknowledgement()
{
	if (!getFrom25x) wait(0);

	int fork_id;
	fork_id = fork();
	
	if (fork_id == 0)
	{
		sendMessage(MTYPE_997_ACK, MSG_ACK);
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
	
	if (msgr.message.srcID == MTYPE_251 | msgr.message.srcID == MTYPE_257)
		if (!getFrom25x) return false; // junk message from Sender 251 or 257; continue
	else if (msgr.message.srcID == MTYPE_997)
		if (!getFrom997) return false; // junk message from Sender 997; continue
	
	printMessage();
	
	if (msgr.message.srcID == MTYPE_997) sendAcknowledgement();
	
	// Receiver 2 terminates at message count max
	msgCount++;
	return true; 
}

bool Receiver::isMessageTerminating()
{
	return strcmp(msgr.message.message, MSG_TERM.c_str()) == 0;
}

// Use after confirming sender terminated
void Receiver::disconnectSender() 
{
	if (msgr.message.srcID == (int)MTYPE_251 | msgr.message.srcID == (int)MTYPE_257)
	{	
		// Disconnects Sender 251 or 257
		getFrom25x = false; 
	}
	else if (msgr.message.srcID == (int)MTYPE_997)
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
	if (id == 1) 
	{
		getMessage(12);
		
		terminateQueue();
	}
	else if (id == REC2) sendMessage(12, "Receiver 2 Terminated!");
	
	wait(0);
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
	while (wait(0) > 0){}
	
	int leftOverMessages = 0;
	int leftOver251 = 0;
	int leftOver257 = 0;
	int leftOver997 = 0;
	int leftOver997R2 = 0;
	
	while (!isQueueEmpty()) 
	{
		getMessage(MTYPE_QUEUE_CLEAN);
		
		if (msgr.message.srcID == 251)
			leftOver251++;
		else if (msgr.message.srcID == 257)
			leftOver257++;
		else if (msgr.message.srcID == 997 && msgr.mType == 1)
			leftOver997++;
		else if (msgr.message.srcID == 997 && msgr.mType == 2)
			leftOver997R2++;
		
		++leftOverMessages;
	}
	
	printf("Messages removed before queue termination count: %d\n", leftOverMessages);
	printf("251: %d, 257: %d, 997_R1: %d, 997_R2: %d\n", leftOver251, leftOver257, leftOver997, leftOver997R2);
}


bool Receiver::isQueueEmpty()
{
	return getQueueMessageCount() == 0;
}

void Receiver::doTerminateSelf()
{
	if (id == 2)
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

void Receiver::terminateSelf() // only receiver 2 self terminates
{

	if (getFrom25x)
	{
		sendMessage(MTYPE_POLL_257, MSG_TERM);
		printf("Sent Sender 257 termination notification.\n");					
		getFrom25x = false;
	}
	if (getFrom997)
	{			
		sendMessage(MTYPE_997_ACK, MSG_TERM);
		printf("Sent Sender 997 termination notification.\n");			
		getFrom997 = false;
	}
}

//Handshaking: To sync all processes
void Receiver::waitForSenders()
{
	if (id == REC1)
	{
		getMessage(id);
	}
	if (id == REC2)
	{
		if (getFrom25x)
		{
			getMessage(id);
			getMessage(id);
		}
		if (getFrom997) getMessage(id);
	}
}

void Receiver::notifyStart()
{
		std::cout << "notifyStart" << std::endl << std::flush;
	if (id == REC1)
	{
		if (getFrom25x)
		{
			std::cout << "REC1 HERE2" << std::endl << std::flush;		
			sendMessage(MTYPE_251, "Sender 251 Start!");
			sendMessage(MTYPE_257, "Sender 257 Start!");
		}
		if (getFrom997) 
		{
			std::cout << "REC1 HERE3" << std::endl << std::flush;
			sendMessage(MTYPE_997, "Sender 997 Start!");
		}
	}
	if (id == REC2) 
	{
			std::cout << "REC2 start!" << std::endl << std::flush;
		sendMessage(REC1, "All Processes Ready! Start!");
	}
}

int Receiver::getQueueMessageCount()
{
	return msgr.getQueueMessageCount(qid);
}

// Prints error message when called
void printError(ErrorCode err, int QID)
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
	
	msgctl(QID, IPC_RMID, NULL);
	
	printf("Press enter to continue... \n\n");
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
	printf("Program has completed... \n\n");
}

int Receiver::getQID() { return qid; }

void Receiver::startReceiver()
{	
	waitForSenders();
	notifyStart();
	
	while (getFrom25x | getFrom997)
	{	
		getMessage(id);
		if (!processMessage()) continue;
		doTerminateSelf();
	}
	
	printf("Exiting after receiving %d events.\n", msgCount);
	doQueueDeallocation();		
}

int main()
{
	int QID;	

	try
	{ 	
		// Instantiates Receiver object
		Receiver r;				
		QID = r.getQID();
		r.startReceiver();	
	}
	catch (int qError)
	{		
		printError(qError, QID);
		return qError;
	}
	catch (ErrorCode err)
	{
		printError(err, QID);
		return err.errorCode;
	}
	
	//clearCinBuffer();
	printExitMessage();
	
	return 0;
}
