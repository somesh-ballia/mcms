#!/mcms/python/bin/python

#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_13



from SysCfgUtils import *
from AlertUtils import *

#------------------------------------------------------------------------------
def PrintSendCleanSetCfg(sysCfgUtils, callName, expectedStatus):
    print callName
    sysCfgUtils.PrintCfgTable()
    
    sysCfgUtils.SendSetCfg(expectedStatus)
    sysCfgUtils.Clean()

#------------------------------------------------------------------------------
def SendInvalidCfgParamNotExist(sysCfgUtils):
    sysCfgUtils.AddCfgParam("MCMS_PARAMETERS_USER", "cucu_lulu", "172.22.188.157", "user")
    sysCfgUtils.AddCfgParam("MCMS_PARAMETERS_USER", "EXTERNAL_DB_PORT","5005","user")
    sysCfgUtils.AddCfgParam("MCMS_PARAMETERS_USER", "EXTERNAL_DB_LOGIN","POLYCOM","user")
    sysCfgUtils.AddCfgParam("MCMS_PARAMETERS_USER", "EXTERNAL_DB_PASSWORD","POLYCOM","user")
    sysCfgUtils.AddCfgParam("MCMS_PARAMETERS_USER", "EXTERNAL_DB_DIRECTORY","A","user")
    sysCfgUtils.AddCfgParam("MCMS_PARAMETERS_USER", "ENABLE_EXTERNAL_DB_ACCESS","YES","user")

    PrintSendCleanSetCfg(sysCfgUtils, "SendInvalidCfgParamNotExist", "The new flag cannot be added. Flag is not recognized by the system")
    
#------------------------------------------------------------------------------
def SendInvalidCfgSectionDontMatch(sysCfgUtils):
    sysCfgUtils.AddCfgParam("cucu_lulu", "EXTERNAL_DB_IP", "172.22.188.157", "user")
    sysCfgUtils.AddCfgParam("MCMS_PARAMETERS_USER", "EXTERNAL_DB_PORT","5005","user")
    sysCfgUtils.AddCfgParam("MCMS_PARAMETERS_USER", "EXTERNAL_DB_LOGIN","POLYCOM","user")
    sysCfgUtils.AddCfgParam("MCMS_PARAMETERS_USER", "EXTERNAL_DB_PASSWORD","POLYCOM","user")
    sysCfgUtils.AddCfgParam("MCMS_PARAMETERS_USER", "EXTERNAL_DB_DIRECTORY","A","user")
    sysCfgUtils.AddCfgParam("MCMS_PARAMETERS_USER", "ENABLE_EXTERNAL_DB_ACCESS","YES","user")

    PrintSendCleanSetCfg(sysCfgUtils, "SendInvalidCfgSectionDontMatch", "Flag cannot be added. Flag added to the wrong System Configuration Section")

#------------------------------------------------------------------------------
def SendInvalidCfgParamSectionDontMatch(sysCfgUtils):
    print "SendInvalidCfgParamSectionDontMatch: Does nothing"
      
#------------------------------------------------------------------------------
def SendInvalidCfgParamTypeDontMatch(sysCfgUtils):
    sysCfgUtils.AddCfgParam("MCMS_PARAMETERS_USER", "APACHE_PRINT", "YES", "user")
    sysCfgUtils.AddCfgParam("MCMS_PARAMETERS_USER", "EXTERNAL_DB_PORT","5005","user")
    sysCfgUtils.AddCfgParam("MCMS_PARAMETERS_USER", "EXTERNAL_DB_LOGIN","POLYCOM","user")
    sysCfgUtils.AddCfgParam("MCMS_PARAMETERS_USER", "EXTERNAL_DB_PASSWORD","POLYCOM","user")
    sysCfgUtils.AddCfgParam("MCMS_PARAMETERS_USER", "EXTERNAL_DB_DIRECTORY","A","user")
    sysCfgUtils.AddCfgParam("MCMS_PARAMETERS_USER", "ENABLE_EXTERNAL_DB_ACCESS","YES","user")

    PrintSendCleanSetCfg(sysCfgUtils, "SendInvalidCfgParamTypeDontMatch", "System flag was added to the incorrect configuration file")

