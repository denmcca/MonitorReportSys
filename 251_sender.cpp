#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <cstring>
#include <iostream>
#include <unistd.h>
#include <sys/wait.h>
#include <cstdlib>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "get_info.h"
#include <string>
#include "MsgPigeon.cpp"

using namespace std;

// Finds number divisible by 251.
int randomDivisorNumber () {
	int event = 1;
	int number = 0;

	// If number generated isn't modular to 251 then try again.
	while(event != 0){
		number = rand();
		event = number%(251);
	}

	// Return number modular to 251.
	return number;
}

// Programs main point of entry.
int main() {
	const int RECEIVER_ID = 1;
	srand(time(NULL)); // To generate random number
	int qid = msgget(ftok(".",'u'), 0); // generates a key value

	// Instantiates messenger object.
	MsgPigeon msg;
	// Gets size of messenger object for system calls.
	int size = sizeof(msg)-sizeof(long);

	// Sends ready message to Receiver 2.
	msg.mType = 20; // Receiver 2's mType is 20.
	strcpy(msg.message.message, "Sender 251 Ready");
	msg.message.srcID = 251;
	cout << "Waiting for message queue...\n" << flush;
	while (msgsnd(qid, (struct msgbuf *)&msg, size, 0) < 0); // sending init call to receiver

	// Awaiting start message from Receiver 1.
	msgrcv(qid, (struct msgbuf *)&msg, size, 251, 0); // Starting message

	// Setting up terminating message to be sent when kill -10 command is called.
	msg.message.srcID = 251; // mtype for 251 sender
	string exitMsg = "Terminating"; // exiting message
	strcpy(msg.message.message, exitMsg.c_str()); //converts string to char
	msg.mType = RECEIVER_ID;
	get_info(qid, (struct msgbuf*)&msg, size, RECEIVER_ID);

	int event = 0; // number that will be sent to receiver

	// Program loop.
	while(true){
		event = randomDivisorNumber(); // number divisor by 251

		string message = to_string(event); //converting number to string
		cout << "[pid " << getpid() << "] Sending message: " << message << endl;

		strcpy(msg.message.message, message.c_str()); //converts string to char
		msg.mType = RECEIVER_ID;
		msg.message.srcID = 251;

		// Sends event to message queue for Receiver 1.
		if (msgsnd(qid, (struct msgbuf *)&msg, size, 0) < 0) // sending number to receiver
		{
			cout << "Queue terminated prematurely!" << endl;
			return -1;
		}
	}
  exit(0);
}
