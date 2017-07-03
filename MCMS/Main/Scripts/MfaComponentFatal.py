#!/mcms/python/bin/python
# For list of profiles look at RunTest.sh
#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_6

from McmsConnection import *
 
c = McmsConnection()
c.Connect()

print "Monitor active alarms..."
c.SendXmlFile("Scripts/GetActiveAlarms.xml")

fault_elm_list = c.xmlResponse.getElementsByTagName("LOG_FAULT_ELEMENT")

num_of_faults = len(fault_elm_list)
 
compone_fatal = 0
 
for index in range(len(fault_elm_list)):
    fault_desc = fault_elm_list[index].getElementsByTagName("DESCRIPTION")[0].firstChild.data
    print fault_desc
    if fault_desc == "UNIT_NOT_RESPONDING":
        compone_fatal = compone_fatal+1
 
if compone_fatal <> 0:
    print "Test failed, UNIT_NOT_RESPONDING found in active alarms BEFORE the test started"
    sys.exit(1)
#################SEND BAD  UNIT STATUS####################################################
print "Send Set unit status to BOARD_ID:1, UNIT_ID:2,STATUS:3 (eFatal) to influence keep alive indication..."
#c.SendXmlFile('Scripts/keep_allive/SetUnitStatus.xml')
c.LoadXmlFile('Scripts/keep_allive/SetUnitStatus.xml') 
c.ModifyXml("SET_UNIT_STATUS_FOR_KEEP_ALIVE","BOARD_ID",1)
c.ModifyXml("SET_UNIT_STATUS_FOR_KEEP_ALIVE","UNIT_ID",2)
c.ModifyXml("SET_UNIT_STATUS_FOR_KEEP_ALIVE","STATUS",3)
c.Send()
print "wait 11 seconds..."
sleep (11)

print "Monitor active alarms after the keep_alive_ind with the bad unit..."
c.SendXmlFile("Scripts/GetActiveAlarms.xml")

fault_elm_list = c.xmlResponse.getElementsByTagName("LOG_FAULT_ELEMENT")
num_of_faults = len(fault_elm_list)

for index in range(len(fault_elm_list)):
    fault_desc = fault_elm_list[index].getElementsByTagName("DESCRIPTION")[0].firstChild.data
    print fault_desc
    if fault_desc == "UNIT_NOT_RESPONDING":
        compone_fatal = compone_fatal+1
            
if compone_fatal== 0:
    print "Test failed,  no UNIT_NOT_RESPONDING found in active alarms after that keep_alive_ind has sent with bad unit"
    sys.exit(1)

if compone_fatal > 1:
    print "Test failed, More than 1 UNIT_NOT_RESPONDING found in active alarms after  that keep_alive_ind has sent with bad unit"
    sys.exit(1) 
    
if compone_fatal== 1:
    print "We are on the right way !!!, 1 UNIT_NOT_RESPONDING found in active alarms after that keep_alive_ind has sent with bad unit"
   
 #################SEND UNIT FIXED STATUS####################################################
print "Send Set unit status to BOARD_ID:1, UNIT_ID:2,STATUS:0 (eOk) to influence keep alive indication..."
#c.SendXmlFile('Scripts/keep_allive/SetUnitStatus.xml')
c.LoadXmlFile('Scripts/keep_allive/SetUnitStatus.xml') 
c.ModifyXml("SET_UNIT_STATUS_FOR_KEEP_ALIVE","BOARD_ID",1)
c.ModifyXml("SET_UNIT_STATUS_FOR_KEEP_ALIVE","UNIT_ID",2)
c.ModifyXml("SET_UNIT_STATUS_FOR_KEEP_ALIVE","STATUS",0)
c.Send()
print "wait 11 seconds..."
sleep (11)

print "Monitor active alarms after the keep_alive_ind with the bad unit..."
c.SendXmlFile("Scripts/GetActiveAlarms.xml")

fault_elm_list = c.xmlResponse.getElementsByTagName("LOG_FAULT_ELEMENT")
num_of_faults = len(fault_elm_list)

for index in range(len(fault_elm_list)):
    fault_desc = fault_elm_list[index].getElementsByTagName("DESCRIPTION")[0].firstChild.data
    print fault_desc
    if fault_desc == "UNIT_NOT_RESPONDING":
        print "Test failed, UNIT_NOT_RESPONDING found in active alarms 30 the unit status has set to OK."
        sys.exit(1)
print "Test passed The UNIT_NOT_RESPONDING removed from the Active alarm"            
c.Disconnect()


