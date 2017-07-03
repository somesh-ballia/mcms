#include "ProductTypeDecider.h"
#include "SystemFunctions.h"

IsTargetFunc g_isTargetFunc = IsTarget;

ProductTypeDecider::ProductTypeDecider(eProductType type)
    : m_productType(type)
{
    g_isTargetFunc = IsTarget;
    m_isGesherNinja = (eProductTypeGesher==m_productType) || (eProductTypeNinja==m_productType);
    m_needSudo = m_isGesherNinja;
}

void ProductTypeDecider::SetProductType(eProductType type)
{
    m_productType = type;
    m_isGesherNinja = (eProductTypeGesher==m_productType) || (eProductTypeNinja==m_productType);
    m_needSudo = m_isGesherNinja;
}

std::string ProductTypeDecider::GetCmdLinePrefix() const
{
    if (m_needSudo)
    {
        return "sudo ";
    }
    else
    {
        return "";
    }
}

std::string ProductTypeDecider::GetCmdLine(char const * base) const
{
    if (m_needSudo)
    {
        return std::string("sudo ") + std::string(base);
    }
    else
    {
        return std::string(base);
    }
}

BOOL ProductTypeDecider::SkipOperation() const
{
    if ((!m_needSudo) && (!(*g_isTargetFunc)()))
    {
        return TRUE;
    }

    return FALSE;
}

std::string ProductTypeDecider::GetMountNewVersionScriptCmdLine() const
{
    if (m_isGesherNinja)
    {
       return "sudo "+MCU_MCMS_DIR+"/Scripts/SoftMountNewVersion.sh ";
    }
    else
    {
       return MCU_MCMS_DIR+"/Scripts/MountNewVersion.sh ";
    }
}

std::string ProductTypeDecider::GetUnmountNewVersionScriptCmdLine() const
{
    if (m_isGesherNinja)
    {
       return "sudo "+MCU_MCMS_DIR+"/Scripts/SoftUnmountNewVersion.sh ";
    }
    else
    {
       return MCU_MCMS_DIR+"/Scripts/UnmountNewVersion.sh ";
    }
}

std::string ProductTypeDecider::GetCycleVersionsScriptCmdLine(std::string const & new_version_name) const
{
    if (m_isGesherNinja)
    {
        return "sudo "+MCU_MCMS_DIR+"/Scripts/SoftCycleVersions.sh " + new_version_name;
    }
    else
    {
        return MCU_MCMS_DIR+"/Scripts/CycleVersions.sh " + new_version_name;
    }
}

std::string ProductTypeDecider::GetFirmwareCheckScriptCmdLine() const
{
    if (m_isGesherNinja)
    {
       return "sudo "+MCU_MCMS_DIR+"/Scripts/SoftRunFirmwareCheck.sh ";
    }
    else
    {
       return MCU_MCMS_DIR+"/Scripts/RunFirmwareCheck.sh ";
    }
}

std::string ProductTypeDecider::GetSimulatedFlashSize() const
{
    if (m_isGesherNinja)
    {
        return "0";
    }
    else
    {
        return "1954";
    }
}

BOOL ProductTypeDecider::IfSkipAddNICRoutingTableRule() const
{
	eProductType curProductType = m_productType;
	if ((eProductTypeRMX2000 == curProductType) || (eProductTypeCallGenerator == curProductType) ||
		(eProductTypeSoftMCU == curProductType) || (eProductTypeSoftMCUMfw == curProductType) ||
		(eProductTypeGesher == curProductType) || (eProductTypeNinja == curProductType) || (eProductTypeEdgeAxis == curProductType))
    {
        return TRUE;
    }

    return FALSE;
}


