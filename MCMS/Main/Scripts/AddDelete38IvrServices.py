#!/mcms/python/bin/python

# For list of profiles look at RunTest.sh
#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_No_Valgrind

from McmsConnection import *


c = McmsConnection()
c.Connect()
retires = 3
num_of_ivr_services = 37

print "Monitoring IVR service list..."
c.SendXmlFile('Scripts/AddDelete38IvrServices/GetIvrList.xml')
ivr_services = c.xmlResponse.getElementsByTagName("IVR_SERVICE")
num_ivr_services = len(ivr_services)
print "num_ivr_services = ", num_ivr_services

for index in range(num_of_ivr_services):
    ivr_name = "IVR_" + str(index+1)
    print "Adding new IVR service - ",ivr_name,"..."
    c.LoadXmlFile('Scripts/AddDelete38IvrServices/AddIVR.xml')
    c.ModifyXml("AV_COMMON","SERVICE_NAME",ivr_name)
    c.Send()
    
    #Wait until the new IVR service is added
    for retry in range(retires+1):
        sleep(1)
        c.SendXmlFile('Scripts/AddDelete38IvrServices/GetIvrList.xml')
        ivr_services = c.xmlResponse.getElementsByTagName("IVR_SERVICE")
        num_ivr_services_after_action = len(ivr_services)
        if num_ivr_services+index+1 == num_ivr_services_after_action:
            print "New IVR Sevice was added"
            break
        if retry == retires:
            c.Disconnect()
            sys.exit("Failed to add IVR service!!!")


for index in range(num_of_ivr_services):
    ivr_name = "IVR_" + str(index+1)
    print "Deleting IVR service - ",ivr_name,"..."
    c.LoadXmlFile('Scripts/AddDelete38IvrServices/DeleteNewIVR.xml')
    c.ModifyXml("DELETE","SERVICE_NAME",ivr_name)
    c.Send()

#Wait until all IVR services are deleted
for retry in range(retires+1):
    sleep(1)
    c.SendXmlFile('Scripts/AddDelete38IvrServices/GetIvrList.xml')
    ivr_services = c.xmlResponse.getElementsByTagName("IVR_SERVICE")
    num_ivr_services_after_action = len(ivr_services)
    print "num_ivr_services = ", num_ivr_services_after_action
    if num_ivr_services == num_ivr_services_after_action:
        print "All IVR Sevices were deleted"
        break
    if retry == retires:
        c.Disconnect()
        sys.exit("Failed to delete IVR service!!!")

c.Disconnect()

