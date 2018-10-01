
#include <iostream>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/types.h>
#include <string>
#include <cstring>
#include <random>
#include "MsgPigeon.cpp"

int qid = msgget(ftok(".",'u'), 0);

void sendMessage(std::string msgContent, long mType)
{
    MsgPigeon msg;
	int size = sizeof(MsgPigeon) - sizeof(long);
    msg.mType = mType;
    strcpy(msg.message, msgContent.c_str());
    msgsnd(qid, (struct MsgPigeon *)&msg, size, 0);
}

int main()
{
    std::cout << "Starting sender 257. . ." << std::endl;
    std::srand(time(NULL));
    sendMessage("KeepAlive", 258);
    //int qid = msgget(ftok(".",'u'), 0);

    while(true)
    {
        int random = std::rand();
        if (random % 257 == 0)
        {
            std::cout << "Event found: " << random << std::endl;
            //sendMessage(random, 2);
        }

        
    }
}
