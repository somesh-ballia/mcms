#include "TestProductTypeDecider.h"
#include "ProductTypeDecider.h"
#include "Trace.h"
#include "DefinesGeneral.h"

CPPUNIT_TEST_SUITE_REGISTRATION( TestProductTypeDecider);

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
extern IsTargetFunc g_isTargetFunc;
BOOL IsTargetTrue()
{
    return TRUE;
}
BOOL IsTargetFalse()
{
    return FALSE;
}

void TestProductTypeDecider::setUp()
{
}

//////////////////////////////////////////////////////////////////////
void TestProductTypeDecider::tearDown()
{
}

//////////////////////////////////////////////////////////////////////
void TestProductTypeDecider::testIfSkipCmdLine()
{
    {
        ProductTypeDecider decider(eProductTypeSoftMCU);
        g_isTargetFunc = IsTargetFalse;

        CPPUNIT_ASSERT(decider.SkipOperation());
    }

    {
        ProductTypeDecider decider(eProductTypeGesher);
        g_isTargetFunc = IsTargetFalse;

        CPPUNIT_ASSERT(!decider.SkipOperation());
    }

    {
        ProductTypeDecider decider(eProductTypeRMX2000);
        g_isTargetFunc = IsTargetTrue;

        CPPUNIT_ASSERT(!decider.SkipOperation());
    }
} 

void TestProductTypeDecider::testProductType()
{
    {
        ProductTypeDecider decider(eProductTypeNinja);
        CPPUNIT_ASSERT(decider.IsGesherNinja());

        decider.SetProductType(eProductTypeSoftMCU);
        CPPUNIT_ASSERT(!decider.IsGesherNinja());
    }
}

void TestProductTypeDecider::testGetCmdLinePrefix()
{
    {
        ProductTypeDecider decider(eProductTypeGesher);
        CPPUNIT_ASSERT_EQUAL(decider.GetCmdLinePrefix(), std::string("sudo "));
    }

    {
        ProductTypeDecider decider(eProductTypeGesher);
        decider.SetProductType(eProductTypeSoftMCU);
        CPPUNIT_ASSERT_EQUAL(decider.GetCmdLinePrefix(), std::string(""));
    }
}

void TestProductTypeDecider::testGetCmdLine()
{
    ProductTypeDecider decider(eProductTypeNinja);
    CPPUNIT_ASSERT_EQUAL(std::string("sudo ls"), decider.GetCmdLine("ls"));

    decider.SetProductType(eProductTypeSoftMCU);
    CPPUNIT_ASSERT_EQUAL(std::string("ls"), decider.GetCmdLine("ls"));

    decider.SetProductType(eProductTypeRMX2000);
    CPPUNIT_ASSERT_EQUAL(std::string("ls"), decider.GetCmdLine("ls"));
}

void TestProductTypeDecider::testGetCycleCmdLine()
{
    {
        ProductTypeDecider decider(eProductTypeGesher); 
        g_isTargetFunc = IsTargetFalse;
        CPPUNIT_ASSERT_EQUAL("sudo "+MCU_MCMS_DIR+"/Scripts/SoftCycleVersions.sh V7.8.0", decider.GetCycleVersionsScriptCmdLine("V7.8.0"));
    }

    {
        ProductTypeDecider decider(eProductTypeRMX2000);
        g_isTargetFunc = IsTargetTrue;
        CPPUNIT_ASSERT_EQUAL(MCU_MCMS_DIR+"/Scripts/CycleVersions.sh V7.8.0", decider.GetCycleVersionsScriptCmdLine("V7.8.0"));
    }
}

void TestProductTypeDecider::testGetMountCmdLine()
{
    {
        ProductTypeDecider decider(eProductTypeGesher); 
        g_isTargetFunc = IsTargetFalse;
        CPPUNIT_ASSERT_EQUAL("sudo "+MCU_MCMS_DIR+"/Scripts/SoftMountNewVersion.sh ", decider.GetMountNewVersionScriptCmdLine());
    }

    {
        ProductTypeDecider decider(eProductTypeRMX2000);
        g_isTargetFunc = IsTargetTrue;
        CPPUNIT_ASSERT_EQUAL(MCU_MCMS_DIR+"/Scripts/MountNewVersion.sh ", decider.GetMountNewVersionScriptCmdLine());
    }
}

void TestProductTypeDecider::testGetUnmountCmdLine()
{
    {
        ProductTypeDecider decider(eProductTypeGesher); 
        g_isTargetFunc = IsTargetFalse;
        CPPUNIT_ASSERT_EQUAL("sudo "+MCU_MCMS_DIR+"/Scripts/SoftUnmountNewVersion.sh ", decider.GetUnmountNewVersionScriptCmdLine());
    }

    {
        ProductTypeDecider decider(eProductTypeRMX2000);
        g_isTargetFunc = IsTargetTrue;
        CPPUNIT_ASSERT_EQUAL(MCU_MCMS_DIR+"/Scripts/UnmountNewVersion.sh ", decider.GetUnmountNewVersionScriptCmdLine());
    }
}

void TestProductTypeDecider::testFirmwareCheckCmdLine()
{
    {
        ProductTypeDecider decider(eProductTypeGesher); 
        g_isTargetFunc = IsTargetFalse;
        CPPUNIT_ASSERT_EQUAL("sudo "+MCU_MCMS_DIR+"/Scripts/SoftRunFirmwareCheck.sh ", decider.GetFirmwareCheckScriptCmdLine());
    }

    {
        ProductTypeDecider decider(eProductTypeRMX2000);
        g_isTargetFunc = IsTargetTrue;
        CPPUNIT_ASSERT_EQUAL(MCU_MCMS_DIR+"/Scripts/RunFirmwareCheck.sh ", decider.GetFirmwareCheckScriptCmdLine());
    }
}

void TestProductTypeDecider::testIfSkipAddNICRoutingTableRule()
{
    {
        ProductTypeDecider decider(eProductTypeGesher); 
        g_isTargetFunc = IsTargetFalse;
        CPPUNIT_ASSERT(decider.IfSkipAddNICRoutingTableRule());
    }

    {
        ProductTypeDecider decider(eProductTypeRMX2000);
        g_isTargetFunc = IsTargetTrue;
        CPPUNIT_ASSERT(decider.IfSkipAddNICRoutingTableRule());
    }

    {
        ProductTypeDecider decider(eProductTypeRMX4000);
        g_isTargetFunc = IsTargetTrue;
        CPPUNIT_ASSERT(!decider.IfSkipAddNICRoutingTableRule());
    }

    {
        ProductTypeDecider decider(eProductTypeRMX1500);
        g_isTargetFunc = IsTargetTrue;
        CPPUNIT_ASSERT(!decider.IfSkipAddNICRoutingTableRule());
    }
}

void TestProductTypeDecider::testGetSimulatedFlashSize()
{
    {
        ProductTypeDecider decider(eProductTypeGesher); 
        g_isTargetFunc = IsTargetFalse;
        CPPUNIT_ASSERT_EQUAL(std::string("0"), decider.GetSimulatedFlashSize());
    }

    {
        ProductTypeDecider decider(eProductTypeSoftMCU); 
        g_isTargetFunc = IsTargetFalse;
        CPPUNIT_ASSERT_EQUAL(std::string("1954"), decider.GetSimulatedFlashSize());
    }
}

