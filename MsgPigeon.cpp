struct MsgPigeon
{
	long mType; // required for queue
	char message[50]; // maybe an int instead?
	
	int getSize() { return sizeof(this) - sizeof(long); }
};

