#include <iostream>
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

const int ID = 997;
const int ACK_ID = 998;
const int R1_ID = 1;
const int R2_ID = 2;

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
	bool sendToR1;
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
	cout << "Sending '" << msgContent << "' to Receiver ";
	if (mType == R1_ID) cout << "1\n";
	else cout << "2\n";
		
	MsgPigeon msg;
	msg.mType = mType;
	msg.message.srcID = ID;
	strcpy(msg.message.message, msgContent.c_str());
	if(msgsnd(qid, (struct MsgPigeon *)&msg, msg.getSize(), 0) < 0)
		throw int(-10);
}

string Sender997::getMessage(long mType)
{
	cout << "Awaiting Acknowledgement from Receiver." << endl;
	
	MsgPigeon msg;
	if (msgrcv(qid, (struct msgbuf*)&msg, msg.getSize(), mType, 0) < 0)
		throw int(-11);
	cout << "Acknowledgement received from Receiver ";
	if (msg.message.srcID == R1_ID) cout << R1_ID;
	else cout << R2_ID;
	cout << "!" << endl;
	
	if (strcmp(msg.message.message, "Terminating") == 0)
	{
		cout << "Received Terminating message" << endl;
		sendToR2 = false;
	}
		
	return msg.message.message;
}

void Sender997::initQID()
{
	const char* ftok_path = ".";
	const int ftok_id = 'u';
	while (true)
	{
		cout << ".";
		if (qid == -1)	qid = msgget(ftok(ftok_path,ftok_id),0);
		else
		{
			cout << "997 found the queue! Now ready! " << endl;
			break;
		}
	}
}

void Sender997::runMainLoop()
{
	MsgPigeon msg;	
	msg.mType = R2_ID;
	strcpy(msg.message.message, "Sender 997 Ready");
	msg.message.srcID = ID;
	msgsnd(qid, (struct msgbuf *)&msg, msg.getSize(), 0); // sending init call to receiver	
	msgrcv(qid, (struct msgbuf *)&msg, msg.getSize(), ID, 0); // Starting message	

	while (true)
	{
		// Generate and process random number
		int randInt = generateRandomNumber();
		if (randInt < 100)
		{
			sendMessage("Terminating", R1_ID);
			if (sendToR2) sendMessage("Terminating", R2_ID);
			cout << "Sender 997 terminated: " << randInt << endl;
			return;
		}
		else if ((randInt % 997) == 0)
		{
			string msgContent = intToString(randInt);	
			sendMessage(msgContent, R1_ID);
			// Get ackowledgement from R1
			getMessage(ACK_ID);			
			
			if (sendToR2)
			{
				// Send event to R2	
				sendMessage(msgContent, R2_ID);
				getMessage(ACK_ID);				
			}
		}
	}	
}

int main()
{
	Sender997 snd;
	snd.initQID();
	
	try 
	{
		snd.runMainLoop();
	}
	catch (int err)
	{
		if (err == -10)
			cout << "getMessage error!" << endl << flush;
		if (err == -11)
			cout << "sendMessage error!" << endl << flush;
	}
}
