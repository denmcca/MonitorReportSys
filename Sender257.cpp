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
//int qid = msgget(ftok(".",'u'), IPC_EXCL|IPC_CREAT|0600); //temp statement to create msg queue

const int MSG_SIZE = sizeof(MsgPigeon) - sizeof(long);
const std::string ALIVE_MSG = "KeepAlive";
const std::string TERM_MSG = "Terminating";
const int ALIVE_ID = 256;
const int ID = 257;
const int RID = 20;

void sendMessage(std::string msgContent, long mType)
{
	MsgPigeon msg;
	msg.mType = mType;
	msg.message.srcID = ID;
	strcpy(msg.message.message, msgContent.c_str());
	msgsnd(qid, (struct MsgPigeon *)&msg, MSG_SIZE, 0);
}

void getMessage(long mType)
{
	MsgPigeon msg;
	msgrcv(qid, (struct msgbuf *)&msg, MSG_SIZE, mType, 0); // read mesg
}

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
		int random = std::rand();
		if (random % ID == 0)
		{
			std::cout << "Event found: " << random << std::endl;
			sendMessage(std::to_string(random), RID);            
			if (!checkAlive())
			{
				sendMessage(TERM_MSG, RID);
				break;
			}
   		}
	}
}
