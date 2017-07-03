#!/mcms/python/bin/python

import os
from McmsConnection import *
from UsersUtils import *
from AlertUtils import *


class IpServiceUtils (McmsConnection) :



# ---------------------------------------------------------------------------------
#        Public Fucntions , Connection done.
# ---------------------------------------------------------------------------------

    def PerformAddService(self, userIndex):
        print "==============PerformAddService"

        userName = "USER" + str(userIndex)
        userPassword = userName
        self.AddConnectOperator(userName, userPassword)

        self.AddServiceAsOperator()

        userName = "SUPPORT" + str(userIndex)
        userPassword = userName
        
        self.AddConnectAdministrator(userName, userPassword)
        self.AddCorruptedServiceAsAdministrator()
        self.AddGoodServiceAsAdministrator()
        
        print "======== add good service second time"
        self.AddService("Scripts/AddIpService.xml", "Maximum number of Services already defined")

        self.Wait(10)
        
        return(0)

    def PerformUpdateService(self, userIndex):
        print "==============PerformUpdateService"

        userName = "USER" + str(userIndex)
        userPassword = userName
        self.AddConnectOperator(userName, userPassword)
        
        self.UpdateServiceAsOperator()

        userName = "SUPPORT" + str(userIndex)
        userPassword = userName
        self.AddConnectAdministrator(userName, userPassword)

        self.UpdateBadServiceAsAdministrator()
        self.UpdateServiceAsAdministrator()
        
        self.Wait(2)
        self.testActiveAlarm("CSMngr", "Manager", "IP_SERVICE_CHANGED", 1 )

        return(0)

    def PerformDeleteService(self, userIndex):
        print "==============PerformDeleteService"

        userName = "USER" + str(userIndex)
        userPassword = userName
        self.AddConnectOperator(userName, userPassword)

        self.DeleteService_AsOperator()

        userName = "SUPPORT" + str(userIndex)
        userPassword = userName
        self.AddConnectAdministrator(userName, userPassword)

        self.DeleteService_CorruptedNameAsAdministrator()

        isShouldSuccess = 1
        self.DeleteGoodServiceAsAdministrator(isShouldSuccess)

        isShouldSuccess = 0
        self.DeleteGoodServiceAsAdministrator(isShouldSuccess)
        
        print "======== add second service"
        self.AddService("Scripts/AddIpService.xml", "Maximum number of Services already defined")
        
        return(0)

    def CheckServiceExist(self, userIndex):
        print "==============CheckServiceExist"
        
        userName = "USER" + str(userIndex)
        userPassword = userName
        self.AddConnectOperator(userName, userPassword)
        
        self.MonitorIpService_Static(1)
        self.MonitorIpService_Dinamic(1)
        
        self.testActiveAlarm("CSMngr", "Manager", "CS_NOT_CONFIGURED", 0 )
        self.testActiveAlarm("Gatekeeper", "Manager", "NO_IP_SERVICE_PARAMS", 0 )
        self.testActiveAlarm("Cards", "Manager", "NO_IP_SERVICE_PARAMS", 0 )
        self.testActiveAlarm("ConfParty", "Manager", "NO_IP_SERVICE_PARAMS", 0 )
        self.testActiveAlarm("CSMngr", "Manager", "CS_NOT_CONFIGURED", 0 )
        
        return(0)

    def CheckServiceUpdated(self, userIndex, isShouldDynamicBeUpdated):
        print "==============CheckServiceUpdated"
        
        userName = "USER" + str(userIndex)
        userPassword = userName
        self.AddConnectOperator(userName, userPassword)

        self.MonitorCheckUpdate_Static()
        self.MonitorCheckUpdate_Dynamic(isShouldDynamicBeUpdated)
        
        return(0)

    def CheckServiceNotExist(self, userIndex):
        print "==============CheckServiceNotExist"

        userName = "USER" + str(userIndex)
        userPassword = userName
        self.AddConnectOperator(userName, userPassword)
        
        self.MonitorIpService_Static(0)
        self.MonitorIpService_Dinamic(0)
        self.testActiveAlarm("CSMngr", "Manager", "NO_SERVICE_FOUND_IN_DB", 1 )
        self.testActiveAlarm("CSMngr", "cucu", "NO_SERVICE_FOUND_IN_DB", 0 )
        
        return(0)

    def CheckServiceDeleted(self, userIndex):
        print "==============CheckServiceDeleted"

        userName = "USER" + str(userIndex)
        userPassword = userName
        self.AddConnectOperator(userName, userPassword)

        self.MonitorIpService_Static(0)
        self.MonitorIpService_Dinamic(1)
        self.testActiveAlarm("CSMngr", "Manager", "IP_SERVICE_DELETED", 1 )
        
        return(0)


    
