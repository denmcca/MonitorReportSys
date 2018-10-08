#include <iostream>
#include "PTools.cpp"
#include <sys/ipc.h>
#include <sys/msg.h>
#include <cstring>
#include <stdlib.h>     
#include <time.h>       
#include <sys/types.h>
#include <unistd.h>
#include "MsgPigeon.cpp"
#include <string>
#include <sstream>

using namespace std;

const int MTYPE_TO_R1 = 997;
const int MTYPE_TO_R2 = 1097;
const int MTYPE_R2_POLL = 996;
const int MTYPE_FROM_R1 = 998;
const int MTYPE_FROM_R2 = 1098;

string intToString (int a)
{
	ostringstream temp;
	temp << a;
	return temp.str();
}

class Sender997
{
public:
	int qid;
	bool sendToR2; // initialized to "true", set to "false" after R2 terminates.
	
	Sender997();
	void initQID();
	int generateRandomNumber();
	void sendMessage(string msgContent, long mType);
	string getMessage(long mType);
	void runMainLoop();
};

Sender997::Sender997()
{
	qid = -1;
	sendToR2 = true;
	srand(time(NULL));
}

int Sender997::generateRandomNumber()
{
	return rand();
}

void Sender997::sendMessage(string msgContent, long mType)
{
	cout << "Sending message '" << msgContent << "' to mType = " << mType << endl;
	MsgPigeon msg;
	msg.mType = mType;
	strcpy(msg.message, msgContent.c_str());
	msgsnd(qid, (struct MsgPigeon *)&msg, msg.getSize(), 0);
}

string Sender997::getMessage(long mType)
{
	cout << "Waiting for message from mType = " << mType << endl;
	MsgPigeon msg;
	msg.mType = mType;
	msgrcv(qid, (struct msgbuf*)&msg, msg.getSize(), mType, 0);
	return msg.message;
}

void Sender997::initQID()
{
	const char* ftok_path = ".";
	const int ftok_id = 'u';
	while (true)
	{
		cout << ".";
		if (qid == -1)
		{
			qid = msgget(ftok(ftok_path,ftok_id),0);
			cout << "qid = " << qid << endl;
		}
		else
		{
			cin.get();
			cout << "qid = " << qid << endl;
			cout << "997 found the queue! Now ready! " << endl;
			cout << "qid = " << qid << endl;
			break;
		}
	}
}

void Sender997::runMainLoop()
{
	while (true)
	{
		// Generate and process random number
		int randInt = generateRandomNumber();
		if (randInt < 100)
		{
			sendMessage("Terminating", MTYPE_TO_R1);
			if (sendToR2) sendMessage("Terminating", MTYPE_TO_R2);
			cout << "Sender 997 terminated" << endl;
			return;
		}
		else if ((randInt % 997) == 0)
		{
			string msgContent = intToString(randInt);
			
			sendMessage(msgContent, MTYPE_TO_R1);

			// Get ackowledgement from R1
			getMessage(MTYPE_FROM_R1);
			
			if (sendToR2)
			{
				sendMessage("Polling", MTYPE_R2_POLL);	
				sendMessage(msgContent, MTYPE_TO_R2);

				// Get ackowledgement from R2
				getMessage(MTYPE_FROM_R2);

				// Get poll message from queue
				string response = getMessage(MTYPE_R2_POLL);
				if (response == "Terminating")
				{
					sendToR2 = false;
				}
			}
		}
	}	
}

int main()
{
	Sender997 snd;

	snd.initQID();

	snd.sendMessage("", 999); // initialization handshake
	snd.sendMessage("", 1099);
	snd.getMessage(1000);
	snd.getMessage(1100);
	snd.getMessage(4);
	
	snd.runMainLoop();
}
