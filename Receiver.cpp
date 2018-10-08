#include "Receiver.h"
#include <iostream>
#include "PTools.cpp"
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/types.h>
#include <unistd.h>
#include <string>
#include <cstring>
#include <pthread.h>

//using namespace std;

Receiver::Receiver()
{
	// keeping track of senders. 1x represents 997, x1 represents 251 or 257	
	senderBit = 11;
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
	std::cout << "Which number do you want to assign to this receiver?\n";
	for (int i = 0; i < idListSize; i++) 
		std::cout << i + 1 << ": " << idList[i] << std::endl;
	
	id = getReceiverNumber();
}

int Receiver::getReceiverNumber() // gets user's choice
{
	int choice;
	
	while(true)
	{
		try
		{		
			std::cout << "Enter selection: ";
			std::cin >> choice;
			if (!std::cin.good() | choice < 1 | choice > idListSize)
				throw int();
			return idList[choice - 1];
		} 
		catch (int e)
		{
			PTools::flushCin();
			std::cout << "Invalid selection!" << std::endl;
		}
	}
}

void Receiver::getMessage(const long& mTypeIn)
{
	//std::cout << "+getMessage" << std::endl;
	//std::cout << "mType " << mTypeIn << std::endl;
		
	//struct msqid_ds buf;
	//msgctl(qid, IPC_STAT, &buf);
	//std::cout << "# of messages before checking: " << buf.msg_qnum << std::endl;
	if (isThread25x())
	{
		//std::cout << "qid " << qid << std::endl;
		msgrcv(qid, (struct msgbuf*)&msgr25x, msgr25x.getSize(), mTypeIn, 0);
		//msgCount++;
		//std::cout << "Message found! (" << msgr25x.mType << ')' << std::endl;
		//std::cout << "msgr25x.message " << msgr25x.message << std::endl;
		return;
	}
	//std::cout << "qid " << qid << std::endl;
		
	msgrcv(qid, (struct msgbuf*)&msgr997, msgr997.getSize(), mTypeIn, 0);
	//msgCount++;
	//std::cout << "Message found! (" << msgr997.mType << ')' << std::endl;
	//std::cout << "msgr997.message " << msgr997.message << std::endl;

}

void Receiver::printQueueNotFoundError()
{
	std::cout << "Error: Message queue is missing!" << std::endl;
}

void Receiver::printMsg()
{
	if (isThread25x())
	{
		std::cout << msgr25x.mType << " sent: " << msgr25x.message << std::endl;
		return;
	}

	std::cout << msgr997.mType << " sent: " << msgr997.message << std::endl;
}

void Receiver::sendMessage(const long& mTypeIn)
{
	//std::cout << "+sendMessage " << std::endl;
	//std::cout << "mType " << mTypeIn << std::endl;
	int result = 0;		
	//struct msqid_ds buf;
	//msgctl(qid, IPC_STAT, &buf);
	//std::cout << "# of messages before sending: " << buf.msg_qnum << std::endl;
		
	if (isThread25x())
	{
		msgr25x.mType = mTypeIn;
		result = msgsnd(qid, (struct msgbuf *)&msgr25x, msgr25x.getSize(), 0);
	}
	else
	{
		msgr997.mType = mTypeIn;
		result = msgsnd(qid, (struct msgbuf *)&msgr997, msgr997.getSize(), 0);
	}
	if (result == -1)
		std::cout << "Error: Message queue missing! ";
	//std::cout << "message sent" << std::endl;	
}

void Receiver::setMessage(const string& msgIn)
{
	//std::cout << "+setMessage " << std::endl;
	//std::cout << "message: " << msgIn << std::endl;
	if (isThread25x())
	{
		strcpy(msgr25x.message, msgIn.c_str());
		return;
	}	

	strcpy(msgr997.message, msgIn.c_str());
}

bool Receiver::isThread25x()
{
	//std::cout << "isThread25x " << (bool)(pthread_equal(thread_id, pthread_self())) << std::endl;
	return pthread_equal(thread_id, pthread_self());
}

void Receiver::sendAcknowledgement()
{
	//std::cout << "+sendAcknowledgement" << std::endl;
	setMessage(MSG_ACK);
	
	if (senderBit >= 10)
	{
		if (id == 1)
		{
			sendMessage(MTYPE_ACK_997);
			return;
			//std::cout << "mType " << MTYPE_ACK_997 << std::endl;
		}
		
		sendMessage(MTYPE_ACK_997_R2);
		return;
		//std::cout << "mType " << MTYPE_ACK_997_R2 << std::endl;	
		//std::cout << "Acknowledgement Sent" << std::endl;
	}
	//std::cout << "Acknowledgement NOT Sent" << std::endl;
}