#------------------------------------------------------------------------------
def SendInvalidCfgParamDataTypeDontMatchIpAddress(sysCfgUtils):
    sysCfgUtils.AddCfgParam("MCMS_PARAMETERS_USER", "EXTERNAL_DB_IP", "cucu_lulu", "user")
    sysCfgUtils.AddCfgParam("MCMS_PARAMETERS_USER", "EXTERNAL_DB_PORT","5005","user")
    sysCfgUtils.AddCfgParam("MCMS_PARAMETERS_USER", "EXTERNAL_DB_LOGIN","POLYCOM","user")
    sysCfgUtils.AddCfgParam("MCMS_PARAMETERS_USER", "EXTERNAL_DB_PASSWORD","POLYCOM","user")
    sysCfgUtils.AddCfgParam("MCMS_PARAMETERS_USER", "EXTERNAL_DB_DIRECTORY","A","user")
    sysCfgUtils.AddCfgParam("MCMS_PARAMETERS_USER", "ENABLE_EXTERNAL_DB_ACCESS","YES","user")

    PrintSendCleanSetCfg(sysCfgUtils, "SendInvalidCfgParamDataTypeDontMatchIpAddress", "Invalid flag value")

#------------------------------------------------------------------------------
def SendInvalidCfgParamDataTypeDontMatchInt(sysCfgUtils):
    sysCfgUtils.AddCfgParam("MCMS_PARAMETERS_USER", "EXTERNAL_DB_IP", "172.22.188.157", "user")
    sysCfgUtils.AddCfgParam("MCMS_PARAMETERS_USER", "EXTERNAL_DB_PORT","cucu_lulu","user")
    sysCfgUtils.AddCfgParam("MCMS_PARAMETERS_USER", "EXTERNAL_DB_LOGIN","POLYCOM","user")
    sysCfgUtils.AddCfgParam("MCMS_PARAMETERS_USER", "EXTERNAL_DB_PASSWORD","POLYCOM","user")
    sysCfgUtils.AddCfgParam("MCMS_PARAMETERS_USER", "EXTERNAL_DB_DIRECTORY","A","user")
    sysCfgUtils.AddCfgParam("MCMS_PARAMETERS_USER", "ENABLE_EXTERNAL_DB_ACCESS","YES","user")

    #PrintSendCleanSetCfg(sysCfgUtils, "SendInvalidCfgParamDataTypeDontMatchInt", "Invalid flag value")
    PrintSendCleanSetCfg(sysCfgUtils, "SendInvalidCfgParamDataTypeDontMatchInt", "Invalid ENUM value")
 
#------------------------------------------------------------------------------
def SendInvalidCfgParamDataTypeDontMatchBool(sysCfgUtils):
    sysCfgUtils.AddCfgParam("MCMS_PARAMETERS_USER", "EXTERNAL_DB_IP", "172.22.188.157", "user")
    sysCfgUtils.AddCfgParam("MCMS_PARAMETERS_USER", "EXTERNAL_DB_PORT","5005","user")
    sysCfgUtils.AddCfgParam("MCMS_PARAMETERS_USER", "EXTERNAL_DB_LOGIN","POLYCOM","user")
    sysCfgUtils.AddCfgParam("MCMS_PARAMETERS_USER", "EXTERNAL_DB_PASSWORD","POLYCOM","user")
    sysCfgUtils.AddCfgParam("MCMS_PARAMETERS_USER", "EXTERNAL_DB_DIRECTORY","A","user")
    sysCfgUtils.AddCfgParam("MCMS_PARAMETERS_USER", "ENABLE_EXTERNAL_DB_ACCESS","cucu_lulu","user")

    PrintSendCleanSetCfg(sysCfgUtils, "SendInvalidCfgParamDataTypeDontMatchBool", "Invalid ENUM value")
    
#------------------------------------------------------------------------------
def SendValidCfg(sysCfgUtils, cfgParamList):
    sysCfgUtils.SetParamList(cfgParamList)

    PrintSendCleanSetCfg(sysCfgUtils, "SendValidCfg", "Status OK")

#------------------------------------------------------------------------------    
def GetCfgFileNameByType(type):
    fileName = "Cfg/SystemCfgUserTmp.xml"
    if type == "debug":
        fileName = "Cfg/SystemCfgDebugTmp.xml"
    elif type <> "user":
        print "CheckFile----Bad Cfg type(user/debug) : " + type
        sys.exit(1)
    return fileName

