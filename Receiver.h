//static int globalReceiverId = 0; // receiver 1 and 2 run as separate programs.
#include "MsgPigeon.cpp"

class Receiver
{
	public:
	int qid;	// id of queue
	int id;		// id of receiver
	static const int idListSize = 2;	// size of list that holds possible receiver ids.
	int idList[idListSize] = {1, 2};	// list that holds values of possible receiver ids.
	MsgPigeon msgr;	// sends and gets values from queue
	
	Receiver(int);	// constructor
	int assignReceiverNumber();	// displays prompt for user to select from ids.
	int getReceiverNumber();	// prompts user to enter choice of ids.
	bool getEvent(long);
	bool terminate(); // clean up values from queue.
};

