#!/mcms/python/bin/python

# For list of profiles look at RunTest.sh
#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_1

from McmsConnection import *

#------------------------------------------------------------------------------
def TestDefaultIvrServices(connection, retries):
    
    num_ivr_services = GetNumOfIvrServices(connection)
    
    ivr_name = "IVR1"
    print "Adding new IVR service - ",ivr_name,"..."
    connection.LoadXmlFile('Scripts/DefaultIvrServices/AddIVR.xml')
    connection.ModifyXml("AV_COMMON","SERVICE_NAME",ivr_name)
    connection.Send()
    
    num_ivr_services = WaitUntilIvrServiceIsAdded(connection,num_ivr_services,retries)
    
    print "Set ", ivr_name, " as default IVR service..."
    connection.LoadXmlFile('Scripts/DefaultIvrServices/SetDefaultIvrService.xml')
    connection.ModifyXml("SET_DEFAULT","SERVICE_NAME",ivr_name)
    connection.Send()
    
    TryToDeleteDefaultService(connection,ivr_name)
    
    num_ivr_services = CheckDefaultServiceWasNotDeleted(connection,num_ivr_services,ivr_name)
     
     
    ###EQ 
    ivr_name = "EQ1"   
    
    print "Adding new EQ service - ", ivr_name, " ..."
    connection.LoadXmlFile('Scripts/DefaultIvrServices/AddEQ.xml')
    connection.ModifyXml("AV_COMMON","SERVICE_NAME",ivr_name)
    connection.Send()
    
    #Wait until the new EQ service is added
    num_ivr_services = WaitUntilIvrServiceIsAdded(connection,num_ivr_services,retries)
    
    print "Set ", ivr_name, " as default EQ service..."
    connection.LoadXmlFile('Scripts/DefaultIvrServices/SetDefaultEqService.xml')
    connection.ModifyXml("SET_DEFAULT_EQ","SERVICE_NAME",ivr_name)
    connection.Send()
    
    TryToDeleteDefaultService(connection,ivr_name)
    
    num_ivr_services = CheckDefaultServiceWasNotDeleted(connection,num_ivr_services,ivr_name)
     





#------------------------------------------------------------------------------

def GetNumOfIvrServices(connection):
    print "Monitoring IVR service list..."
    connection.SendXmlFile('Scripts/DefaultIvrServices/GetIvrList.xml')
    ivr_services = connection.xmlResponse.getElementsByTagName("IVR_SERVICE")
    num_ivr_services = len(ivr_services)
    print "num_ivr_services = ", num_ivr_services
    return num_ivr_services


#------------------------------------------------------------------------------

def WaitUntilIvrServiceIsAdded(connection,num_ivr_services,retries):
    #Wait until the new IVR service is added
    for retry in range(retries+1):
        sleep(1)
        connection.SendXmlFile('Scripts/DefaultIvrServices/GetIvrList.xml')
        ivr_services = connection.xmlResponse.getElementsByTagName("IVR_SERVICE")
        num_ivr_services_after_action = len(ivr_services)
        if num_ivr_services+1 == num_ivr_services_after_action:
            print "New IVR/EQ Sevice was added"
            break
        if retry == retries:
            connection.Disconnect()
            sys.exit("Failed to add IVR/EQ service!!!")
    return num_ivr_services_after_action


#------------------------------------------------------------------------------

def TryToDeleteDefaultService(connection,ivr_name):
    print "Trying to delete IVR/EQ service - ",ivr_name,"..."
    connection.LoadXmlFile('Scripts/DefaultIvrServices/DeleteNewIVR.xml')
    connection.ModifyXml("DELETE","SERVICE_NAME",ivr_name)
    connection.Send("Default IVR Service cannot be deleted")



#------------------------------------------------------------------------------

def CheckDefaultServiceWasNotDeleted(connection,num_ivr_services,ivr_name):
    print "Monitoring IVR service list..."
    connection.SendXmlFile('Scripts/DefaultIvrServices/GetIvrList.xml')
    ivr_services = connection.xmlResponse.getElementsByTagName("IVR_SERVICE")
    num_ivr_services_after_action = len(ivr_services)
    if (num_ivr_services_after_action == num_ivr_services):
        print "IVR service ", ivr_name, " failed to be deleted - OK! (default service)"
    else:
        connection.Disconnect()
        sys.exit("ERROR - Default IVR/EQ service was deleted!!!")
    return num_ivr_services_after_action




#------------------------------------------------------------------------------

## ---------------------- Test --------------------------
c = McmsConnection()
c.Connect()

TestDefaultIvrServices(c,
                       50)# retries

#c.Disconnect()