#------------------------------------------------------------------------------    
def CheckFile(wantedCfgParamList, type):
    checkFileSysCfgUtils = SyscfgUtilities()
    
    fileName = GetCfgFileNameByType(type)
    checkFileSysCfgUtils.LoadXmlFile(fileName)
    cfgSectionList = checkFileSysCfgUtils.loadedXml.getElementsByTagName("CFG_SECTION")
    if cfgSectionList.length < 1:
        print "CheckFile----cfgSectionList.length < 1"
        sys.exit(1)

    
    sectionNameElementList = cfgSectionList[0].getElementsByTagName("NAME")
    if sectionNameElementList.length <> 1:
        print "CheckFile----sectionNameElementList.length <> 1"
        sys.exit(1)
    sectionName = sectionNameElementList[0].firstChild.data
    
    cfgParamList = cfgSectionList[0].getElementsByTagName("CFG_PAIR")
    for cfgPair in cfgParamList:
        cfgKey = cfgPair.getElementsByTagName("KEY")
        if cfgKey.length <> 1:
            print "CheckFile----cfgKey.length <> 1"
            sys.exit(1)
        
        cfgData = cfgPair.getElementsByTagName("DATA")
        if cfgData.length <> 1:
            print "CheckFile----cfgData.length <> 1"
            sys.exit(1)
            
        keyName = cfgKey[0].firstChild.data
        dataName = cfgData[0].firstChild.data

        currentParam = checkFileSysCfgUtils.CreateCfgParam(sectionName, keyName, dataName, type)
        isContain = wantedCfgParamList.IsContain(currentParam)
        if isContain == False:
            print "CheckFile----Param was not found"
            currentParam.Print()
            return False
        
    print "CheckFile " + fileName + " : Succeeded"
    return True
    
#------------------------------------------------------------------------------
def CheckParamOverwriten(cfgParamUserList):

    retCode = True
    outputFileName1 = "tmpTerminalCommandOutput1.yura"
    outputFileName2 = "tmpTerminalCommandOutput2.yura"
    separateChar = "="
    
    for param in cfgParamUserList.m_SysCfgParamList:
        
        commandLine = "Bin/McuCmd @get cdr " + param.m_Key + " > "  + outputFileName1
        os.system(commandLine)
        
        commandLine = "cat " + outputFileName1 + " | awk '{print $1\"" + separateChar + "\"$2\"" + separateChar + "\"}' > " + outputFileName2
        os.system(commandLine)
        
        commandOutput = open(outputFileName2);
        
        keyLen = len(param.m_Key)
        bFound = False
        line = "Cucu_lulu"
        while (line != ""):
            line = commandOutput.readline()
            if(line[0:keyLen] == param.m_Key):
                bFound = True
                break

        if bFound == False:
            print "key was not found in the [" + commandLine  + "] answer"
            retCode = False
            commandOutput.close()
            break
        else:
            lineSplit = line.split(separateChar)
            data = lineSplit[1]

            if data <> param.m_Data:
                print "data is different"
                print "Expected:Given    " + param.m_Key + " -> " + param.m_Data + " : " + data
                retCode = False
                commandOutput.close()
                break

        commandOutput.close()

    
    commandLine = "rm " + outputFileName1
    os.system(commandLine)

    commandLine = "rm " + outputFileName2
    os.system(commandLine)
        
    if retCode:
        print "CheckParamOverwriten: SUCCESS"
    else:
        print "CheckParamOverwriten: FAILURE"
    
    return retCode
    

#------------------------------------------------------------------------------
def Reset():
    os.system("Scripts/Destroy.sh")
    os.system("Scripts/Startup.sh")


#------------------------------------------------------------------------------
def CreateCfgParamUserList():
    createUserCfgUtils = SyscfgUtilities()
    createUserCfgUtils.AddCfgParam("MCMS_PARAMETERS_USER", "EXTERNAL_DB_IP", "172.22.188.42", "user")
    createUserCfgUtils.AddCfgParam("MCMS_PARAMETERS_USER", "EXTERNAL_DB_PORT","5001","user")
    createUserCfgUtils.AddCfgParam("MCMS_PARAMETERS_USER", "EXTERNAL_DB_LOGIN","CUCU_LULU","user")
    createUserCfgUtils.AddCfgParam("MCMS_PARAMETERS_USER", "EXTERNAL_DB_PASSWORD","CUCU_LULU","user")
    createUserCfgUtils.AddCfgParam("MCMS_PARAMETERS_USER", "EXTERNAL_DB_DIRECTORY","B","user")
    createUserCfgUtils.AddCfgParam("MCMS_PARAMETERS_USER", "ENABLE_EXTERNAL_DB_ACCESS","NO","user")
    return createUserCfgUtils.m_SysCfgParamList


