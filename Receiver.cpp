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
	int senderBit = 01; // keeping track of senders. 1x represents 997, x1 represents 251 or 257
	
	Receiver receiver(msgget(ftok(ftok_path,ftok_id),IPC_CREAT|0600)); // Removed IPC_EXCL since Q must be shared
	
	if (receiver.id == 1)
	{
		cout << "receiver.id is " << 1 << endl;
		while (true)
		{
			// get 251
			
			if (senderBit % 10 == 1)
			{
				if (receiver.getEvent(251) != -1) // if 251 found
				{
					cout << "Received message from 251" << endl;
					cout << "Copying 251 ack to msgr" << endl;
					strcpy(receiver.msgr.message, "251 ack'd by r1");
					cout << "Sending 251 ack" << endl;
					receiver.sendEvent(251);
				}
			}
			
			if (senderBit >= 10)
			{
				if (receiver.getEvent(997) != -1) // if 997 found
				{	
					cout << "Received message for 997" << endl;
					// add later
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
			
			if (!tempConfirmContinue(receiver))
			{
				break;
			}
		}		
	}
	
	else if (receiver.id == 2)
	{
		while (true)
		{
			// get 257
			// get 1097

			receiver.getEvent(997); // should go before if statement since both receive
				
			// send 1098
			// if message count is 5000
				// send 2 for sender 257
				// terminate
				
			if (!tempConfirmContinue(receiver))
			{
				cout << "You have chosen to quit" << endl;
				break;
			}
			
		}
	}
		
	if (!receiver.terminate())
	{
		cout << "Warning: unable to deallocate queue!\n";
		cout << "Press any key to exit program: ";
		cin.get();
	}
	
	return 0;
}
