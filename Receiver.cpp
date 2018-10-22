#include "Receiver.h"
#include <iostream>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/types.h>
#include <unistd.h>
#include <string>
#include <cstring>
#include <pthread.h>
#include <stdio.h>

Receiver::Receiver()
{
	// keeping track of senders. 1x represents 997, x1 represents 251 or 257	
	sendTo25x = true;
	sendTo997 = true;
	setMessage("Initializing...");
	initializeQueue();
	assignReceiverNumber();
}

void Receiver::initializeQueue()
{
	qid = msgget(ftok(ftok_path, ftok_id), IPC_CREAT | 0600);
	if (qid == -1)
		throw int(-1);
}

void Receiver::assignReceiverNumber() // prompts user for choice
{
	printf("Which receiver do you want to activate?\n");
	
	for (int i = 0; i < 2; i++) 
		printf("%d: Receiver %d\n", i + 1, i + 1);
	
	id = getReceiverNumber();
}

int Receiver::getReceiverNumber() // gets user's choice
{
	int choice;
	
	while(true)
	{
		try
		{		
			printf("Enter selection: ");
			std::cin >> choice;
			
			if (choice < 1 | choice > 2)
				throw int();
			return choice;
		} 
		catch (int e)
		{
			printf("Invalid selection!\n");
		}
	}
}

void Receiver::getMessage(const long& mTypeIn)
{
	if (isThread25x())
	{
		msgrcv(qid, (struct msgbuf*)&msgr25x, msgr25x.getSize(), mTypeIn, 0);

		return;
	}
		
	msgrcv(qid, (struct msgbuf*)&msgr997, msgr997.getSize(), mTypeIn, 0);
}

void Receiver::printQueueNotFoundError()
{
	printf("Error: message queue is missing!\n");
}

void Receiver::printMsg()
{
	MsgPigeon* msgr;
	int senderNum;
	
	if (isThread25x())
	{
		msgr = &msgr25x;
		senderNum = msgr25x.mType;
	}
	else
	{
		msgr = &msgr997;
		if (id == 1)
			senderNum = msgr->mType;
		else
			senderNum = msgr->mType - 100;
	}
	
	printf("Sender %d sent: %s\n", senderNum, msgr->message);
}

void Receiver::sendMessage(const long& mTypeIn)
{
	int result = 0;		
		
	if (isThread25x())
	{
		msgr25x.mType = mTypeIn;
		result = msgsnd(qid, (struct msgbuf *)&msgr25x, msgr25x.getSize(), 0);
		msgr25x.mType = 0;
		strcpy(msgr25x.message, "");
	}
	else
	{
		msgr997.mType = mTypeIn;
		result = msgsnd(qid, (struct msgbuf *)&msgr997, msgr997.getSize(), 0);
	}
	if (result == -1)
		throw int(result);
}

void Receiver::setMessage(const string& msgIn)
{
	if (isThread25x())
	{
		strcpy(msgr25x.message, msgIn.c_str());
		return;
	}	

	strcpy(msgr997.message, msgIn.c_str());
}

bool Receiver::isThread25x()
{
	return pthread_equal(thread_id, pthread_self());
}

void Receiver::sendAcknowledgement()
{
	setMessage(MSG_ACK);
	const long* MTYPE_ACK;
	
	if (sendTo997)
	{
		if (id == 1)
			MTYPE_ACK = &MTYPE_ACK_997;
		else
			MTYPE_ACK = &MTYPE_ACK_997_R2;
		
		sendMessage(*MTYPE_ACK);
		printf("Acknowledgement sent to Sender997\n");		
	}
}

void Receiver::process25x()
{
	//std::cout << "+process25x" << std::endl;
	if (sendTo25x) // if set to send to 25x
	{
		if (id == 1)
			getMessage(MTYPE_EVENT_251);
		else
			getMessage(MTYPE_EVENT_257);
		
		processMessage(); // Checks if termination message
	}
}

void Receiver::process997()
{
	//std::cout << "+process997" << std::endl;
	if (sendTo997 && (msgCount < MSG_COUNT_MAX_R2 | id == 1)) // if sender 997 still alive
	{
		if (id == 1) // Assigns proper receiver code (997 or 1097)
			getMessage(MTYPE_EVENT_997); 
		else
			getMessage(MTYPE_EVENT_997_R2);
		
		processMessage(); // Checks if termination message
	}
}

void Receiver::processMessage()
{
	//std::cout << "+processMessage" << std::endl;
	if (isMessageTerminate()) // if 997 or 251 is terminating
	{
		disconnectSender();
		return;
	}
	printMsg();	
	// Receiver 2 terminates at message count max
	msgCount++; 
}

