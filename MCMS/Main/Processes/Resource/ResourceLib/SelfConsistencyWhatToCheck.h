#ifndef SELFCONSISTENCYWHATTOCHECK_H_
#define SELFCONSISTENCYWHATTOCHECK_H_

#include "DataTypes.h"

class CSelfConsistencyWhatToCheck
{
public:
	CSelfConsistencyWhatToCheck();
	virtual ~CSelfConsistencyWhatToCheck();
	
	void CheckEverything();
	
	BOOL m_bCheckNoMoreConferences;
	BOOL m_bCheckNumOfConferences;
	BOOL m_bCheckAudioVideoConfiguration;
	BOOL m_bCheckNIDs;
	BOOL m_bCheckPhones;	
	BOOL m_bCheckParties;
	BOOL m_bCheckPartiesResourceDescriptors;
	BOOL m_bCheckPartiesResourceDescriptorsCheckSharedMemory;
	BOOL m_bCheckPartiesUDPDescriptors;
	BOOL m_bCheckPartiesPorts;
	
private:
	void SetAll(BOOL bset);	
};

#endif /*SELFCONSISTENCYWHATTOCHECK_H_*/
