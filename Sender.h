#include "MsgPigeon.cpp"

class Sender
{
	public:
	int qid;	// id for queue.
	int marker;	// holds modulus value for events
	static const int numListSize = 3;	// size of list that holds marker values
	int numList[numListSize] = {251, 257, 997};	// marker values
	MsgPigeon msgr;	// Sends and gets values from queue
	
	Sender(int);	// Constructor
	int assignNumber();	// prompts user to select from available marker values
	int getMarkerNumber(); // prompts user to select value from marker list
	void sendEvent(int, long); // sends event
	bool terminate();
	int generateRandomEvent();
};


