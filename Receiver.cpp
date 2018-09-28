#include "Receiver.h"
#include <iostream>
#include "PTools.cpp"
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/types.h>
#include <unistd.h>
#include <cstring>

const char* ftok_path = ".";
const int ftok_id = 'u';

using namespace std;

Receiver::Receiver(int qidIn)
{
	qid = qidIn;
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

bool Receiver::getEvent(long mTypeIn)
{
	cout << "getEvent" << endl;
	
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

bool Receiver::sendEvent(long mTypeIn)
{
	cout << "****mTypeIn = " << mTypeIn << endl;
	cout << "sendEvent" << endl;
	
	cout << "qid = " << qid << ", mTypeIn = " << mTypeIn << endl;
	
	msgr.mType = mTypeIn;
	
	int result = msgsnd(qid, (struct msgbuf *)&msgr, msgr.getSize(), 0);
	
	if (result == -1)
	{
		cout << "Error: Message unsuccessfully sent to the queue! ";
		cin.get();
		
		return false;
	}
	
	return true;
}

bool Receiver::terminate()
{
	int result = msgctl(qid, IPC_RMID, NULL);
	
	if (result == 0)
		return true;
	
	return false;
}

void Receiver::cleanUpQueue()
{

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
	int senderBit = 10; // keeping track of senders. 1x represents 997, x1 represents 251 or 257
	
	Receiver receiver(msgget(ftok(ftok_path,ftok_id),IPC_CREAT|0600)); // Removed IPC_EXCL since Q must be shared
	
	if (receiver.id == 1)
	{
		cout << "receiver.id is " << receiver.id << endl;
		while (true)
		{	
			if (senderBit % 10 == 1) // if sender 25x still alive
			{
				if (receiver.getEvent(251) != -1) // if 251 found // should only receive event and print as long as sender is active
				{
					cout << "Received message from 251" << endl; //testing
					
					cout << "Checking if message is for terminating" << endl;
										
					if (strcmp(receiver.msgr.message, "Terminating") == 0) // if 251 is terminating
					{
						senderBit--; // adjust senderBit to x0
						cout << "senderBit is now " << senderBit << endl; // testing
						
					}
					//cout << "Copying 251 ack to msgr" << endl;
					//strcpy(receiver.msgr.message, "251 ack'd by r1");
					//cout << "Sending 251 ack" << endl;
					//receiver.sendEvent(251);
				}
			}
			
			if (senderBit >= 10) // if sender 997 still alive
			{
				if (receiver.getEvent(997) != -1) // if 251 found // should only receive event and print as long as sender is active
				{
					cout << "Received message from 997" << endl; //testing
					
					cout << "Checking if message is for terminating" << endl;
										
					if (strcmp(receiver.msgr.message, "Terminating") == 0) // if 997 is terminating
					{
						senderBit -= 10; // adjust senderBit to 0x
						cout << "senderBit is now " << senderBit << endl; // testing
					}
					
					cout << "Copying 997 ack to msgr" << endl;
					strcpy(receiver.msgr.message, "997 ack'd by r1");
					cout << "Sending 997 ack" << endl;
					receiver.sendEvent(998);
					cout << "Sent" << endl;
					
					if (senderBit < 10)
					{
						cout << "Waiting for final acknowledgement." << endl;
						receiver.getEvent(997);
						cout << "final acknowledgement received." << endl;
						break;
					}
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
	}
	
	else if (receiver.id == 2) // 257 and 997
	{
		cout << "receiver.id is " << receiver.id << endl;
		while (true)
		{	
			if (senderBit % 10 == 1) // if sender 25x still alive
			{
				if (receiver.getEvent(257) != -1) // if 257 found // should only receive event and print as long as sender is active
				{
					cout << "Received message from 257" << endl; //testing
					
					cout << "Checking if message is for terminating" << endl;
										
					if (strcmp(receiver.msgr.message, "Terminating") == 0) // if 251 is terminating
					{
						senderBit--; // adjust senderBit to x0
						cout << "senderBit is now " << senderBit << endl; // testing
						
					}
				}
			}
			
			if (senderBit >= 10) // if sender 997 still alive
			{
				if (receiver.getEvent(1097) != -1) // if 997 found // should only receive event and print as long as sender is active
				{
					cout << "Received message from 997" << endl; //testing
					
					cout << "Checking if message is for terminating" << endl;
										
					if (strcmp(receiver.msgr.message, "Terminating") == 0) // if 997 is terminating
					{
						senderBit -= 10; // adjust senderBit to 0x
						cout << "senderBit is now " << senderBit << endl; // testing
					}
					
					cout << "Copying 997 ack to msgr" << endl;
					strcpy(receiver.msgr.message, "997 ack'd by r2");
					cout << "Sending 997 ack" << endl;
					receiver.sendEvent(1098);
					cout << "Sent" << endl;
					
					if (senderBit < 10)
					{
						cout << "Waiting for final acknowledgement." << endl;
						receiver.getEvent(1097);
						cout << "final acknowledgement received." << endl;
						break;
					}
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
	}
	
	cin.get();
		
	if (!receiver.terminate())
	{
		cout << "Warning: unable to deallocate queue!\n";
		cout << "Press any key to exit program: ";
		cin.get();
	}
	
	return 0;
}
