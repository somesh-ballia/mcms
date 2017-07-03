#ifndef __IPMI_ENTITY_FAN_LEVEL_H__
#define __IPMI_ENTITY_FAN_LEVEL_H__

#include "SerializeObject.h"
#include "SysConfig.h"
#include "McuMngrDefines.h"
#include "McuMngrStructs.h"
#include <vector>
using std::vector;

class CXMLDOMElement;
class CStructTm;

enum
{
      IPMI_FAN_LEVEL_LOWER_CRITICAL = 0
    , IPMI_FAN_LEVEL_DEFAULT = 2
    , IPMI_FAN_LEVEL_UPPER_CRITICAL = 4
};

struct IpmiFanLevel
{
    int fanLevel;

    IpmiFanLevel(int level=IPMI_FAN_LEVEL_DEFAULT) : fanLevel(level) {}
};

struct IpmiFanLevelLs
{
    IpmiFanLevel levelOfAll;
    vector<IpmiFanLevel> levels;

    void clear()
    {
        levelOfAll = IPMI_FAN_LEVEL_DEFAULT;
        levels.clear();
    }
};

class CIpmiEntityFanLevel : public CSerializeObject
{
    CLASS_TYPE_1(CIpmiEntityFanLevel,CPObject)
public:
    //Constructors
    CIpmiEntityFanLevel();
    virtual ~CIpmiEntityFanLevel();


    // Implementation
    virtual CSerializeObject* Clone()
    {
        return new CIpmiEntityFanLevel;
    }
    virtual void SerializeXml(CXMLDOMElement*& pFatherNode) const;
    virtual int  DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError,const char* action);

    const char*  NameOf() const;

    void Update();

protected:
    IpmiFanLevelLs m_levels;
};

#endif /* __IPMI_ENTITY_FAN_LEVEL_H__ */

