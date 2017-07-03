
#ifdef LED_CLI
#include "LedLightClient.h"
#else
#include "LedLightServer.h"
#endif

#include "LedLightTask.h"

//compile comand: g++ -g cppTest.cpp -o gTest
/* for Led Client
g++ -o ledCli main.cpp LedLightClient.cpp LedLightTask.cpp BaseProcessor.cpp -DLED_CLI
*/

/* for Led Server
g++ -o ledSrv main.cpp LedLightServer.cpp LedLightTask.cpp SystemAlarmProcessor.cpp UsbUpgradeProcessor.cpp HotStandbyProcessor.cpp DiagnosticProcessor.cpp sysLightTimerMngr.cpp BaseProcessor.cpp LightCntlApi.cpp -DLED_SRV -L /root/ctest/ledlight/ledApi/  -lninja_led*/

/*
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/root/ctest/ledlight/ledApi/
*/
LedLightTask* CreateNewProcess();

int main(int argc, char *argv[])
{
	LedLightTask* process = CreateNewProcess();
	int res = 0;

    process->SetArgv(argv);
    process->SetArgc(argc);
	res = process->Run();

	delete process;

	return res;
}

