#include <iostream>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/types.h>
#include <stdio.h>
#include <string>
#include <cstring>
#include <random>
#include "MsgPigeon.cpp"

// Getting message queue ID
int qid = msgget(ftok(".",'u'), 0);

const int MSG_SIZE = sizeof(MsgPigeon) - sizeof(long);
const std::string ALIVE_MSG = "KeepAlive";
const std::string TERM_MSG = "Terminating";
const int ALIVE_ID = 256;
const int ID = 257;
const int RID = 20;

// Sends message to queue using system call.
void sendMessage(std::string msgContent, long mType)
{
	MsgPigeon msg;
	msg.mType = mType;
	msg.message.srcID = ID;
	strcpy(msg.message.message, msgContent.c_str());
	msgsnd(qid, (struct MsgPigeon *)&msg, MSG_SIZE, 0);
}

// Gets message from queue using system call.
void getMessage(long mType)
{
	MsgPigeon msg;
	msgrcv(qid, (struct msgbuf *)&msg, MSG_SIZE, mType, 0); // read mesg
}

// Checks polling message if terminating message from receiver has been set.
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

// Programs main point of entry.
int main()
{
	std::cout << "Starting sender 257. . ." << std::endl;
	std::srand(time(NULL));

	// Send message to Receiver 2 to notify presence.
	MsgPigeon msg;
	msg.mType = RID;
	strcpy(msg.message.message, "Sender 257 Ready");
	msg.message.srcID = ID;
	std::cout << "Waiting for message queue...\n" << std::flush;
	while(msgsnd(qid, (struct msgbuf *)&msg, MSG_SIZE, 0) < 0);

	// Waiting for response from Receiver 1.
	msgrcv(qid, (struct msgbuf *)&msg, MSG_SIZE, ID, 0); // Start message

	// Sending preliminary poll message
	sendMessage(ALIVE_MSG, ALIVE_ID);

	// Main program loop.
	while(true) // Moved checkAlive into loop for efficiency.
	{
		int random = std::rand();
		if (random % ID == 0)
		{
			std::cout << "Event found: " << random << std::endl << std::flush;
			sendMessage(std::to_string(random), RID);

			// After event sent check to see if polling message is terminating.
			if (!checkAlive())
			{
				// If poll message is terminate then send terminate message back
				sendMessage(TERM_MSG, RID);
				// Exit loop
				break;
			}
   	}
	}
	std::cout << "Sender 257 has finished!\n";
}
