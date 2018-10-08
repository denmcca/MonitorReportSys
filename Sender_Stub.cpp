#include "Sender.h"
#include <iostream>
#include "PTools.cpp"
#include <sys/ipc.h>
#include <sys/msg.h>
#include <string>
#include <cstring>
#include <stdlib.h>     
#include <time.h>       
#include <sys/types.h>
#include <unistd.h>

/*
	Using as test stub for testing with Receiver program. - Dennis 
	
	Ideas:
	-- How to manage deallocation of queue 
	Receiver 1 creates queue, receiver 2 sends special message to queue, 
	receiver 1 set to terminate, receiver 1 checks if queue is empty to
	deallocate queue, receiver 2 sets to terminate, receiver 2 checks if
	queue is empty, queue is not empty, receiver 2 retrieves any message,
	receiver 2 checks if queue is empty, queue is empty, receiver 2 
	terminates, receiver 1 deallocates queue then terminates.
	
	
	
*/

using namespace std;

const char* ftok_path = ".";
const int ftok_id = 'u';

Sender::Sender(int qidIn)
{
	//cout << "qidIn = " << qidIn << endl;
	qid = qidIn;
	marker = 251;
	MsgPigeon msgr;	// sends and gets values from queue
	
	receiverBit = 01; // for 997 where 1x is r1 and x1 is r2
	
	setMessage("");
	
	srand(time(NULL));
}

/*
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
	
	return choice;
}
*/

void Sender::setMessage(string msgIn)
{
	strcpy(msgr.message, msgIn.c_str());
}

int Sender::sendMessage(long mTypeIn) // mTypeIn incase need to send other than marker
{
	//cout << "sendMessage with mType " << mTypeIn << endl;
	msgr.mType = mTypeIn;
	
	msgsnd(qid, (struct msgbuf*)&msgr, msgr.getSize(), 0);
	
	// reset message
	strcpy(msgr.message, "");
	msgr.mType = 0;
}

void Sender::getMessage(long mTypeIn)
{
	//cout << "getMessage" << endl;
	
	msgr.mType = mTypeIn;
	
	//cout << "qid = " << qid << ", mTypeIn = " << mTypeIn << ", event = " << event << endl;
	
	msgrcv(qid, (struct msgbuf*)&msgr, msgr.getSize(), mTypeIn, 0);
	
	//cout << "Event received!" << endl;
	
	//cout << "Message = " << msgr.message << endl;
}

/*
bool Sender::terminate() // mType will be for termination calls
{
	cout << "terminate" << endl;
	// sets message to terminate when terminating.
	setMessage(MSG_TERM);
	
	//if 997 notify both receivers
	cout << "marker = " << marker << endl;
	
	if (marker == 997)
	{
	
		if (receiverBit % 10 == 1)
		{	
			receiverBit -= 1;
		}
		if (receiverBit >= 10)
		{
			receiverBit -= 10;
		}
	}
	
	return true;
}
*/

int Sender::generateRandomNumber() // how to generate 32 bit integers?
{
	event = rand();
	//cout << "random number generated = " << event << endl;
	return event;
}

bool Sender::processNumber() // modular event will be 118
{	
	if (event % marker == 0)
	{
		//string eventConv = std::to_string(event);
		strcpy(msgr.message, std::to_string(event).c_str());
		return true;
	}
	
	return false;
}

int main()
{		
	Sender sender(msgget(ftok(ftok_path,ftok_id),0));
	
	//* Testing 251 to receiver 1 with loop, and event generator
	
	int count = 0;
	sender.receiverBit = 1;
	
	sender.sendMessage(253); // initialization handshake
	sender.getMessage(254);
	sender.getMessage(4);
	
	while(sender.receiverBit > 0)
	{		
		sender.generateRandomNumber();
		
		if (sender.processNumber() && sender.receiverBit > 0)
		{	
			if (sender.receiverBit % 10 == 1)
			{
				sender.setMessage(std::to_string(sender.event));	
				sender.sendMessage(251);
				count++;
				//std::cout << "count = " << count << std::endl;
			}
		}
		
		if (count > 10000) // Used in place of kill command
		{
			//sender.getMessage(251);
			sender.setMessage("Terminating");
			sender.sendMessage(251);
			--sender.receiverBit;
		}
	}
	//*/	
		
	return 0;
}