bool Receiver::isMessageTerminate()
{
	//std::cout << "+isMessageTerminate" << std::endl;
	if (isThread25x())
	{
		return strcmp(msgr25x.message, MSG_TERM.c_str()) == 0;
	}
	return strcmp(msgr997.message, MSG_TERM.c_str()) == 0;
}

void Receiver::disconnectSender() // use after confirming sender terminated
{
	//std::cout << "+disconnectSender" << std::endl;
	if (isThread25x())
	{	
		if (msgr25x.mType == MTYPE_EVENT_251)
		{
			sendTo25x = false; // disconnects 251
		}
	}
	else if (msgr997.mType == MTYPE_EVENT_997)
	{
		sendTo997 = false; // disconnects 997	
	}	
}

bool Receiver::isMessageCountMax()
{
	//std::cout << "+isMessageCountMax" << std::endl;
	//std::cout << "msgCount " << msgCount << std::endl;
	if (id == 2)
	{
		if(msgCount >= MSG_COUNT_MAX_R2)
		{
			return true;
		}
	}
	return false;
}

void Receiver::doQueueDeallocation()
{
	//std::cout << "+doQueueDeallocation" << std::endl;
	if (id == 1)
	{
		//getMessage(2); // stalls if receiver 2 alive
		terminateQueue();
	}	
}

void Receiver::terminateQueue()
{
	//std::cout << "+terminateQueue" << std::endl;
	cleanUpQueue();
	
	if (isQueueEmpty()) // if isQueueEmpty is 0 then queue is empty
	{	
		if (msgctl(qid, IPC_RMID, NULL) == 0) // if msgctl returns 0 then queue successfully deallocated
		{
			printf("Queue %d is now deallocated!\n", qid);
			return;
		}
		
		printf("Warning: unable to deallocate queue %d!\n", qid);
	}
}

void Receiver::cleanUpQueue()
{	
	//std::cout << "+cleanUpQueue" << std::endl;
	int leftOverMessages = 0;
	int leftOver251 = 0;
	int leftOver257 = 0;
	int leftOver997 = 0;
	int leftOver1097 = 0;
	
	waitForShutdownConfirmations();
	
	while (!isQueueEmpty()) // It is normal that 257 would send more messages than R2 will receive.
	{
		getMessage(MTYPE_QUEUE_CLEAN);
		
		if (msgr997.mType == 251)
			leftOver251++;
		else if (msgr997.mType == 257)
			leftOver257++;
		else if (msgr997.mType == 997)
			leftOver997++;
		else if (msgr997.mType == 1097)
			leftOver1097++;
		
		++leftOverMessages;
	}
	
	printf("Messages removed before queue termination count: %d\n", leftOverMessages);
	printf("251: %d, 257: %d, 997: %d, 1097: %d\n", leftOver251, leftOver257, leftOver997, leftOver1097);
}

bool Receiver::isQueueEmpty()
{
	//std::cout << "+isQueueEmpty" << std::endl;
	return getQueueMessageCount() == 0;
}

void Receiver::postCompletion() // could send out 3x for r1, s257, and s997
{
	//std::cout << "+postCompletion" << std::endl;
	if (id == 2)
	{
		sendMessage(MTYPE_SHUTDOWN_R2);
	}
}

void Receiver::doTerminateSelf()
{
	//std::cout << "+doTerminateSelf" << std::endl;
	if (isMessageCountMax())
	{
		terminateSelf();
	}	
}

void Receiver::terminateSelf() // only receiver 2 self terminates
{ 
	//std::cout << "+terminateSelf" << std::endl;

	if (id == 2)
	{
		if (isThread25x())
		{
			if (sendTo25x)
			{
				senderTerminationNotification(MTYPE_POLL_257);
				printf("Sent Sender257 termination notification.\n");					
				sendTo25x = false;
			}
		}
		else if (sendTo997)
		{			
			setMessage(MSG_TERM);
			sendMessage(MTYPE_ACK_997_R2);
			printf("Sent Sender997 termination notification.\n");			
			sendTo997 = false;
		}
	}
}

// Used to send termination message to Sender 257
void Receiver::senderTerminationNotification(const long& mTypeIn)
{	
	// Gets poll message from 257 or 997	
	getMessage(mTypeIn);
	// Updates poll message from 257 or 997 with termination message	
	setMessage(MSG_TERM);
	// Sends termination message back to queue for 257 997 to receive	
	sendMessage(mTypeIn);	
}

