
#include <iostream>
#include <stdio.h>
#include "../IncludeInternalMcms/OsFileIF.h"


#include "ProcessBase.h"
#include "SystemFunctions.h"

CProcessBase* CreateNewProcess();


int main(int argc, char* argv[])
{
	CProcessBase* process = CreateNewProcess();
	int res = 0;

    process->SetArgv(argv);
    process->SetArgc(argc);
	res = process->Run();

	delete process;

	return res;
}

bool IsUnitTest()
{
	return false;
}

void UnitTestAssert(const char * text)
{

}
