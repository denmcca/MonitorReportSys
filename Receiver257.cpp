
#include <iostream>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/types.h>
#include <string>
#include <cstring>
#include "MsgPigeon.cpp"

int qid = msgget(ftok(".",'u'), IPC_EXCL|IPC_CREAT|0600); //temp statement to create msg queue

const int MSG_SIZE = sizeof(MsgPigeon) - sizeof(long);
const std::string KILL_MSG = "Terminating";
const int ALIVE_ID = 256;

void sendMessage(std::string msgContent, long mType)
{
    MsgPigeon msg;
	//int size = sizeof(MsgPigeon) - sizeof(long);
    msg.mType = mType;
    strcpy(msg.message, msgContent.c_str());
    msgsnd(qid, (struct MsgPigeon *)&msg, MSG_SIZE, 0);
}

int main()
{
    int count = 5000;
    std::cout << "Starting receiver 257 test program. . ." << std::endl;

    while(count > 0)
    {
        MsgPigeon msg;
        msgrcv(qid, (struct msgbuf *)&msg, MSG_SIZE, 257, 0); // read msg
        std::cout << "Message: " << msg.message << std::endl;
        count--;
    }

    MsgPigeon msg;
    msgrcv(qid, (struct msgbuf *)&msg, MSG_SIZE, ALIVE_ID, 0); // read terminate msg to remove it
    sendMessage("Terminate", ALIVE_ID); //Send terminate message to sender 257
    std::cout << "Terminating . . . " << std::endl;
    msgctl (qid, IPC_RMID, NULL);
}
