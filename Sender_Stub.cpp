#include "Sender.h"
#include <iostream>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <string>
#include <cstring>
#include <stdlib.h>     
#include <time.h>       
#include <sys/types.h>
#include <unistd.h>
#include "get_info.h"

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
	srand(time(NULL));
	qid = qidIn;
	marker = 251;
	setMessage("NO MESSAGE YET");
	msgr.mType = 0;
	isConnected = true;	
}

void Sender::setMessage(string msgIn)
{
	strcpy(msgr.message, msgIn.c_str());
}

int Sender::sendMessage(long mTypeIn) // mTypeIn incase need to send other than marker
{
	cout << "sendMessage with mType " << mTypeIn << endl;
	msgr.mType = mTypeIn;
	
	if (msgsnd(qid, (struct msgbuf*)&msgr, msgr.getSize(), 0) == -1)
	{
		std::cout << "Message Queue has been Deallocated Prematurely! ";
		cin.get();
	}
	
	// reset message
	strcpy(msgr.message, "");
	msgr.mType = 0;
}

void Sender::getMessage(long mTypeIn)
{
	//cout << "getMessage" << endl;
	
	msgr.mType = mTypeIn;
	
	cout << "qid = " << qid << ", mTypeIn = " << mTypeIn << ", event = " << event << endl;
	
	msgrcv(qid, (struct msgbuf*)&msgr, msgr.getSize(), mTypeIn, 0);
	
	//cout << "Event received!" << endl;
	
	//cout << "Message = " << msgr.message << endl;
}

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

int Sender::getMessageSize()
{
	return msgr.getSize();
}

int main()
{		
	Sender sender(msgget(ftok(ftok_path,ftok_id),0));
	//* Testing 251 to receiver 1 with loop, and event generator
	
	int count = 0;
	
	////////////////////////////////////////////////////
	// Testing patch
				std::cout << "Here 0" << endl;
		std::cout << "sender.exitMsgPtr.message: " << sender.exitMsgPtr.message << endl;

	strcpy(sender.exitMsgPtr.message, "Terminating");
	sender.exitMsgPtr.mType = 251;
	get_info(sender.qid, (struct msgbuf*)&(sender.exitMsgPtr), sender.getMessageSize(), 251);	
			std::cout << "Here" << endl;
		std::cout << "sender.exitMsgPtr.message: " << sender.exitMsgPtr.message << endl;
		std::cout << "&(sender.exitMsgPtr.message): " << &(sender.exitMsgPtr.message) << endl;	
	
	//*///////////////////////////////////////////////////
		
	// initialization handshaking
	sender.sendMessage(253);
	sender.getMessage(254);
	sender.getMessage(4);

	
	//cout<<sender.exitMsgPrt<< " " << sender.getMessageSize()<<"<<<<<"<<endl;
	
	
	
	while(sender.isConnected)
	{		
		sender.generateRandomNumber();
		
		if (sender.processNumber())
		{	

			sender.setMessage(std::to_string(sender.event));	
			sender.sendMessage(251);
			count++;
			//std::cout << "count = " << count << std::endl;
			///// Used in place of kill -10 command
			
			std::cout << "Here 2" << endl;
			std::cout << "sender.exitMsgPtr.message: " << sender.exitMsgPtr.message << endl;
			//std::cout << "&(sender.exitMsgPtr.message): " << &(sender.exitMsgPtr.message) << endl;
			////

		/*///////
			if (count >= 100000 && true)
			{
				//sender.getMessage(251);
				sender.setMessage("Terminating");
				sender.sendMessage(251);
				sender.isConnected = false;
				//cout << "Count " << count << endl;
			}
		/*///	
			//*/	
		}
		//std::cout << "exitMsgPtr->message: " << exitMsgPtr->message << std::endl;
		///if (exitMsgPtr.message == "Terminate")		
	}
	
	// Shutdown confirmation
	sender.setMessage("251 Shutting down."); 
	sender.sendMessage(255);
	//*/	
		
	return 0;
}
