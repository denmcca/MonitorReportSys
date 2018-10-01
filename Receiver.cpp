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
	MSG_COUNT_MAX_R2 = 100000; // set to 5000 for final
	// keeping track of senders. 1x represents 997, x1 represents 251 or 257	
	senderBit = 10;
	MSG_TERM = "Terminating";
	MSG_ACK = "Acknowledged";
	
	qid = qid_in;
	id = assignReceiverNumber();
	cout << "id is " << id << endl;
}

int Receiver::assignReceiverNumber()
{
	cout << "Which number do you want to assign to this receiver?\n";
	for (int i = 0; i < idListSize; i++)
	{
		cout << i << ": " << idList[i] << endl;
	}
	
	return getReceiverNumber();
}

int Receiver::getReceiverNumber()
{
	int choice;
	
	while(true)
	{
		try
		{		
			cout << "Enter selection: ";
			cin >> choice;
			if (!cin.good() | choice < 0 | choice > idListSize - 1)
				throw int();
			return choice + 1;
		} 
		catch (int e)
		{
			PTools::flushCin();
			cout << "Invalid selection!" << endl;
		}
	}
}

bool Receiver::getMessage(long mTypeIn)
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

void Receiver::setMessage(string msgIn)
{
	strcpy(msgr.message, msgIn.c_str());
}

bool Receiver::sendMessage(long mTypeIn)
{
	cout << "sendMessage" << endl;
	
	cout << "****mTypeIn = " << mTypeIn << endl;
	
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
	
	//string msg = "r" + std::to_string(id) + string(" acknowledgement received");
	//setMessage(msg);
	
	setMessage(MSG_ACK);
	//cout << "Sending 997 ACK" << endl;
	sendMessage(msgr.mType + 1);
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
	if (senderBit > 10)
	{
		cout << "Notifying 997 of Termination." << endl;
		
		getMessage(1096);	// gets status message from 997
		setMessage(MSG_TERM);	// updates status message from 997 with term. msg.
		sendMessage(1096);	// sends terminationn msg back to queue for 997 to get
	}
	if (senderBit % 10 == 1)
	{
		cout << "Notifying 25x of Termination." << endl;

		getMessage(256);	// gets status message from 257
		setMessage(MSG_TERM);	// updates status message from 257 with term. msg.
		sendMessage(256);	// sends terminationn msg back to queue for 257 to get
		
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

bool tempConfirmContinue(Receiver recIn)
{
	cout << "tempConfirmContinue" << endl;
	
	char checkQueue; // used to control loop temporarily.

	cout << "qid = " << recIn.qid << endl;
	
	cout << "Do you want to check queue again? (y/n): ";
	cin >> checkQueue;
	
	if (checkQueue != 'y' & checkQueue != 'Y')
	{
		return false;
	}
	
	return true;
}





/////////////////////////MAIN PROGRAM////////////////////////////////////////////////



int main()
{
	
	// assign codes/mtypes as ints here
	 
	bool terminated = false;
	int msgCount = 0;
	int mType = 0;
	
	// Removed IPC_EXCL since Q must be shared
	Receiver receiver(msgget(ftok(ftok_path, ftok_id), IPC_CREAT | 0600)); 
	
	//setBit here
	
	while (receiver.senderBit != 0)
	{
		////PROCESS SENDER 25X MESSAGE
		if (receiver.id == 1) // handles messages from 251
		{		

			if (receiver.senderBit % 10 == 1) // if sender 25x still alive
			{
				if (receiver.getMessage(251) != -1) // if 251 
				// should only receive event and print as long as sender is active
				{
					cout << "Received message from 251" << endl; //testing
					
					cout << "Checking if message is for terminating" << endl;
										
					if (receiver.isTerminate()) // if 251 is terminating
					{
						receiver.senderBit--; // adjust senderBit to x0
						
						// testing
						cout << "senderBit is now " << receiver.senderBit << endl; 
					}
				}
				else
				{
					receiver.printQueueNotFoundError();
					return 1; // message queue is gone
				}
			}		
		}
		
		else if (receiver.id == 2 && receiver.senderBit % 10 == 1) // handles messages from 257
		{					
			// get 257 message (does not terminate before r2)
			receiver.getMessage(257);
			++msgCount;
			cout << "(1) msgCount = " << msgCount << endl;					
			
			//receiver.setMessage("Alive");
			//receiver.sendMessage(2);
						

			if (msgCount == receiver.MSG_COUNT_MAX_R2)
			{
				cout << "***Set to terminate. (1)" << endl;
				receiver.terminateSelf();
			}
		}
		
		
		
		//// PROCESS 997 MESSAGE			
		if (receiver.senderBit >= 10) // if sender 997 still alive
		{
			if (receiver.id == 1)
				mType = 997;
			else
				mType = 1097;
			
			if (receiver.getMessage(mType) != -1) // if 997 found // should only
			// receive event and print as long as sender is active
			{
				cout << "Received message from 997" << endl; //testing
				
				cout << "Checking if message is for terminating" << endl;
				if (receiver.isTerminate()) // if 997 is terminating
				{
					receiver.senderBit -= 10; // adjust senderBit to 0x
					cout << "senderBit is now " << receiver.senderBit << endl; // testing
				}
				else
				{
					// if message count is 5000, 257 and 997 must be told to stop
					receiver.printMsg();
					++msgCount; 
					cout << "(2) msgCount = " << msgCount << endl;					
				}
					
				receiver.sendAcknowledgement();
				
				if (receiver.id == 2)
				{
					if(msgCount == receiver.MSG_COUNT_MAX_R2)
					{
						cout << "***Set to terminate. (2)" << endl;
						receiver.terminateSelf();
					}
				}
				
			}
			else
			{
				receiver.printQueueNotFoundError();
			}
		}
		
	} // end while, terminate is true
	
	
	// If r1 make sure r2 is complete then deallocate queue
	if (receiver.id == 1)
	{
		receiver.getMessage(2);
		receiver.terminateQueue();
		
	}
	// If r2 send message in queue to notify r1 that r2 is complete
	else if (receiver.id == 2)
	{
		receiver.sendMessage(2);
	}
		
	PTools::flushCin();
	
	cout << "Press any key to exit program: ";
	cin.get();
	
	return 0;
}
