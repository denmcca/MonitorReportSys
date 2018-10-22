
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <cstring>
#include <iostream>
#include <unistd.h>
#include <sys/wait.h>
#include <cstdlib>
using namespace std;

//251 receiver

int main() {

  // create my msgQ with key value from ftok()
	int qid = msgget(ftok(".",'u'), IPC_EXCL|IPC_CREAT|0600);

	// declare my message buffer
	struct buf {
		long mtype; // required
		char greeting[50]; // mesg content
	};

	buf msg;
	int size = sizeof(msg)-sizeof(long);

	msgrcv(qid,(struct msgbuf *)&msg,size,251,0);
	cout<<msg.greeting<<endl;

	//test receiver
	//int count = 0;
	while (true){
		msgrcv(qid, (struct msgbuf *)&msg, size, 251, 0); // read mesg
		cout << getpid()<< ": " << msg.greeting << endl;
		//count++;
	}

	// now safe to delete message queue
	msgctl (qid, IPC_RMID, NULL);
  exit(0);
}
