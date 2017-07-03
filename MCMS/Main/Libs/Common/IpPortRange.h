// CIpPortRange.h: interface for the CIpPortRange class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _IpPortRange_H_
#define _IpPortRange_H_


#include "PObject.h"
#include "McuMngrStructs.h"

using namespace std;



class CIpPortRange : public CPObject
{

CLASS_TYPE_1(CIpPortRange, CPObject)

public:
	CIpPortRange ();
	CIpPortRange (const IP_PORT_RANGE_S ipPortRange);
	virtual ~CIpPortRange ();
	virtual void Dump(ostream& msg) const;
	
	virtual const char* NameOf() const { return "CIpPortRange";}
	
	CIpPortRange& operator = (const CIpPortRange &rOther);

	IP_PORT_RANGE_S  GetIpPortRangeStruct();
	void             SetIpPortRangeStruct(CIpPortRange ipPortRange);


	DWORD            GetDynamicPortAllocation ();
	void             SetDynamicPortAllocation (const DWORD allocation);

	DWORD            GetSignallingFirstPort ();
	void             SetSignallingFirstPort (const DWORD sgnlngFirstPort);

	DWORD            GetSignallingNumPorts ();
	void             SetSignallingNumPorts (const DWORD sgnlngNumPorts);

	DWORD            GetControlFirstPort ();
	void             SetControlFirstPort (const DWORD cntrlFirstPort);

	DWORD            GetControlNumPorts ();
	void             SetControlNumPorts (const DWORD cntrlNumPorts);

	DWORD            GetAudioFirstPort ();
	void             SetAudioFirstPort (const DWORD audFirstPort);

	DWORD            GetAudioNumPorts ();
	void             SetAudioNumPorts (const DWORD audNumPorts);

	DWORD            GetVideoFirstPort ();
	void             SetVideoFirstPort (const DWORD vidFirstPort);

	DWORD            GetVideoNumPorts ();
	void             SetVideoNumPorts (const DWORD vidNumPorts);

	DWORD            GetContentFirstPort ();
	void             SetContentFirstPort (const DWORD content_FirstPort);

	DWORD            GetContentNumPorts ();
	void             SetContentNumPorts (const DWORD content_NumPorts);

	DWORD            GetFeccFirstPort ();
	void             SetFeccFirstPort (const DWORD fecc_FirstPort);

	DWORD            GetFeccNumPorts ();
	void             SetFeccNumPorts (const DWORD fecc_NumPorts);

	DWORD            GetNumIntendedCalls ();
	void             SetNumIntendedCalls (const DWORD numIntndCalls);

	DWORD            GetEnablePortRange ();
	void             SetEnablePortRange (const DWORD enableRange);

	
	void             SetData(IP_PORT_RANGE_S ipPortRange);


protected:
	IP_PORT_RANGE_S  m_ipPortRangeStruct;
};

#endif // _IpPortRange_H_
