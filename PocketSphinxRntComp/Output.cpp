#include "Output.h"

using namespace PocketSphinxRntComp;

#include <Windows.h>
#include <iostream>
#include <sstream>

#define DBOUT( s )							\
{											\
	std::wostringstream os_;				\
	os_ << s;								\
	OutputDebugStringW( os_.str().c_str());	\
}

Output::Output()
{

}

void Output::WriteLine(Platform::String^ message)
{
	Write(message);
	Write("\n");
}

void Output::Write(Platform::String^ message)
{
#if defined (_DEBUG)
	std::wstring wStrMessage = std::wstring(message->Begin(), message->End());
	LPCWSTR messageLPCWSTR = wStrMessage.c_str();

	OutputDebugStringW(messageLPCWSTR);
#endif
}

//void Output::WriteTest()
//{
//	//int x = 11;
//	//DBOUT("The value of x is " << x);
//}