void Receiver::terminateQueue()
{
	//std::cout << "+terminateQueue" << std::endl;
	cleanUpQueue();
	
	if (isQueueEmpty()) // if isQueueEmpty is 0 then queue is empty
	{	
		if (msgctl(qid, IPC_RMID, NULL) == 0) // if msgctl returns 0 then queue successfully deallocated
		{
			std::cout << "Queue " << qid << " Deallocated!" << std::endl;
			return;
		}

		std::cout << "Warning: unable to deallocate queue!" << std::endl;
	}
}

void Receiver::terminateSelf() // only receiver 2 self terminates
{ 
	//std::cout << "+terminateSelf" << std::endl;
	//std::cout << "senderBit is " << senderBit << std::endl;
	if (id == 2)
	{
		if (isThread25x())
		{
			if (senderBit % 10 == 1)
			{
				std::cout << "Notifying 257 of Termination." << std::endl;
				senderTerminationNotification(MTYPE_POLL_257);
				--senderBit;
			}
		}
		else if (senderBit >= 10)
		{
			std::cout << "Notifying Sender 997 of Termination." << std::endl;
			senderTerminationNotification(MTYPE_POLL_997);
			setMessage("Receiver 2 has Terminated.");
			sendMessage(MTYPE_ACK_997_R2); // sent incase 997 misses terminating poll
			senderBit -= 10;
		}
	}
	//std::cout << "senderBit is now " << senderBit << std::endl;
}

// Used to send termination message to Sender 257
void Receiver::senderTerminationNotification(const long& mTypeIn)
{		
	//std::cout << "+senderTerminationNotification" << std::endl;
	//std::cout << "mType " << mTypeIn << std::endl;
	getMessage(mTypeIn);	// gets status message from 257 or 997
	setMessage(MSG_TERM);	// updates status message from 257 or 997 with term. msg.
	sendMessage(mTypeIn);	// sends termination msg back to queue for 257 997 to receive.
}

