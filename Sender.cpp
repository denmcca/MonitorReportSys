#include "Sender.h"
#include <iostream>
#include "PTools.cpp"
#include <sys/ipc.h>
#include <sys/msg.h>
#include <cstring>
#include <stdlib.h>     
#include <time.h>       
#include <sys/types.h>
#include <unistd.h>

using namespace std;

const char* ftok_path = ".";
const int ftok_id = 'u';

Sender::Sender(int qidIn)
{
	cout << "qidIn = " << qidIn << endl;
	qid = qidIn;
	marker = numList[assignNumber()];
	MsgPigeon msgr;	// sends and gets values from queue
	
	strcpy(eventMsg, "Hello World");
	
	srand(time(NULL));
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

void Sender::sendEvent(long mTypeIn)
{
	cout << "sendEvent" << endl;
	
	cout << "qid = " << qid << ", mTypeIn = " << mTypeIn << ", event = " << event << endl;
	
	strcpy(msgr.message, "Hello world!");
	
	cout << "msgr.message = " << msgr.message << endl;
	
	//cout << "(struct msgbuf*)&msgr->message =" << (struct msgbuf*)&msgr->message << endl;
	cout << "msgr.getSize() = " << msgr.getSize() << endl;
	
	
	msgr.mType = mTypeIn;
	msgsnd(qid, (struct msgbuf*)&msgr, msgr.getSize(), 0);
}

bool Sender::terminate() // mType will be 122 for termination calls
{
	//if 997 notify both receivers
	if (marker == 997)
	{
		sendEvent(900);
	}
	
	//if 251 notify receiver 1
	
	//if 257 do not notify any receivers
	
	//somehow exit program
	return true;
}

int Sender::generateRandomNumber() // how to generate 32 bit integers?
{
	event = rand();
	return event;
}

bool Sender::processNumber() // modular event will be 118
{	
	if (event % marker == 0)
	{
		//string eventConv = std::to_string(event);
		strcpy(msgr.message, std::to_string(event).c_str());
		sendEvent(118);	
		return true;
	}
	
	return false;
}

int main()
{
	Sender sender(msgget(ftok(ftok_path,ftok_id),0));	 // qid = 0
	
	//cout << "getpid() =" << getpid() << endl; // does not work!

	while (true)
	{
		if (sender.qid != -1)
		{
			sender.qid = msgget(ftok(ftok_path,ftok_id),0);
			cout << "sender.qid = " << sender.qid << endl;
			//sender.sendEvent(118);
			//cin.get();
			break;			
		}
		else
		{
			cout << "sender.qid = " << sender.qid << endl;

		}
	}
	
	while (true)
	{
		sender.generateRandomNumber();	
		sender.processNumber();	
	
		if (sender.marker == 997)
		{
			if (sender.event < 100)
			{
				//sender.terminate(); // function that initiates terminating process.
			}
			
			sender.sendEvent(997);
		}
		
		cin.get();
	}
	
	cout << sender.marker << " found the queue! Now ready! " << endl;
	
	cout << "qid = " << sender.qid << endl;
	
	cin.get();
	
	return 0;
}
