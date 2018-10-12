
#include <iostream>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/types.h>
#include <string>
#include <cstring>
#include <random>
#include "MsgPigeon.cpp"

int qid = msgget(ftok(".",'u'), 0);
//int qid = msgget(ftok(".",'u'), IPC_EXCL|IPC_CREAT|0600); //temp statement to create msg queue

const int MSG_SIZE = sizeof(MsgPigeon) - sizeof(long);
const std::string ALIVE_MSG = "KeepAlive";
const int ALIVE_ID = 256;

void sendMessage(std::string msgContent, long mType)
{
	std::cout << "sendMessage" << std::endl;
	std::cout << "mType " << mType << std::endl;
    MsgPigeon msg;
	//int size = sizeof(MsgPigeon) - sizeof(long);
    msg.mType = mType;
    strcpy(msg.message, msgContent.c_str());
    msgsnd(qid, (struct MsgPigeon *)&msg, MSG_SIZE, 0);
}

void getMessage(long mType)
{
	std::cout << "getMessage" << std::endl;
	std::cout << "mType " << mType << std::endl;
	MsgPigeon msg;
	msgrcv(qid, (struct msgbuf *)&msg, MSG_SIZE, mType, 0); // read mesg
	std::cout << "Message found." << std::endl;
}

bool checkAlive()
{
	std::cout << "checkAlive" << std::endl;
    MsgPigeon msg;
    msgrcv(qid, (struct msgbuf *)&msg, MSG_SIZE, ALIVE_ID, 0); // read mesg
    if (ALIVE_MSG.compare(msg.message) == 0)
    {
        sendMessage(ALIVE_MSG, ALIVE_ID);
        return true;
    }
    return false;
}

int main()
{
    std::cout << "Starting sender 257. . ." << std::endl;
    std::srand(time(NULL));
    
    sendMessage("", 259); // initialization handshake
    getMessage(260);
    getMessage(4);
    
    sendMessage(ALIVE_MSG, ALIVE_ID);
    //int qid = msgget(ftok(".",'u'), 0);

    while(true) // checkAlive when event found instead
    {
        int random = std::rand();
        if (random % 257 == 0)
        {
            std::cout << "Event found: " << random << std::endl;
            sendMessage(std::to_string(random), 257);
            
            if (!checkAlive())
            	break;
        }
        
    }
    
	sendMessage("257 Shutting down.", 261);
    //Temporary remove for creating the message queue
    //msgctl (qid, IPC_RMID, NULL);
}