// Handshaking: To sync all processes
void Receiver::waitForSenders()
{
	if (id == 1)
	{
		if (sendTo25x)
			getMessage(MTYPE_SHAKE_251);
		if (sendTo997)
			getMessage(MTYPE_SHAKE_997);
		if (sendTo25x)
			sendMessage(MTYPE_SHAKE_251_ACK);
		if (sendTo997)		
			sendMessage(MTYPE_SHAKE_997_ACK);
		getMessage(MTYPE_R2_READY);			
	}
	else
	{
		if (sendTo25x)
			getMessage(MTYPE_SHAKE_257);
		if (sendTo997)
			getMessage(MTYPE_SHAKE_997_R2);
		if (sendTo25x)
			sendMessage(MTYPE_SHAKE_257_ACK);
		if (sendTo997)
			sendMessage(MTYPE_SHAKE_997_R2_ACK);
	}
}

// Shutdown: To prevent message queue deallocation while process actively using message queue
void Receiver::waitForShutdownConfirmations()
{
	if (id == 1)
	{
		// Messages to receive when each program shutsdown.
		// Used to prevent premature queue deallocation.
		getMessage(MTYPE_SHUTDOWN_257);
		getMessage(MTYPE_SHUTDOWN_997);
		getMessage(MTYPE_SHUTDOWN_R2);
		printf("All other systems have shutdown.\n"); 
	}
}

// 
int Receiver::getQueueMessageLimit()
{
	//std::cout << "getQueueMessageLimit " << msgr25x.getQueueMessageLimit(qid) << std::endl;
	return msgr25x.getQueueMessageLimit(qid);
}

int Receiver::getQueueMessageCount()
{
	//std::cout << "getQueueMessageCount " << msgr25x.getQueueMessageCount(qid) << std::endl;
	return msgr25x.getQueueMessageCount(qid);
}

bool Receiver::isMessageQueueFull()
{
	//std::cout << "isMessageQueueFull " << msgr25x.isMessageQueueFull(qid) << std::endl;
	return msgr25x.isMessageQueueFull(qid);
}

// Receiver 2 signal ready to all other processes 
void Receiver::sendStartMessages()
{
	if (id == 2)
	{
		// 4 messages from Receiver 2 to four other programs
		setMessage("ALL SYSTEMS GO");				
		sendMessage(MTYPE_R2_READY);
		sendMessage(MTYPE_R2_READY);
		sendMessage(MTYPE_R2_READY);
		sendMessage(MTYPE_R2_READY);
	}
}

// Returns true if Sender 251 or 257 is alive, otherwise false
bool Receiver::isSendTo25x()
{
	return sendTo25x;
}

// Returns true if Sender 997 is alive, otherwise false
bool Receiver::isSendTo997()
{
	return sendTo997;
}

// Returns the number of messages received by Receiver
int Receiver::getMessageCount()
{
	return msgCount;
}

// Prints error message when called
void printError(int& errorCode)
{
	printf("Error: Message Queue not found. (%d)", errorCode);
	printf("Press enter to continue... ");
	cin.get();
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
	printf("Press enter to exit program... ");
	cin.get();
}

// Communicates with Sender 251 or 257
void* startComm25x(void* rIn) // How to get this into Receiver class?
{	
	Receiver *r = (Receiver*)rIn;
	r->thread_id = pthread_self();
	
	while (r->isSendTo25x())
	{
		// Gets 251 or 257 message
		r->process25x();
		// If Receiver 2 reaches message count max then terminate
		r->doTerminateSelf();
	}
}

// Communicates with sender 997
void* startComm997(void* rIn)
{ 
	Receiver *r = (Receiver*)rIn;	
	
	while (r->isSendTo997())
	{
		// Gets 997 message, and acknowledges
		r->process997();
		// If Receiver 2 reaches message count max then terminate
		r->doTerminateSelf(); 
		// Sends 997 ack message
		r->sendAcknowledgement(); 
	}
}

int main()
{
	try
	{ 	
		// Instantiates Receiver object
		Receiver receiver;
		
		// Instantiates threads
		// Thread that handles 25x Senders
		pthread_t senderThread25xID;
		// Thread that handles 977 Sender
		pthread_t senderThread997ID;
		
		// Synchronization: Waits for all senders to send message to queue
		receiver.waitForSenders();
		
		// Receiver 2 sends start message to all other programs
		receiver.sendStartMessages();

		// Initiates threads
		pthread_create(&senderThread25xID, NULL, startComm25x, &receiver);
		pthread_create(&senderThread997ID, NULL, startComm997, &receiver);
			
		// Process resumes when both threads complete
		pthread_join(senderThread25xID, NULL);
		pthread_join(senderThread997ID, NULL);
		
		// Deallocates queue when both receivers have finished
		receiver.doQueueDeallocation();
		
		// Receiver 2 will notify Receiver 1 that it has finished
		receiver.postCompletion();
		
		printf("Exiting after receiving %d events.\n", receiver.getMessageCount());		
		
	}
	catch (int qError)
	{		
		printError(qError);
		return qError;
	}
	
	clearCinBuffer();
	printExitMessage();
	
	return 0;
}
