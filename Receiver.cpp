#include "Receiver.h"
#include <iostream>
#include "PTools.cpp"
#include <sys/ipc.h>
#include <sys/msg.h>

const char* ftok_path = ".";
const int ftok_id = 'w';

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

bool Receiver::terminate()
{
	return true;
}

int main()
{
	Receiver receiver(msgget(ftok(ftok_path,ftok_id),IPC_EXCL|IPC_CREAT|0600));
		
	while (true)
	{
		cout << "receiver.qid = " << receiver.qid << endl;
		cin.get();
		receiver.getEvent(100);
	}
	
	return 0;
}