#------------------------------------------------------------------------------
def CreateCfgParamDebugList():
    createDebugCfgUtils = SyscfgUtilities()
    createDebugCfgUtils.AddCfgParam("MCMS_PARAMETERS_DEBUG", "APACHE_PRINT", "YES", "debug")
    createDebugCfgUtils.AddCfgParam("MCMS_PARAMETERS_DEBUG", "CFS_KEYCODE_VERSION_NUM_IGNORED","YES","debug")
    createDebugCfgUtils.AddCfgParam("MCMS_PARAMETERS_DEBUG", "CHANGE_MODE_IN_VSW_FIXED","YES","debug")
    createDebugCfgUtils.AddCfgParam("MCMS_PARAMETERS_DEBUG", "CS_KEEP_ALIVE_RECEIVE_PERIOD","100","debug")
    createDebugCfgUtils.AddCfgParam("MCMS_PARAMETERS_DEBUG", "CS_XML_CONVERTER","NO","debug")
    createDebugCfgUtils.AddCfgParam("MCMS_PARAMETERS_DEBUG", "DEBUG_MODE","YES","debug")
    return createDebugCfgUtils.m_SysCfgParamList


#------------------------------------------------------------------------------
def SendInvalidParams(sysCfgUtils):
    SendInvalidCfgParamNotExist(sysCfgUtils)
    SendInvalidCfgSectionDontMatch(sysCfgUtils)
    SendInvalidCfgParamTypeDontMatch(sysCfgUtils)

    # now there is no validation of IP Address
#    SendInvalidCfgParamDataTypeDontMatchIpAddress(sysCfgUtils)
    SendInvalidCfgParamDataTypeDontMatchInt(sysCfgUtils)
    SendInvalidCfgParamDataTypeDontMatchBool(sysCfgUtils)
    
    SendInvalidCfgParamSectionDontMatch(sysCfgUtils)


#------------------------------------------------------------------------------
def CheckParamsWereInput():
    aaUtils = AlertUtils()
    isExist = aaUtils.IsAlertExist("McuMngr", "Manager", "CFG_CHANGED")
    if(False == isExist):
        print "Alert CFG_CHANGED was not found"
        sys.exit(1)
        
    checkRes = CheckFile(cfgParamUserList,"user" )
    if checkRes == False:
        print "check user file FAILED, expected success"
        sys.exit(1)
            
    checkRes = CheckFile(cfgParamDebugList, "debug")
    if checkRes == False:
        print "check debug file FAILED, expected SUCCESS"
        sys.exit(1)
                
                
    checkRes = CheckFile(cfgParamUserList,"debug" )
    if checkRes == True:
        print "check user file SUCCESSES, expected FAILURE"
        sys.exit(1)


#------------------------------------------------------------------------------
def CheckParamsWereOverwriten():
    checkRes = CheckParamOverwriten(cfgParamUserList)
    if checkRes == False:
        print "check User params were overwriten  FAILED, expected success"
        sys.exit(1)
    
    checkRes = CheckParamOverwriten(cfgParamDebugList)
    if checkRes == False:
        print "check Debug params were overwriten  FAILED, expected success"
        sys.exit(1)













    
#------------------------------------------------------------------------------
#    Test
#------------------------------------------------------------------------------

sysCfgUtils = SyscfgUtilities()
sysCfgUtils.ConnectSpecificUser("SUPPORT", "SUPPORT")

SendInvalidParams(sysCfgUtils)

cfgParamUserList = CreateCfgParamUserList()
SendValidCfg(sysCfgUtils, cfgParamUserList)

cfgParamDebugList = CreateCfgParamDebugList()
SendValidCfg(sysCfgUtils, cfgParamDebugList)

sysCfgUtils.Disconnect()


CheckParamsWereInput()


os.environ["CLEAN_CFG"]="NO"
Reset()


CheckParamsWereOverwriten()


print "Test System Configuration PASSED"