bool Receiver::isQueueEmpty()
{
	//std::cout << "+isQueueEmpty" << std::endl;
	return getQueueMessageCount() == 0;
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

void Receiver::cleanUpQueue()
{	
	//std::cout << "+cleanUpQueue" << std::endl;
	int leftOverMessages = 0;
	bool r2Active = true;
	while (!isQueueEmpty() | r2Active)
	{
		getMessage(-2000);
		
		if (msgr997.mType == 2)
		{
			r2Active = false;
			//std::cout << "r2Active now false" << std::endl;
		}
		//std::cout << msgr25x.message << std::endl;
		++leftOverMessages;
	}
	std::cout << leftOverMessages << " Messages were found and removed from the Queue." << std::endl;
	std::cout << "Queue is now empty." << std::endl;
}

void Receiver::disconnectSender() // use after confirming sender terminated
{
	//std::cout << "+disconnectSender" << std::endl;
	if (isThread25x())
	{	
		if (msgr25x.mType == MTYPE_EVENT_251)
		{
			senderBit -= 1;
		}
	}
	else if (msgr997.mType == MTYPE_EVENT_997)
	{
		senderBit -= 10; // disassociates 997	
	}	
}

void Receiver::processMessage()
{
	//std::cout << "+processMessage" << std::endl;
	if (isMessageTerminate()) // if 997 is terminating
	{
		//senderBit -= 10; // adjust senderBit to 0x
		disconnectSender();
		return;
		//std::cout << "***********senderBit is now " << senderBit << std::endl; // testing
	}
	// if message count is 5000, 257 and 997 must be told to stop
	printMsg();
	msgCount++; 
}

void Receiver::process25x()
{
	//std::cout << "+process25x" << std::endl;
	if (senderBit % 10 == 1) // if set to send to 25x
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
	if (senderBit >= 10 && (msgCount < MSG_COUNT_MAX_R2 | id == 1)) // if sender 997 still alive
	{
		if (id == 1) // Assigns proper receiver code (997 or 1097)
			getMessage(MTYPE_EVENT_997); 
		else
			getMessage(MTYPE_EVENT_997_R2);
		
		processMessage(); // Checks if termination message
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

void Receiver::postCompletion() // could send out 3x for r1, s257, and s997
{
	//std::cout << "+postCompletion" << std::endl;
	if (id == 2)
	{
		sendMessage(2);
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

/*/ Using senderTerminationNotification instead
void Receiver::sendPoll()
{
	//std::cout << "+sendPoll" << std::endl;
	if (senderBit % 10 == 1)
	{		
		if (id == 1)
		{
			setMessage(MSG_ALIVE);
			sendMessage(MTYPE_POLL_251);
			return;
		}
	}
	//std::cout << "no poll sent" << std::endl;
}
*/

/*/ Sender 251 does not require polling (just sends termination message)
void Receiver::getPoll()
{
	//std::cout << "+getPoll" << std::endl;
	if (senderBit % 10 == 1)
	{	
		if (id == 1)
		{
			getMessage(MTYPE_POLL_251);
			
			if (isMessageTerminate())
			{
				--senderBit;
				setMessage(MSG_TERM);
				sendMessage(MTYPE_POLL_251);
			}
		}
	}
}
*/

void Receiver::waitForSenders()
{
	if (id == 1)
	{
		if (senderBit % 10 == 1)
			getMessage(253);
		if (senderBit >= 10)
			getMessage(999);
		if (senderBit % 10 == 1)
			sendMessage(254);
		if (senderBit >= 10)		
			sendMessage(1000);
		getMessage(4);			
	}
	else
	{
		if (senderBit % 10 == 1)
			getMessage(259);
		if (senderBit >= 10)
			getMessage(1099);
		if (senderBit % 10 == 1)
			sendMessage(260);
		if (senderBit >= 10)
			sendMessage(1100);
	}
}


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

void Receiver::sendStartMessages()
{
	if (id == 2)
	{
		setMessage("ALL SYSTEMS GO");				
		sendMessage(4);
		sendMessage(4);
		sendMessage(4);
		sendMessage(4);
	}
}

/////////////////////////MAIN PROGRAM////////////////////////////////////////////////

// Communicates with sender 2xx
void* startComm25x(void* rIn) // How to get this into Receiver class?
{	
	//std::cout << "+startComm25x" << std::endl;
	Receiver *r = (Receiver*)rIn;
	
	r->thread_id = pthread_self();
	
	while (r->senderBit % 10 == 1)
	{
		//std::cout << "****************startComm25x loop top" << std::endl;
		//std::cout << "senderBit is " << r->senderBit << std::endl;
		//sendPoll(*r); // to 251
		r->process25x();
		r->doTerminateSelf();
		//getPoll(*r); // from 251
		//std::cout << "senderBit is now " << r->senderBit << std::endl;
	}
	
	//std::cout << "**********************startComm25x ending" << std::endl;
}

// Communicates with sender 997
void* startComm997(void* rIn)
{ 
	//std::cout << "+startComm997" << std::endl;
	Receiver *r = (Receiver*)rIn;	
	
	while (r->senderBit >= 10)
	{
		//std::cout << "*****************startComm997 loop top" << std::endl;
		//std::cout << "senderBit is " << r->senderBit << std::endl;
		//997 should poll r2
		//sendPoll(*r); // sends for 997
		r->process997(); // Gets 997 message, and acknowledges	
		r->doTerminateSelf(); // If R2 reach max message then terminate
		r->sendAcknowledgement(); // Sends 997 ack message
		//getPoll(*r);	
		//std::cout << "senderBit is now " << r->senderBit << std::endl;		
	}
	
	//std::cout << "*******************startComm997 ending" << std::endl;
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
		
		// Waits for all senders to send message to queue
		// Sends response --- may remove this part
		receiver.waitForSenders(); // synchronize start	
		
		// Receiver 2 sends start message to all other programs
		receiver.sendStartMessages();

		// Initiates threads
		pthread_create(&senderThread25xID, NULL, startComm25x, &receiver);
		pthread_create(&senderThread997ID, NULL, startComm997, &receiver);
			
		// Program resumes here when both threads complete
		pthread_join(senderThread25xID, NULL);
		pthread_join(senderThread997ID, NULL);
		
		//std::cout << "Both Threads are now complete." << std::endl;
		
		// Deallocates queue when both receivers have finished
		receiver.doQueueDeallocation();
		
		// Receiver 2 will notify Receiver 1 that it has finished
		receiver.postCompletion();
		
	}
	catch (int qError)
	{
		std::cout << "Error: Message Queue not found. (" << qError << ')' << std::endl;
		std::cout << "Press enter to continue... ";
	}
	// Clears input stream buffer
	PTools::flushCin();
	
	// Notify user that Reciever program has completed
	std::cout << "Press any key to exit program... ";
	std::cin.get();
	
	return 0;
}
