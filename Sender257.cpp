#include <iostream>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/types.h>
#include <stdio.h>
#include <string>
#include <cstring>
#include <random>
#include "MsgPigeon.cpp"

int qid = msgget(ftok(".",'u'), 0);

const int MSG_SIZE = sizeof(MsgPigeon) - sizeof(long);
const std::string ALIVE_MSG = "KeepAlive";
const std::string TERM_MSG = "Terminating";
const int ALIVE_ID = 256;
const int ID = 257;
const int RID = 20;

//Function that sends a message with specified content and mtype to the message queue
void sendMessage(std::string msgContent, long mType)
{
	MsgPigeon msg;
	msg.mType = mType;
	msg.message.srcID = ID;
	strcpy(msg.message.message, msgContent.c_str());
	msgsnd(qid, (struct MsgPigeon *)&msg, MSG_SIZE, 0);
	//printf("Sending %s to channel %d\n",msg.message.message, msg.mType);
}

//Function to read a single message of an mtype from the message queue
void getMessage(long mType)
{
	MsgPigeon msg;
	msgrcv(qid, (struct msgbuf *)&msg, MSG_SIZE, mType, 0); // read mesg
}

//Function that leaves a message in the queue that determines whether Sender257 should exit or not
//If the message is modified by Receiver 2, it will return false. If it is not modified, the message
//is put back into the queue and the function returns true.
bool checkAlive()
{
	MsgPigeon msg;
	msgrcv(qid, (struct msgbuf *)&msg, MSG_SIZE, ALIVE_ID, 0); // read mesg

	if (strcmp(msg.message.message, ALIVE_MSG.c_str()) == 0)
	{
		sendMessage(ALIVE_MSG, ALIVE_ID);
		return true;
	}

	return false;
}


int main()
{
	std::cout << "Starting sender 257. . ." << std::endl;
	std::srand(time(NULL));

	MsgPigeon msg;	
	msg.mType = RID;
	strcpy(msg.message.message, "Sender 257 Ready");
	msg.message.srcID = ID;
	msgsnd(qid, (struct msgbuf *)&msg, MSG_SIZE, 0); // sending init call to receiver

	msgrcv(qid, (struct msgbuf *)&msg, MSG_SIZE, ID, 0); // Start message

	sendMessage(ALIVE_MSG, ALIVE_ID);

	while(true) // checkAlive when event found instead
	{
		int random = std::rand(); //Generate a random number representing an 'event' and check if it is divisible by 257
		if (random % ID == 0)
		{
			//If an event is found, report it
			std::cout << "Event found: " << random << std::endl;
			sendMessage(std::to_string(random), RID);            
			if (!checkAlive()) //Call checkAlive to see if Receiver 2 is still around. If it isn't, exit sender 257
			{
				sendMessage(TERM_MSG, RID);
				break;
			}
   		}
	}
}
