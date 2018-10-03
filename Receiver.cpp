#include "Receiver.h"
#include <iostream>
#include "PTools.cpp"
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/types.h>
#include <unistd.h>
#include <string>
#include <cstring>

const char* ftok_path = ".";
const int ftok_id = 'u';

using namespace std;

Receiver::Receiver(int qid_in)
{
	// keeping track of senders. 1x represents 997, x1 represents 251 or 257	
	senderBit = 10;
	
	qid = qid_in;
	id = assignReceiverNumber();
}

int Receiver::assignReceiverNumber() // prompts user for choice
{
	cout << "Which number do you want to assign to this receiver?\n";
	for (int i = 0; i < idListSize; i++)
	{
		cout << i + 1 << ": " << idList[i] << endl;
	}
	
	return getReceiverNumber();
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
	cout << "sendMessage" << endl;
	
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
	if (senderBit >= 10)
	{
		cout << "Notifying 997 of Termination." << endl;
		
		getMessage(MTYPE_POLL_997);	// gets status message from 997
		setMessage(MSG_TERM);	// updates status message from 997 with term. msg.
		sendMessage(MTYPE_POLL_997);	// sends termination msg back to queue for 997 to get.
	}
	if (senderBit % 10 == 1)
	{
		cout << "Notifying 25x of Termination." << endl;

		getMessage(MTYPE_POLL_257);	// gets status message from 257
		setMessage(MSG_TERM);	// updates status message from 257 with term. msg.
		sendMessage(MTYPE_POLL_257);	// sends terminationn msg back to queue for 257 to get
		
	}
	senderBit = 00;
}

bool Receiver::isQueueEmpty()
{
	struct msqid_ds buf_nfo;
	
	msgctl(qid, IPC_STAT, &buf_nfo); // buf gets queue data including number of messages
	
	//buf_nfo.msg_qnum = 0; // messages no longer accessible
	
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

void disconnectSender(Receiver& r, long marker)
{
	if (marker == 997)
	{
		r.senderBit -= 10; // disassociates 997	
	}
	
	else if (marker == 257)
	{
		r.senderBit -= 1;
	}
}

void processMessage(Receiver& r)
{
	if (r.isTerminate()) // if 997 is terminating
	{
		//r.senderBit -= 10; // adjust senderBit to 0x
		disconnectSender(r, 997);
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
		{
			r.current_mType = r.MTYPE_EVENT_251;
		}
		else if (r.id == 2)
		{
			r.current_mType = r.MTYPE_EVENT_257;
		}
		else
		{
			cout << "Error: receiver id has invalid value which neither 1 or 2." << endl;
		}

		if (r.getMessage(r.current_mType) != -1) // if 251 
		// should only receive event and print as long as sender is active
		{
			cout << "Received message from " << r.current_mType << endl; //testing
								
			if (r.isTerminate()) // if 25x is terminating
			{
				//r.senderBit--; // adjust senderBit to x0
				disconnectSender(r, 257);
				
				// testing
				cout << "senderBit is now " << r.senderBit << endl; 
			}
			else
			{
				r.printMsg();
				++r.msgCount;
			}
		}
		else
		{
			r.printQueueNotFoundError();
		 	return; // message queue is gone
		}	
	}
}

void process997(Receiver& r)
{
	if (r.senderBit >= 10 && (r.msgCount < r.MSG_COUNT_MAX_R2 | r.id == 1)) // if sender 997 still alive
	{
		if (r.id == 1)
			r.current_mType = r.MTYPE_EVENT_997;
		else
			r.current_mType = r.MTYPE_EVENT_997_R2;
		
		/*
		if (r.getMessage(r.current_mType) != -1) // if 997 found // should only
		// receive event and print as long as sender is active
		{
			if (r.isTerminate()) // if 997 is terminating
			{
				//r.senderBit -= 10; // adjust senderBit to 0x
				disconnectSender(997);
				cout << "***********senderBit is now " << r.senderBit << endl; // testing
			}
			else
			{
				// if message count is 5000, 257 and 997 must be told to stop
				r.printMsg();
				r.msgCount++; 
			}
				
			r.sendAcknowledgement();				
		}
		*/
		
		r.getMessage(r.current_mType);
		processMessage(r);
		r.sendAcknowledgement();
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

/////////////////////////MAIN PROGRAM////////////////////////////////////////////////



int main()
{	 	
	// Removed IPC_EXCL since Q must be shared
	Receiver receiver(msgget(ftok(ftok_path, ftok_id), IPC_CREAT | 0600)); 
	
	while (receiver.senderBit != 0)
	{
		cout << "******senderBit = " << receiver.senderBit << endl;
		
		process25x(receiver);
		
		process997(receiver);
		
		doTerminateSelf(receiver);
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
