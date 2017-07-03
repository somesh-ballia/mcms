#ifndef __IPMI_ENTITY_LIST_H__
#define __IPMI_ENTITY_LIST_H__

#include "SerializeObject.h"
#include "SysConfig.h"
#include "McuMngrDefines.h"
#include "McuMngrStructs.h"
#include "CardContent.h"

class CXMLDOMElement;
class CStructTm;

/////////////////////////////////////////////////////////////////////////////
// CIpmiEntityList


class CIpmiEntityList : public CSerializeObject
{
    CLASS_TYPE_1(CIpmiEntityList,CPObject)
public:
    //Constructors
    CIpmiEntityList();
    virtual ~CIpmiEntityList();


    // Implementation
    virtual CSerializeObject* Clone()
    {
        return new CIpmiEntityList;
    }
    virtual void SerializeXml(CXMLDOMElement*& pFatherNode) const;
    virtual int  DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError,const char* action);

    const char*  NameOf() const;

    void Update();

protected:
    vector<CardContent> m_cardContents;
};

#endif /* __IPMI_ENTITY_LIST_H__ */

