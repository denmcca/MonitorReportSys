struct MsgPigeon
{
	long mType; // required for queue
	char message[50];
	
	int getSize() { return sizeof(MsgPigeon) - sizeof(long); }
};

