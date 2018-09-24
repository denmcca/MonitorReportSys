#include <iostream>

using namespace std;

class PTools
{

	public:
	static void flushCin()
	{
		cin.clear();
		cin.ignore(INT8_MAX, '\n');
	}

};
