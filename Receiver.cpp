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

using namespace std;

Receiver::Receiver()
{
	// keeping track of senders. 1x represents 997, x1 represents 251 or 257	
	senderBit = 11;
	getQID();
	assignReceiverNumber();
}

void Receiver::getQID()
{
	qid = msgget(ftok(ftok_path, ftok_id), IPC_CREAT | 0600);
}

void Receiver::assignReceiverNumber() // prompts user for choice
{
	cout << "Which number do you want to assign to this receiver?\n";
	for (int i = 0; i < idListSize; i++)
	{
		cout << i + 1 << ": " << idList[i] << endl;
	}
	
	id = getReceiverNumber();
}

int Receiver::getReceiverNumber() // gets user's choice
{
	int choice;
	
	while(true)
	{
		try
		{		
			cout << "Enter selection: ";
			cin >> choice;
			if (!cin.good() | choice < 1 | choice > idListSize)
				throw int();
			return idList[choice - 1];
		} 
		catch (int e)
		{
			PTools::flushCin();
			cout << "Invalid selection!" << endl;
		}
	}
}

bool Receiver::getMessage(const long& mTypeIn)
{
	cout << "getMessage" << endl;
	
	cout << "qid = " << qid << ", mTypeIn = " << mTypeIn << endl;
	
	if (msgrcv(qid, (struct msgbuf*)&msgr, msgr.getSize(), mTypeIn, 0) != -1)
	{
		//msgCount++;
		cout << "Message found!" << endl;
		cout << "msgr.message = " << msgr.message << endl;
		return true;
	}
	else
	{
		cout << "Message not found!" << endl;
		return false;
	}
	
}

void Receiver::printQueueNotFoundError()
{
	cout << "Error: Message queue is missing!" << endl;
}

void Receiver::printMsg()
{	
	cout << msgr.mType << " sent: " << msgr.message << endl;
}

void Receiver::setMessage(const string& msgIn)
{
	strcpy(msgr.message, msgIn.c_str());
}

void Receiver::setMessageType(const long& mTypeIn)
{
	// code
}

bool Receiver::sendMessage(const long& mTypeIn)
{
	cout << "sendMessage " << msgr.message << " to " << mTypeIn << endl;
	
	//cout << "qid = " << qid << ", mTypeIn = " << mTypeIn << endl;
	
	msgr.mType = mTypeIn;
	
	//cout << "+++Sending message with msgr.mType = " << msgr.mType << endl;
	
	int result = msgsnd(qid, (struct msgbuf *)&msgr, msgr.getSize(), 0);
	
	if (result == -1)
	{
		cout << "Error: Message queue missing! ";
		
		return false;
	}
	
	return true;
}

bool Receiver::sendAcknowledgement()
{
	cout << "sendAcknowledgement" << endl;
	
	setMessage(MSG_ACK);
	
	if (id == 1)
		sendMessage(MTYPE_ACK_997);
	else
		sendMessage(MTYPE_ACK_997_R2);
}

void Receiver::terminateQueue()
{
	cleanUpQueue();
	
	if (isQueueEmpty()) // if 0 then queue is empty
	{
	
		int result = msgctl(qid, IPC_RMID, NULL); // deallocates queue
	
		if (result == 0) // if 0 queue successfully deallocated
		{
			cout << "Queue " << qid << " deallocated!" << endl;
		}
		else
		{
			cout << "Warning: unable to deallocate queue!" << endl;
		}
	}
}

void Receiver::terminateSelf() // only receiver 2 self terminates
{
	if (id == 2)
	{
		if (senderBit >= 10)
		{
			cout << "Notifying 997 of Termination." << endl;
			senderTerminationNotification(MTYPE_POLL_997);
		}
		if (senderBit % 10 == 1)
		{
			cout << "Notifying 257 of Termination." << endl;
			senderTerminationNotification(MTYPE_POLL_257);
		}
	}
	
	senderBit = 00;
}

void Receiver::senderTerminationNotification(const long& mTypeIn)
{		
	getMessage(mTypeIn);	// gets status message from 257 or 997
	setMessage(MSG_TERM);	// updates status message from 257 or 997 with term. msg.
	sendMessage(mTypeIn);	// sends termination msg back to queue for 257 997 to receive.
}

bool Receiver::isQueueEmpty()
{
	struct msqid_ds buf_nfo;
	
	msgctl(qid, IPC_STAT, &buf_nfo); // buf gets queue data including number of messages
	
	if (buf_nfo.msg_qnum == 0)
	{
		return true;
	}
	
	return false;
}

bool Receiver::isTerminate()
{
	cout << "isTerminate" << endl;
	
	if (strcmp(msgr.message, MSG_TERM.c_str()) == 0)
	{
		cout << "True" << endl;
		return true;
	}
	
	cout << "False" << endl;
	
	return false;
}

void Receiver::cleanUpQueue()
{
	while (!isQueueEmpty())
	{
		getMessage(-2000);
		cout << msgr.message << endl;
	}
	
	cout << "Queue is now empty." << endl;
}

