#ifndef __PRODUCT_TYPE_DECODER_H__
#define __PRODUCT_TYPE_DECODER_H__

#include "ProductType.h"
#include "DataTypes.h"
#include <string>

typedef BOOL (*IsTargetFunc)();

class ProductTypeDecider
{
public:
    ProductTypeDecider(eProductType type);

    void SetProductType(eProductType type);
    eProductType GetProductType() const { return m_productType; }
    BOOL IsGesherNinja() const { return m_isGesherNinja; }

    std::string GetCmdLinePrefix() const;
    std::string GetCmdLine(char const * base) const;
    BOOL SkipOperation() const;
    //char const * GetHddDevName() const;
    std::string GetCycleVersionsScriptCmdLine(std::string const & new_version_name) const;
    std::string GetMountNewVersionScriptCmdLine() const;
    std::string GetUnmountNewVersionScriptCmdLine() const;
    std::string GetFirmwareCheckScriptCmdLine() const;
    std::string GetSimulatedFlashSize() const;
    BOOL IfSkipAddNICRoutingTableRule() const;

private:
    eProductType m_productType;
    BOOL m_isGesherNinja;
    BOOL m_needSudo;
};

#endif

