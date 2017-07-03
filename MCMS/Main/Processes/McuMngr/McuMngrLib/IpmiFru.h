#ifndef __IPMI_FRU_H__
#define __IPMI_FRU_H__

#include "SerializeObject.h"
#include "SysConfig.h"
#include "McuMngrDefines.h"
#include "McuMngrStructs.h"
#include "CardContent.h"
#include "IpmiConsts.h"
#include "ProductType.h"
#include <strings.h>

class CXMLDOMElement;
class CStructTm;

struct IpmiFruInfo
{
    char boardSerialNumber[64];
    char boardFileId[16];
    int boardMfgDateTime;
    char boardPartNumber[64];
    int chassisType;
    char boardHardwareVers[64];
    char boardProductName[32];
    int subBoardID;
    char boardSoftwareVers[64];
    char macs[4][64];
    //Begin:added by richer for displaying DCLP version
    char riserCardCpldVersion [64];
    //End:added by richer for displaying DCLP version
    IpmiFruInfo()
        : boardMfgDateTime(111)
        , chassisType(0)
        , subBoardID(1)
    {
        bzero(boardSerialNumber, sizeof(boardSerialNumber));
        bzero(boardFileId, sizeof(boardFileId));
        bzero(boardPartNumber, sizeof(boardPartNumber));
        bzero(boardHardwareVers, sizeof(boardHardwareVers));
	bzero(boardProductName, sizeof(boardProductName));
        bzero(boardSoftwareVers, sizeof(boardSoftwareVers));
        bzero(macs, sizeof(macs));
	 //Begin:added by richer for displaying DCLP version
	 bzero(riserCardCpldVersion, sizeof(riserCardCpldVersion));
	 //End:added by richer for displaying DCLP version
    }
};

class CIpmiFru : public CSerializeObject
{
    CLASS_TYPE_1(CIpmiFru,CPObject)
public:
    //Constructors
    CIpmiFru();
    virtual ~CIpmiFru();


    // Implementation
    virtual CSerializeObject* Clone()
    {
        return new CIpmiFru;
    }
    virtual void SerializeXml(CXMLDOMElement*& pFatherNode) const;
    virtual int  DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError,const char* action);

    const char*  NameOf() const;

    void Update(int slotId);

protected:
    int m_curSlotId;
    IpmiFruInfo m_fruInfo;
};

#endif /* __IPMI_FRU_H__ */

