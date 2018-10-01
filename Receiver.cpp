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
	cout << "****mTypeIn = " << mTypeIn << endl;
	cout << "sendMessage" << endl;
	
	cout << "qid = " << qid << ", mTypeIn = " << mTypeIn << endl;
	
	
	if (id == 1)
	{
		msgr.mType = mTypeIn;
	}
	if (id == 2)
	{
		msgr.mType = mTypeIn + 100;
	}
	
	cout << "+++Sending message with msgr.mType = " << msgr.mType << endl;
	
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
	cout << "Copying 997 ack to msgr" << endl;
	string msg = "r" + std::to_string(id) + string(" acknowledgement received");
	setMessage(msg);
	cout << "Sending 997 ack" << endl;
	sendMessage(msgr.mType + 1);
	cout << "Sent" << endl;
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
			cout << "Warning: unable to deallocate queue!\n";
		}
	}
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
	if (strcmp(msgr.message, "Terminating") == 0)
	{
		return true;
	}
	
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


int main()
{
	
	// assign codes/mtypes as ints here		
		
	int senderBit = 10; // keeping track of senders. 1x represents 997, x1 represents 251 or 257
	bool terminated = false;
	int msgCount = 0;
	int mType = 0;
	
	Receiver receiver(msgget(ftok(ftok_path, ftok_id), IPC_CREAT | 0600)); // Removed IPC_EXCL since Q must be shared
	
	//setBit
	
	while (!terminated && senderBit != 0)
	{
		if (receiver.id == 1)
		{		

			if (senderBit % 10 == 1) // if sender 25x still alive
			{
				if (receiver.getMessage(251) != -1) // if 251 
				// should only receive event and print as long as sender is active
				{
					cout << "Received message from 251" << endl; //testing
					
					cout << "Checking if message is for terminating" << endl;
										
					if (receiver.isTerminate()) // if 251 is terminating
					{
						senderBit--; // adjust senderBit to x0
						cout << "senderBit is now " << senderBit << endl; // testing
					}
				}
				else
				{
					receiver.printQueueNotFoundError();
					return 1; // message queue is gone
				}
			}
			
			
			// if termination
				// change senderbit
				// if senderbit is 0
					// cleanup mtypes (997, 251) from queue
					// check if other receiver is alive???
					// terminate self
				
				// if senderbit is 0
					//
			
			/*if (!tempConfirmContinue(receiver))
			{
				break;
			}
			*/
		
		}
		
		else if (receiver.id == 2 && senderBit % 10 == 1) // 257
		{					
			// get 257 message (does not terminate before r2)
			receiver.getMessage(257);
			++msgCount;
			cout << "(1) msgCount = " << msgCount << endl;					
			
			//receiver.setMessage("Alive");
			//receiver.sendMessage(2);
		}
		
		if (msgCount == 5000)
		{
			cout << "***Set to terminate. (1)" << endl;
			terminated = true;
		}
				
		if (senderBit >= 10 && !terminated) // if sender 997 still alive
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
					senderBit -= 10; // adjust senderBit to 0x
					cout << "senderBit is now " << senderBit << endl; // testing
				}
				else
				{
					++msgCount; // if message count is 5000, 257 and 997 must be told to stop
					cout << "(2) msgCount = " << msgCount << endl;					
				}
					
				receiver.sendAcknowledgement();
				
				/* Probably not necessary, if receivers clean up after termination
				if (senderBit < 10)
				{
					cout << "Waiting for final acknowledgement." << endl;
					receiver.getMessage(1097);
					cout << "final acknowledgement received." << endl;
					break;
				}
				*/
			}
			else
			{
				receiver.printQueueNotFoundError();
			}
			
		}
		
		if(msgCount == 5000)
		{
			cout << "***Set to terminate. (2)" << endl;
			terminated = true;
		}
		
	} // end while, terminate is true
	
	if (receiver.id == 1)
	{
		receiver.getMessage(2);
		receiver.terminateQueue();
		
	}
	else if (receiver.id == 2)
	{
		receiver.sendMessage(2);
	}
		
	PTools::flushCin();
	
	cout << "Press any key to exit program: ";
	cin.get();
	
	return 0;
}
