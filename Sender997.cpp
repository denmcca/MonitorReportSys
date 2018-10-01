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

const char* ftok_path = ".";
const int ftok_id = 'u';
int qid = msgget(ftok(ftok_path,ftok_id),0);

const int MTYPE_TO_R1 = 997;
const int MTYPE_TO_R2 = 1097;
const int MTYPE_FROM_R1 = 998;
const int MTYPE_FROM_R2 = 1098;

int generateRandomNumber()
{
	return rand();
}

string intToString (int a)
{
	ostringstream temp;
	temp << a;
	return temp.str();
}

void sendMessage(string msgContent, long mType)
{
	cout << "Sending message '" << msgContent << "' to mType = " << mType << endl;
	MsgPigeon msg;
	msg.mType = mType;
	strcpy(msg.message, msgContent.c_str());
	msgsnd(qid, (struct MsgPigeon *)&msg, msg.getSize(), 0);
}


string getMessage(long mType)
{
	cout << "Waiting for message from mType = " << mType << endl;
	MsgPigeon msg;
	msg.mType = mType;
	msgrcv(qid, (struct msgbuf*)&msg, msg.getSize(), mType, 0);
	return msg.message;
}

void initQID()
{
	while (true)
	{
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

int main()
{
	srand(time(NULL));

	initQID();

	while (true)
	{
		int randInt = generateRandomNumber();
		if (randInt < 100)
		{
			sendMessage("Terminated", MTYPE_TO_R1);
			sendMessage("Terminated", MTYPE_TO_R2);
			cout << "Sender 997 terminated" << endl;
			return 0;
		}
		else if ((randInt % 997) == 0)
		{
			string msgContent = intToString(randInt);
			//cout << "Sending message " << msgContent << " to R1" << endl;
			sendMessage(msgContent, MTYPE_TO_R1);
			//cout << "Sending message " << msgContent << " to R2" << endl;
			sendMessage(msgContent, MTYPE_TO_R2);

			// get ackowledgements
			getMessage(MTYPE_FROM_R1);
			getMessage(MTYPE_FROM_R2);
		}
	}
}
