#include "Apple1.h"
#include "smart_pointer.h"

int main()
{
	Ptr<Emu::Apple1> computer(new Emu::Apple1());
	computer->run();
	
	return 0;
}