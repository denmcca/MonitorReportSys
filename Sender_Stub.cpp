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

/*
	Using as test stub for testing with Receiver program. - Dennis 
*/

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
	
	return choice;
}

void Sender::sendEvent(long mTypeIn) // mTypeIn incase need to send other than marker
{
	cout << "sendEvent" << endl;
	
	cout << "qid = " << qid << ", mTypeIn = " << mTypeIn << ", event = " << event << endl;
	
	cout << "msgr.message = " << msgr.message << endl;
	
	cout << "msgr.getSize() = " << msgr.getSize() << endl;
	
	if (msgr.message == "Terminating")
	{
		terminate();
	}
	
	else
	{
		
		msgr.mType = mTypeIn;
		msgsnd(qid, (struct msgbuf*)&msgr, msgr.getSize(), 0);
	
		strcpy(msgr.message, "");
		msgr.mType = 0;
	}
}

void Sender::getEvent(long mTypeIn)
{
	cout << "getEvent" << endl;
	
	cout << "qid = " << qid << ", mTypeIn = " << mTypeIn << ", event = " << event << endl;
	
	msgrcv(qid, (struct msgbuf*)&msgr, msgr.getSize(), mTypeIn, 0);
	
	cout << "Event received!" << endl;
	
	cout << "Message = " << msgr.message << endl;
}

bool Sender::terminate() // mType will be 122 for termination calls
{
	//if 997 notify both receivers
	if (marker == 997)
	{
		if (receiverBit % 10 == 1)
		{	
			sendEvent(marker + 1);
		}
		if (receiverBit >= 10)
		{
			sendEvent(marker + 101);
		}
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
		return true;
	}
	
	return false;
}

bool tempConfirmContinue(Sender sendIn)
{
	cout << "tempConfirmContinue" << endl;
	
	char checkQueue; // used to control loop temporarily.

	cout << "qid = " << sendIn.qid << endl;
	
	cout << "Do you want to send to queue again? (y/n): ";
	cin >> checkQueue;
	
	if (checkQueue != 'y' & checkQueue != 'Y')
	{
		return false;
	}
	
	return true;
}

int main()
{
	Sender sender(msgget(ftok(ftok_path,ftok_id),0));	
	
	/* Testing 251 mType
	sender.msgr.mType = 251;
	strcpy(sender.msgr.message, "TESTING");
	sender.sendEvent(sender.marker); // working
	*/	
	
	/* Testing 251 mType with Termination message
	sender.msgr.mType = 251;
	strcpy(sender.msgr.message, "Terminating");
	sender.sendEvent(sender.marker); // working
	//*/
	
	/* Testing 997 to receiver 1 with acknowledgement
	sender.msgr.mType = 997;
	strcpy(sender.msgr.message, "Hello from 997");
	sender.sendEvent(sender.marker);
	sender.getEvent(sender.marker + 1); // working
	//*/	
	
	/* Testing 997 to receiver 2 with acknowledgement
	sender.msgr.mType = 1097;
	strcpy(sender.msgr.message, "Terminating");
	sender.sendEvent(1097);
	sender.getEvent(sender.marker + 101); // working
	//*/
	
	/* Testing 997 termination to both receivers
	sender.msgr.mType = 997;
	strcpy(sender.msgr.message, "Terminating");
	sender.sendEvent(sender.marker);
	sender.getEvent(sender.marker + 1);
	  
	sender.msgr.mType = 1097;
	strcpy(sender.msgr.message, "Terminating");
	sender.sendEvent(1097);
	sender.getEvent(sender.marker + 101); // working
	//*/
	
	//* Testing 997 events to termination loop to both receivers
	int test_loop_counter = 0;
	
	while(true)
	{		
		cout << "test_loop_counter = " << test_loop_counter << endl;
		
		sender.generateRandomNumber();
		
		if (sender.processNumber())
		{
			sender.sendEvent(sender.marker);
			sender.getEvent(sender.marker + 1);
			
			sender.sendEvent(sender.marker + 100);
			sender.getEvent(sender.marker + 101);
		}
		
		if (test_loop_counter++ == 1000)
		{
			sender.msgr.mType = 997;
			strcpy(sender.msgr.message, "Terminating");
			sender.sendEvent(sender.marker);
			sender.getEvent(sender.marker + 1);
			  
			sender.msgr.mType = 1097;
			strcpy(sender.msgr.message, "Terminating");
			sender.sendEvent(1097);
			sender.getEvent(sender.marker + 101);
			
			// need final acknowledgement from sender
			
			sender.msgr.mType = 997;
			strcpy(sender.msgr.message, "Goodbye, #1!");
			sender.sendEvent(sender.marker);
			  
			sender.msgr.mType = 1097;
			strcpy(sender.msgr.message, "Goodbye, #2!");
			sender.sendEvent(1097);
			
			break;
			
		}
	}
	//*/
		
	return 0;
}