# ---------------------------------------------------------------------------------
#        Private Fucntions , Connection should done before call to private functions.
# ---------------------------------------------------------------------------------

    def AddService(self, servicePath, expectedStatus = "Status OK"):
        print "==============AddService : " + servicePath + ", Expected status : " + expectedStatus

        self.LoadXmlFile(servicePath)     
        self.Send(expectedStatus)
        
        return(0)

    def AddServiceAsOperator(self):
        print "==============add service as operator"

        self.AddService("Scripts/AddIpService.xml", "You are not authorized to perform this operation")
        self.Wait(2)
        self.MonitorIpService_Static(0)
        self.MonitorIpService_Dinamic(0)
        self.testActiveAlarm("CSMngr", "Manager", "NO_SERVICE_FOUND_IN_DB", 1 )
        self.testActiveAlarm("Cards", "Manager", "NO_IP_SERVICE_PARAMS", 1 )
        self.testActiveAlarm("ConfParty", "Manager", "NO_IP_SERVICE_PARAMS", 1 )
        
        return(0)
    
    def AddGoodServiceAsAdministrator(self):
        print "==============add good service"
        
        self.AddService("Scripts/AddIpService.xml", "Status OK")
        
        return(0)
    
    def AddCorruptedServiceAsAdministrator(self):
        print "==============AddCorruptedService"
        
        self.AddService_NoSpans()
        self.Wait(2)
        self.MonitorIpService_Static(0)
        self.MonitorIpService_Dinamic(0)
        self.testActiveAlarm("CSMngr", "Manager", "NO_SERVICE_FOUND_IN_DB", 1 )
        self.testActiveAlarm("Cards", "Manager", "NO_IP_SERVICE_PARAMS", 1 )
        self.testActiveAlarm("ConfParty", "Manager", "NO_IP_SERVICE_PARAMS", 1 )

        print "=============="
        self.AddService_CorruptedGKPrefixNotNumeric()
        self.Wait(2)
        self.MonitorIpService_Static(0)
        self.MonitorIpService_Dinamic(0)
        self.testActiveAlarm("CSMngr", "Manager", "NO_SERVICE_FOUND_IN_DB", 1 )
        self.testActiveAlarm("Cards", "Manager", "NO_IP_SERVICE_PARAMS", 1 )
        self.testActiveAlarm("ConfParty", "Manager", "NO_IP_SERVICE_PARAMS", 1 )

        print "=============="
        self.AddService_CorruptedGKPrefixTooManyDigits()
        self.Wait(2)
        self.MonitorIpService_Static(0)
        self.MonitorIpService_Dinamic(0)
        self.testActiveAlarm("CSMngr", "Manager", "NO_SERVICE_FOUND_IN_DB", 1 )
        self.testActiveAlarm("Cards", "Manager", "NO_IP_SERVICE_PARAMS", 1 )
        self.testActiveAlarm("ConfParty", "Manager", "NO_IP_SERVICE_PARAMS", 1 )

