//Revisions and Updates:
//
//Date         Updated By         Description
//
//1/12/05		Udi				Porting to Carmel
//========   ==============   =====================================================================

#include <utility>
#include <algorithm>
#include "StatusesGeneral.h"
#include "ObjString.h"
#include "DHTask.h"
#include "ProcessBase.h"
#include "SysConfig.h"
#include "StructTm.h"
#include "SystemFunctions.h"
#include "IpEncryptionDefinitions.h"
#include "ManagerApi.h"
#include "OpcodesMcmsInternal.h"

extern "C" {
	 void sgenrand(UINT32 seed);
}


extern void GetHalfKeyWithPrime(DWORD g,CDHKey* rnd, CDHKey* hk, BYTE* prime);


//----------------------------------- DHTask Map ----------------------------
PBEGIN_MESSAGE_MAP(CDiffieHelman)
	ONEVENT(REQ_DH_FOR_NEW_KEY    ,ANYCASE    ,  CDiffieHelman::OnReqToNewKey )
PEND_MESSAGE_MAP(CDiffieHelman,CStateMachine);
//---------------------------------------------------------------------------


//////////////////////////////////////////////////////////////////////////////
extern "C" void DHEntryPoint(void* appParam)
{
   CDiffieHelman*  pDHTaskApp = new CDiffieHelman;
   pDHTaskApp->Create(*(CSegment*)appParam);
}


//////////////////////////CDHApi/////////////////////////////////////////////
const char*   CDHApi::NameOf() const
{
 	return "CDHApi";
}

////////////////////////////////////////////////////////////////////////////
void  CDHApi::Create(void (*entryPoint)(void*),COsQueue& creatorRcvMbx)
{
 	CTaskApi::Create(creatorRcvMbx);
	LoadApp(entryPoint);
}

////////////////////////////////////////////////////////////////////////////
void CDHApi::GetNewKey(DWORD gen, BYTE bIsGenKeyFromStartup)
{
	CSegment * pSeg = new CSegment;
	*pSeg <<  gen;
	*pSeg <<  bIsGenKeyFromStartup;
	SendMsg(pSeg,REQ_DH_FOR_NEW_KEY);
}

////------------------------------ CDiffieHelman------------------------------
/////////////////////////////////////////////////////////////////////////////
CDiffieHelman::CDiffieHelman() // constructor
{
	//Set the task group, for setting the task priority
	m_Thread_Group =  eTaskGroupLow;
}

/////////////////////////////////////////////////////////////////////////////
CDiffieHelman::~CDiffieHelman() // destructor
{
}

/////////////////////////////////////////////////////////////////////////////
const char * CDiffieHelman::GetTaskName() const
{
	return "CDiffieHelman";
}

/////////////////////////////////////////////////////////////////////////////
const char*   CDiffieHelman::NameOf()  const
{
	return "CDiffieHelman";
}

/////////////////////////////////////////////////////////////////////////////
void  CDiffieHelman::Create(CSegment& appParam)
{
	CTaskApp::Create(appParam/*,"DHTK"*/);
	//CTaskLoader::SetTaskParams(E_dhtk,this,LOW_TASK_PRIORITY);
	//SetPriority(LOW_TASK_PRIORITY);
    //Run();
}

/////////////////////////////////////////////////////////////////////////////
void CDiffieHelman::InitTask()
{
	/*Generating Seed - Called only once on startup
	 any nonzero integer can be used as a seed for random generator
	 Saves the random seed in Global variable  */

	//Settting the seed for the random lib
	CStructTm curTime;
    PASSERT(SystemGetTime(curTime));
    srand( (unsigned)(curTime));
	sgenrand(rand());
}

/////////////////////////////////////////////////////////////////////////////
void CDiffieHelman::OnReqToNewKey(CSegment * pSeg)
{
	DWORD gen ;
	BYTE bIsGenKeyFromStartup = NO;
	CdhEntry * pDHEntry = new CdhEntry;

	*pSeg >> gen; //take the relevant generator
	*pSeg >> bIsGenKeyFromStartup;

	//Make sure the Generator is valid
	if ( IsValidGenerator(gen) == FALSE )
	{
		PTRACE(eLevelError,"CDiffieHelman::OnReqToNewKey UnValid Generator !!!!");
		PASSERT(1);
		POBJDELETE(pDHEntry);
		return;
	}

	WORD fips140Status = pDHEntry->CreateRandom();

	if (fips140Status != STATUS_OK)
	{
		PTRACE(eLevelError,"CDiffieHelman::OnReqToNewKey CreateRandom Failed !!!!");
		PASSERT(1);
	}
	else
	{
		//Set the relevant prime number
		BYTE * prime = (gen == TANBERG_H320_DH_GENERATOR ? tandbergH320Prime : polycomPrime);
		//PTRACE2INT(eLevelInfoNormal,"------CDiffieHelman::OnReqToNewKey - generating key of type: G%d ---------",gen );
		pDHEntry->GenerateHKWithPrime(gen, prime);
	}

	SendKeyToManager(fips140Status, bIsGenKeyFromStartup, gen, pDHEntry);
	POBJDELETE(pDHEntry);
}

/////////////////////////////////////////////////////////////////////////////
void CDiffieHelman::SendKeyToManager(DWORD fips140Status, BYTE bIsGenKeyFromStartup, DWORD gen,CdhEntry * pDHEntry)
{
    CManagerApi api(eProcessEncryptionKeyServer);

    CSegment * pSeg = new CSegment;

    *pSeg <<  fips140Status ;
    *pSeg <<  bIsGenKeyFromStartup ;
    *pSeg <<  gen ;
     pDHEntry->Serialize(NATIVE,*pSeg);

	api.SendMsg(pSeg,DH_IND_ON_NEW_KEY);
}

/////////////////////////////////////////////////////////////////////////////
BOOL CDiffieHelman::IsValidGenerator(DWORD gen)const
{
	if (gen == TANBERG_H320_DH_GENERATOR ||
			gen == POLYCOM_DH_GENERATOR  ||
			gen == TANBERG_H323_DH_GENERATOR)
		return TRUE;

	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////
void CDiffieHelman::Destroy()
{

}

/////////////////////////////////////////////////////////////////////////////
void CDiffieHelman::SelfKill()
{
	CTaskApp::SelfKill();
}

