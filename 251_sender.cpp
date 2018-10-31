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

int randomDivisorNumber () {
	int event = 1;
	int number = 0;
	while(event != 0){
		number = rand();
		event = number%(251);
	}
	return number;
}

int main() {
	const int RECEIVER_ID = 9;
	srand(time(NULL)); // To generate random number
	int qid = msgget(ftok(".",'u'), 0); // generates a key value
	MsgPigeon msg; // The same struct message as receiver
	int size = sizeof(msg)-sizeof(long); // size the of message
	
	msg.message.srcID = 251; // mtype for 251 sender
	string exitMsg = "Terminating"; // exiting message
	strcpy(msg.message.message, exitMsg.c_str()); //converts string to char
	msg.mType = RECEIVER_ID;
	get_info(qid, (struct msgbuf*)&msg, size, RECEIVER_ID);
	
	msg.mType = 20;
	strcpy(msg.message.message, "Sender 251 Ready");
	msg.message.srcID = 251;
	msgsnd(qid, (struct msgbuf *)&msg, size, 0); // sending init call to receiver
	msgrcv(qid, (struct msgbuf *)&msg, size, 251, 0); // Starting message	
	
	int event = 0; // number that will be sent to receiver
	
	while(true){
		event = randomDivisorNumber(); // number divisor by 251
		string message = to_string(event); //converting number to string
		cout << "[pid " << getpid() << "] Sending message: " << message << endl;

		strcpy(msg.message.message, message.c_str()); //converts string to char		
		msg.mType = RECEIVER_ID;
		msg.message.srcID = 251;
		if (msgsnd(qid, (struct msgbuf *)&msg, size, 0) < 0) // sending number to receiver
		{
<<<<<<< HEAD
			cout << "Queue terminated prematurely!" << endl;
=======
			cout << "Queue terminated prematurely!" << endl << flush; //flushes if -1
>>>>>>> 727baec07174e375c954db5050d8989c4522b7e4
			return -1;
		}
	}
  exit(0);
}