void disconnectSender(Receiver& r) // use after confirming sender terminated
{
	if (r.msgr.mType == r.MTYPE_EVENT_997)
	{
		r.senderBit -= 10; // disassociates 997	
	}
	
	else if (r.msgr.mType == r.MTYPE_EVENT_251)
	{
		r.senderBit -= 1;
	}
}

void processMessage(Receiver& r)
{
	if (r.isTerminate()) // if 997 is terminating
	{
		//r.senderBit -= 10; // adjust senderBit to 0x
		disconnectSender(r);
		cout << "***********senderBit is now " << r.senderBit << endl; // testing
	}
	else
	{
		// if message count is 5000, 257 and 997 must be told to stop
		r.printMsg();
		
		r.msgCount++; 
	}
}

void process25x(Receiver& r)
{
	if (r.senderBit % 10 == 1) // if set to send to 25x
	{
		if (r.id == 1)
			r.current_mType = r.MTYPE_EVENT_251;
		else
			r.current_mType = r.MTYPE_EVENT_257;

		r.getMessage(r.current_mType); // Gets message from 25x
		
		processMessage(r); // Checks if termination message
	}
}

void process997(Receiver& r)
{
	if (r.senderBit >= 10 && (r.msgCount < r.MSG_COUNT_MAX_R2 | r.id == 1)) // if sender 997 still alive
	{
		if (r.id == 1) // Assigns proper receiver code (997 or 1097)
			r.current_mType = r.MTYPE_EVENT_997; 
		else
			r.current_mType = r.MTYPE_EVENT_997_R2;
		
		r.getMessage(r.current_mType); // Gets message from 997
		processMessage(r); // Checks if termination message
		r.sendAcknowledgement(); // Sends 997 ack message
	}
}

bool isMessageCountMax(Receiver& r)
{
	if (r.id == 2)
	{
		if(r.msgCount == r.MSG_COUNT_MAX_R2)
		{
			return true;
		}
	}
	return false;
}

void doQueueDeallocation(Receiver& r)
{
	if (r.id == 1)
	{
		r.getMessage(2); // stalls if receiver 2 alive
		r.terminateQueue();
		
	}	
}

void postCompletion(Receiver& r) // could send out 3x for r1, s257, and s997
{
	if (r.id == 2)
	{
		r.sendMessage(2);
	}
}

void doTerminateSelf(Receiver& r)
{
	cout << "doTerminateSelf msgCount = " << r.msgCount << endl;
	if (isMessageCountMax(r))
	{
		r.terminateSelf();
	}	
}

void sendPoll(Receiver& r)
{
	cout << "sendPoll" << endl;
	
	r.setMessage(r.MSG_ALIVE);

	if (r.senderBit % 10 == 1)
	{		
		if (r.id == 1)
		{
			r.sendMessage(r.MTYPE_POLL_251);
		}
	}
	/*
	if (r.senderBit >= 10)
	{
		r.sendMessage(r.MTYPE_POLL_997);
	}
	*/
}

void getPoll(Receiver& r)
{
	cout << "getPoll" << endl;
	
	if (r.senderBit % 10 == 1)
	{	
		if (r.id == 1)
		{
			r.getMessage(r.MTYPE_POLL_251);
			
			if (r.isTerminate())
			{
				--r.senderBit;
				r.setMessage(r.MSG_TERM);
				r.sendMessage(r.MTYPE_POLL_997);
			}
		}
	}
	/*
	if (r.senderBit >= 10)
	{
		r.getMessage(r.MTYPE_POLL_997);
		
		if (r.isTerminate())
		{
			r.senderBit -= 10;
		}
	}
	*/	
}

// Communicates with sender 2xx
void startComm2xx(Receiver& r)
{
	while (r.senderBit % 10 == 1)
	{
		sendPoll(r);
		process25x(r);
		getPoll(r);
	}
}

// Comunicates wtih sender 997
void startComm997(Receiver& r)
{ 
	while (r.senderBit >= 10)
	{
		sendPoll(r);
		process997(r); // Gets 997 message, and acknowledges	
		doTerminateSelf(r); // If R2 reach max message then terminate
		getPoll(r);	
	}
}

/////////////////////////MAIN PROGRAM////////////////////////////////////////////////



int main()
{	 	
	// Removed IPC_EXCL since Q must be shared
	Receiver receiver;
	
	while (receiver.senderBit != 0)
	{
		cout << "******senderBit = " << receiver.senderBit << endl;
		
		sendPoll(receiver);
		
		process25x(receiver); // Gets 25x message
		
		process997(receiver); // Gets 997 message, and acknowledges
		
		doTerminateSelf(receiver); // If R2 reach max message then terminate
		
		getPoll(receiver); // gets p
		
	} // end while, terminate is true
	
	// If r1 make sure r2 is complete then deallocate queue
	doQueueDeallocation(receiver);
	
	// If r2 send message in queue to notify r1 that r2 is complete
	postCompletion(receiver);
		
	PTools::flushCin();
	
	cout << "Press any key to exit program: ";
	cin.get();
	
	return 0;
}
