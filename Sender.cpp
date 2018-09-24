#include "Sender.h"
#include <iostream>
#include "PTools.cpp"
#include <sys/ipc.h>
#include <sys/msg.h>
#include <cstring>

using namespace std;

const char* ftok_path = ".";
const int ftok_id = 'w';

Sender::Sender(int qidIn)
{
	cout << "qidIn = " << qidIn << endl;
	qid = qidIn;
	marker = numList[assignNumber()];
	MsgPigeon msgr;	// sends and gets values from queue
}

int Sender::assignNumber()
{
	cout << "Which number do you want to assign to this sender's marker value?\n";
	
	for (int i = 0; i < numListSize; i++)
	{
		cout << i << ": " << numList[i] << endl;
	}
	
	return getMarkerNumber();
}

int Sender::getMarkerNumber()
{
	int choice;
	
	while(true)
	{
		try
		{
		
			cout << "Enter number: ";
			cin >> choice;
			if (!cin.good() | choice < 0 | choice > numListSize - 1)
				throw int();
			break;
		} 
		catch (int e)
		{
			PTools::flushCin();
			cout << "Invalid selection!" << endl;
		}
	}
}

void Sender::sendEvent(int eventIn, long mTypeIn)
{
	cout << "sendEvent" << endl;
	string eventConv = std::to_string(eventIn);
	strcpy(msgr.message, eventConv.c_str());
	msgr.mType = mTypeIn;
	msgsnd(qid, (struct msgbuf*)&msgr, msgr.getSize(), 0);
}

bool Sender::terminate()
{
	return true;
}

int main()
{
	Sender sender(msgget(ftok(ftok_path,ftok_id),0));	 // qid = 0

	while (true)
	{
		if (sender.qid != -1)
		{
			sender.qid = msgget(ftok(ftok_path,ftok_id),0);
			cout << "sender.qid = " << sender.qid << endl;
			sender.sendEvent(300, 100);
			cin.get();			
		}
		else
		{
			cout << "sender.qid = " << sender.qid << endl;

		}
	}
	
	cout << sender.marker << " found the queue! Now ready! " << endl;
	
	cout << "qid = " << sender.qid << endl;
	
	cin.get();
	
	return 0;
}
