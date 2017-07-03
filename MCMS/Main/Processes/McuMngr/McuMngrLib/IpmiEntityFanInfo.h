#ifndef __IPMI_ENTITY_FAN_INFO_H__
#define __IPMI_ENTITY_FAN_INFO_H__

#include "SerializeObject.h"
#include "SysConfig.h"
#include "McuMngrDefines.h"
#include "McuMngrStructs.h"

class CXMLDOMElement;
class CStructTm;

struct IpmiFanInfo
{
    int maxSpeedLevel;
    int normalOperatingLevel;
    int minSpeedLevel;
};

class CIpmiEntityFanInfo : public CSerializeObject
{
    CLASS_TYPE_1(CIpmiEntityFanInfo,CPObject)
public:
    //Constructors
    CIpmiEntityFanInfo();
    virtual ~CIpmiEntityFanInfo();


    // Implementation
    virtual CSerializeObject* Clone()
    {
        return new CIpmiEntityFanInfo;
    }
    virtual void SerializeXml(CXMLDOMElement*& pFatherNode) const;
    virtual int  DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError,const char* action);

    const char*  NameOf() const;

    void Update();

protected:
    IpmiFanInfo m_entry;
};

#endif /* __IPMI_ENTITY_FAN_INFO_H__ */

