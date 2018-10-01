
#include <iostream>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/types.h>
#include <string>
#include <cstring>
#include <random>
#include "MsgPigeon.cpp"

//int qid = msgget(ftok(".",'u'), 0);
int qid = msgget(ftok(".",'u'), IPC_EXCL|IPC_CREAT|0600); //temp statement to create msg queue

const int MSG_SIZE = sizeof(MsgPigeon) - sizeof(long);
const std::string ALIVE_MSG = "KeepAlive";

void sendMessage(std::string msgContent, long mType)
{
    MsgPigeon msg;
	//int size = sizeof(MsgPigeon) - sizeof(long);
    msg.mType = mType;
    strcpy(msg.message, msgContent.c_str());
    msgsnd(qid, (struct MsgPigeon *)&msg, MSG_SIZE, 0);
}

bool checkAlive()
{
    MsgPigeon msg;
    msgrcv(qid, (struct msgbuf *)&msg, MSG_SIZE, 258, 0); // read mesg
    if (ALIVE_MSG.compare(msg.message) == 0)
    {
        sendMessage(ALIVE_MSG, 258);
        return true;
    }
    return false;
}

int main()
{
    std::cout << "Starting sender 257. . ." << std::endl;
    std::srand(time(NULL));
    sendMessage(ALIVE_MSG, 258);
    //int qid = msgget(ftok(".",'u'), 0);

    while(checkAlive())
    {
        int random = std::rand();
        if (random % 257 == 0)
        {
            std::cout << "Event found: " << random << std::endl;
            sendMessage(std::to_string(random), 257);
        }
        
    }
    //Temporary remove for creating the message queue
    msgctl (qid, IPC_RMID, NULL);
}
