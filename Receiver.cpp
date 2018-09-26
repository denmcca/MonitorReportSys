#include "Receiver.h"
#include <iostream>
#include "PTools.cpp"
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/types.h>
#include <unistd.h>

const char* ftok_path = ".";
const int ftok_id = 'u';

using namespace std;

Receiver::Receiver(int qidIn)
{
	qid = qidIn;
	id = assignReceiverNumber();
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
			return choice;
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
	
	if (msgrcv(qid, (struct msgbuf*)&msgr, msgr.getSize(), mTypeIn, 1) != -1)
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

bool Receiver::terminate()
{
	return true;
}

int main()
{
	char checkQueue;
	Receiver receiver(msgget(ftok(ftok_path,ftok_id),IPC_CREAT|0600)); // Removed IPC_EXCL since Q must be shared
	
	//cout << "getpid() =" << getpid() << endl; // does not work!
		
	while (true)
	{
		if (receiver.id == 1)
		{
			receiver.getEvent(997);
		}
		
		else if (receiver.id == 2)
		{
			receiver.getEvent(997); // should go before if statement since both receive
		}
		cout << "receiver.qid = " << receiver.qid << endl;
		
		cout << "Do you want to check queue again? (y/n): ";
		cin >> checkQueue;
		
		if (checkQueue != 'y' & checkQueue != 'Y')
		{
			break;
		}

	}
	
	msgctl(receiver.qid, IPC_RMID, NULL);
	
	return 0;
}
