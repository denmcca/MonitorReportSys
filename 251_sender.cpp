
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
using namespace std;

int randomDivisorNumber () {
	int event = 1;
	int number = 0;
	while(event != 0){
		number = rand();
		event = number%251;
	}
	return number;
}

int main() {
	srand(time(NULL)); // To generate random number
	int qid = msgget(ftok(".",'u'), 0); // generates a key value

	// declare my message buffer
	struct buf {
		long mtype; // required - lunix call that is required, uses msgsnd
		char greeting[50]; // mesg content
	};
	buf msg;
	int size = sizeof(msg)-sizeof(long);
	msg.mtype = 251; // mtype for 251 sender
	//msgbuf *exitmsg
	string exitMsg = "Terminating"; // exiting message
	//string * exitMsgPtr = (msgbuf*)&exitMsg;

//--------------------testing---------------
	strcpy(msg.greeting, exitMsg.c_str()); //converts string to char
	//get_info(int qid, msgbuf *exitmsg, int size, long mtype);
	get_info(qid, (struct msgbuf*)&msg, size, 251);

	// initialization handshaking to reciver1
	msg.mtype = 253;
	msgsnd(qid, (struct msgbuf *)&msg, size, 0); // sending number to receiver
	msgrcv(qid, (struct msgbuf *)&msg, size, 254, 0); // reading
	msgrcv(qid, (struct msgbuf *)&msg, size, 4, 0); // reading



	msg.mtype = 251; // mtype for 251 sender
	int event = 0; // number that will be sent to receiver
	while(true){
//------------stuck here need to fix the greeting

		event = randomDivisorNumber(); // number divisor by 251
		string message = to_string(event); //converting number to string
		strcpy(msg.greeting, message.c_str()); //converts string to char
		cout << "Sending message: " << getpid() << ": "  << msg.greeting << ": " << msg.mtype  << endl;
		msgsnd(qid, (struct msgbuf *)&msg, size, 0); // sending number to receiver
	}

  exit(0);
}