#        print "=============="
#        self.AddService_CorruptedGKPrefixTooBig()
#        self.Wait(2)
#        self.MonitorIpService_Static(0)
#        self.MonitorIpService_Dinamic(0)
#        self.testActiveAlarm("CSMngr", "Manager", "NO_SERVICE_FOUND_IN_DB", 1 )
#        self.testActiveAlarm("Cards", "Manager", "NO_IP_SERVICE_PARAMS", 1 )
#        self.testActiveAlarm("ConfParty", "Manager", "NO_IP_SERVICE_PARAMS", 1 )

        print "=============="
        self.AddService_CorruptedGKNoPrefixNoAlias()
        self.Wait(2)
        self.MonitorIpService_Static(0)
        self.MonitorIpService_Dinamic(0)
        self.testActiveAlarm("CSMngr", "Manager", "NO_SERVICE_FOUND_IN_DB", 1 )
        self.testActiveAlarm("Cards", "Manager", "NO_IP_SERVICE_PARAMS", 1 )
        self.testActiveAlarm("ConfParty", "Manager", "NO_IP_SERVICE_PARAMS", 1 )

        return(0)

    def AddService_NoSpans(self):
        print "==============AddService_NoSpans"
        
        self.LoadXmlFile("Scripts/AddIpServiceWithoutSpans.xml")
        self.Send("Span definition in the Network Service is missing")
        
        return(0)
    
    def AddService_CorruptedGKPrefixNotNumeric(self):
        print "==============AddService_CorruptedGKPrefixNotNumeric"
        
        self.LoadXmlFile("Scripts/IpServiceWithBadGk.xml")
        self.ModifyXml("GATEKEEPER","NAME", "cucu-lulu")
        self.Send("MCU Prefix in gatekeeper must be numeric")

        return(0)

    def AddService_CorruptedGKPrefixTooManyDigits(self):
        print "==============AddService_CorruptedGKPrefixTooManyDigits"

        self.LoadXmlFile("Scripts/IpServiceWithBadGk.xml")
        overLimit = "012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789"
        self.ModifyXml("GATEKEEPER","NAME", overLimit)
        self.Send("MCU Prefix in gatekeeper contains too many digits")
        
        return(0)

    def AddService_CorruptedGKPrefixTooBig(self):
        print "==============AddService_CorruptedGKPrefixTooBig"

        self.LoadXmlFile("Scripts/IpServiceWithBadGk.xml")
        overLimit = 2147483647 + 1
        self.ModifyXml("GATEKEEPER","NAME", overLimit)
        self.Send("MCU Prefix in gatekeeper is too long")

        return(0)
    
    def AddService_CorruptedGKNoPrefixNoAlias(self):
        print "==============AddService_CorruptedGKNoPrefixNoAlias"

        self.LoadXmlFile("Scripts/IpServiceWithBadGk.xml")
        self.ModifyXml("GATEKEEPER","NAME", "")
        self.ModifyXml("IP_SPAN","NAME", "")
        self.Send("Gatekeeper alias is missing")

        return(0)

    def UpdateBadServiceAsAdministrator(self):
        print "============== UpdateBadServiceAsAdministrator"

        self.LoadXmlFile("Scripts/UpdateIPService.xml")
        self.ModifyXml("IP_SERVICE","NAME", "CUCU_LULU")
        self.Send("Network Service does not exist")
        
        return(0)
    
    def UpdateServiceAsAdministrator(self):
        print "==============UpdateServiceAsAdministrator"

        self.LoadXmlFile("Scripts/UpdateIPService.xml")
        self.ModifyXml("IP_SERVICE","MASK", "1.2.3.4")
        self.Send("Status OK")

        self.Wait(2)
        self.testActiveAlarm("CSMngr", "Manager", "IP_SERVICE_CHANGED", 1 )
        
        return(0)

    def UpdateServiceAsOperator(self):
        print "==============UpdateServiceAsOperator"

        self.LoadXmlFile("Scripts/AddIpService.xml")
        self.ModifyXml("IP_SERVICE","MASK", "1.2.3.4")
        self.Send("You are not authorized to perform this operation")

        return(0)
    
    def DeleteService(self, servicePath, expectedStatus = "Status OK"):
        print "==============DeleteService : " + servicePath + ", Expected status : " + expectedStatus

        self.LoadXmlFile(servicePath)
        self.Send(expectedStatus)
        
        return(0)

    def DeleteService_AsOperator(self):
        print "==============DeleteService_AsOperator"

        self.LoadXmlFile("Scripts/DeleteIpService.xml")
        self.Send("You are not authorized to perform this operation")

        self.Wait(2)
        self.MonitorIpService_Static(1)
        self.MonitorIpService_Dinamic(1)
        self.testActiveAlarm("CSMngr", "Manager", "IP_SERVICE_DELETED", 0 )
        
        return(0)

    def DeleteService_CorruptedNameAsAdministrator(self):
        print "==============DeleteService_CorruptedNameAsAdministrator"

        self.LoadXmlFile("Scripts/DeleteIpService.xml")
        self.ModifyXml("DEL_IP_SERVICE","NAME", "cucu-lulu")
        self.Send("Service provider name not found")

        self.Wait(2)
        self.MonitorIpService_Static(1)
        self.MonitorIpService_Dinamic(1)
        self.testActiveAlarm("CSMngr", "Manager", "IP_SERVICE_DELETED", 0 )
        
        return(0)
    
    def DeleteGoodServiceAsAdministrator(self, isShouldSuccess):
        print "==============DeleteGoodServiceAsAdministrator"

        self.LoadXmlFile("Scripts/DeleteIpService.xml")
        
        expectedStatus = "Service provider name not found"
        if(isShouldSuccess == 1):
            expectedStatus = "Status OK"
            
        self.Send(expectedStatus)

        self.Wait(2)
        
        return(0)
    
    def MonitorIpService_Static(self, expectedNumOfServices):
        print "==============MonitoripService_Static : Expected num Of Services : " + str(expectedNumOfServices)

        self.LoadXmlFile("Scripts/GetIpServiceMonitor.xml")
        self.Send("Status OK")

        service = self.xmlResponse.getElementsByTagName("IP_SERVICE")
        if(len(service) <> expectedNumOfServices):
            sys.exit(1)        
        return(0)
    
    def MonitorIpService_Dinamic(self, expectedNumOfServices):
        print "==============MonitorIpService_Dinamic : Expected num Of Services : " + str(expectedNumOfServices)
 
        self.LoadXmlFile("Scripts/GetIpServiceMonitorDynamic.xml")
        self.Send("Status OK")

        service = self.xmlResponse.getElementsByTagName("FULL_IP_SERVICE")
        if(len(service) <> expectedNumOfServices):
            sys.exit(1)
            
        return(0)

    def MonitorCheckUpdate_Static(self):
        print "============== MonitorCheckUpdate_Static"

        self.LoadXmlFile("Scripts/GetIpServiceMonitor.xml")
        self.Send("Status OK")

        serviceMask = self.xmlResponse.getElementsByTagName(
            "IP_SERVICE")[0].getElementsByTagName("MASK")[0].firstChild.data

        if(serviceMask != "1.2.3.4"):
           sys.exit(1) 

        return(0)

    def MonitorCheckUpdate_Dynamic(self, isShouldBeNew):
        strIsShouldBeNew = "Should be new"
        if(0 == isShouldBeNew):
            strIsShouldBeNew = "Should not be new"
        print "============== MonitorCheckUpdate_Dynamic : " + strIsShouldBeNew

        self.LoadXmlFile("Scripts/GetIpServiceMonitorDynamic.xml")
        self.Send("Status OK")

        serviceMask = self.xmlResponse.getElementsByTagName(
            "IP_SERVICE")[0].getElementsByTagName("MASK")[0].firstChild.data

        if(1 == isShouldBeNew):
            if(serviceMask != "1.2.3.4"):
                sys.exit(1) 
        else:
            if(serviceMask == "1.2.3.4"):
                sys.exit(1)
                
        return(0)

    
    def AddConnectAdministrator(self, userName, userPassword):
        print "==============AddConnectAdministrator"

        userUtils = UsersUtilities()
        userUtils.Connect()
        userUtils.AddNewUser(userName, userPassword, "administrator")
        self.ConnectSpecificUser(userName, userPassword)
        
        return(0)

    def AddConnectOperator(self, userName, userPassword):
        print "==============AddConnectOperator"

        userUtils = UsersUtilities()
        userUtils.Connect()
        userUtils.AddNewUser(userName, userPassword, "operator")
        self.ConnectSpecificUser(userName, userPassword)
        
        return(0)
    
    def testFault(self, expectedProcessName, expectedFault, expectedFaultNum = 1):
          print "==============testFault : "+ expectedProcessName + ", Expected fault : " + expectedFault + "("+str(expectedFaultNum) +")."

          self.SendXmlFile("Scripts/GetFaults.xml")
          fault_elm_list = self.xmlResponse.getElementsByTagName("LOG_FAULT_ELEMENT")
          num_of_faults = len(fault_elm_list)
          wantedFaultsCnt = 0

          for index in range(len(fault_elm_list)):
              fault_desc = fault_elm_list[index].getElementsByTagName("DESCRIPTION")[0].firstChild.data
              proc_name = fault_elm_list[index].getElementsByTagName("PROCESS_NAME")[0].firstChild.data
              if fault_desc == expectedFault and proc_name == expectedProcessName:
                  wantedFaultsCnt = wantedFaultsCnt + 1

          if wantedFaultsCnt <> expectedFaultNum :
              print "==============Test failed, "
              sys.exit(1)
              
          return(0)  

    def testActiveAlarm(self, processName, taskName,  wantedActiveAlarm, expectedAANum = 1):
        alertsUtils = AlertUtils()
        bAlertCheck = alertsUtils.IsAlertExist(processName, taskName,  wantedActiveAlarm, expectedAANum)
        if bAlertCheck == False:
            sys.exit(1)
          
      
    def Wait(self, seconds):
        index = 0
        sys.stdout.write("Sleep " + str(seconds) + " seconds : ")
        sys.stdout.flush()
        while index < seconds:
            sys.stdout.write(".")
            sys.stdout.flush()
            sleep(1)
            index = index + 1
        print ""
        
 
    def WaitUntilStartupEnd(self):
        print "==============Wait until startup ends"
        retStatus = 1
        title = "Card startup failed"
        for x in range(9999999):
            self.SendXmlFile('Scripts/GetMcuState.xml')
            state = self.GetTextUnder("MCU_STATE","DESCRIPTION")
            print "Mcu status:" + state
            if state != "Startup":
                title = "Card startup OK"
                retStatus = 0
                break
            sleep(2)
                
        print title
        return retStatus
