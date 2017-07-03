#!/mcms/python/bin/python
#################################################################
#  Script tests the resource disabling functionality by using
#  simulation of MFA's component fatal indication.
#################################################################
# For list of profiles look at RunTest.sh
#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_6

#*export MPL_SIM_FILE="VersionCfg/MPL_SIM_2.XML"
#*export SYSTEM_CARDS_MODE_FILE="VersionCfg/SystemCardsMode_mpm.txt"

# For list of processes not for valgrind look at RunTest.sh
#*PROCESSES_NOT_FOR_VALGRIND=Logger Resource GideonSim EndpointsSim MplApi CSApi


from ResourceUtilities import *

c = ResourceUtilities()
c.Connect()

#command_line = "Bin/McuCmd set Cards DEBUG_MODE YES" + '\n'
#os.system(command_line) no need anymore


print "Monitor active alarms..."
c.SendXmlFile("Scripts/GetActiveAlarms.xml")

fault_elm_list = c.xmlResponse.getElementsByTagName("LOG_FAULT_ELEMENT")

um_of_faults = len(fault_elm_list)
 
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
print "Send Set unit status to BOARD_ID:1, UNIT_ID:1,STATUS:3 (eFatal) to influence keep alive indication..."
#c.SendXmlFile('Scripts/keep_allive/SetUnitStatus.xml')
#c.LoadXmlFile('Scripts/keep_allive/SetUnitStatus.xml') 

#for  i in range(1):

c.LoadXmlFile('Scripts/keep_allive/SetUnitStatus.xml') 
c.ModifyXml("SET_UNIT_STATUS_FOR_KEEP_ALIVE","BOARD_ID",1)
c.ModifyXml("SET_UNIT_STATUS_FOR_KEEP_ALIVE","UNIT_ID",1)
c.ModifyXml("SET_UNIT_STATUS_FOR_KEEP_ALIVE","STATUS",3)
c.Send()

print "wait 12 seconds..."
sleep (12)

# take unit's detail for dsp No.1.
c.SendXmlFile("Scripts/ResourceDetailParty/CardDetailReq.xml")
unit_summary = c.xmlResponse.getElementsByTagName("UNIT_RESOURCE_DESCRIPTOR")
unit_len =len(unit_summary)
if (unit_len == 0):
  sys.exit("error" + " - unit's detail list is empty" )
  
if unit_summary[0].getElementsByTagName("UNIT_TYPE")[0].firstChild.data == "video":
      v_disable = unit_summary[0].getElementsByTagName("ID")[0].firstChild.data
      v_util = unit_summary[0].getElementsByTagName("UTILIZATION")[0].firstChild.data
      if (int(v_disable) == 0 ):
         print " v_disable = " + str(v_disable)
         sys.exit("error" + " - unit isn't disabled in Resource Allocator" )
#print "Monitor active alarms after the keep_alive_ind with the bad unit..."
#c.SendXmlFile("Scripts/GetActiveAlarms.xml")

#fault_elm_list = c.xmlResponse.getElementsByTagName("LOG_FAULT_ELEMENT")
#num_of_faults = len(fault_elm_list)

#for index in range(len(fault_elm_list)):
#    fault_desc = fault_elm_list[index].getElementsByTagName("DESCRIPTION")[0].firstChild.data
#    print fault_desc
#    if fault_desc == "UNIT_NOT_RESPONDING":
#        compone_fatal = compone_fatal+1
            
#if compone_fatal== 0:
#    print "Test failed,  no UNIT_NOT_RESPONDING found in active alarms after that keep_alive_ind has sent with bad unit"
#    sys.exit(1)

#if compone_fatal > 1:
#    print "Test failed, More than 1 UNIT_NOT_RESPONDING found in active alarms after  that keep_alive_ind has sent with bad unit"
   # sys.exit(1) 
    
#if compone_fatal== 1:
#    print "We are on the right way !!!, 1 UNIT_NOT_RESPONDING found in active alarms after that keep_alive_ind has sent with bad unit"


############################# RESOURCE TEST WITH DISABLED ##################################################################
c.SendXmlFile("Scripts/ResourceFullCapacity/ResourceMonitorCarmel.xml")
num_of_resource = c.GetTotalCarmelParties()

print "RSRC_REPORT_RMX[Total] = " + str(num_of_resource)

c.TestFreeCarmelParties(num_of_resource)

num_without_dsbl = int(num_of_resource) - 2
c.SimpleXmlConfPartyTest('Scripts/ResourceFullCapacity/AddVideoCpConf.xml',
                         'Scripts/ResourceFullCapacity/AddVideoParty1.xml',
                         int(num_without_dsbl),
                         50,
                         "FALSE")
                         
# take unit's detail for dsp No.1.
c.SendXmlFile("Scripts/ResourceDetailParty/CardDetailReq.xml")
unit_summary = c.xmlResponse.getElementsByTagName("UNIT_RESOURCE_DESCRIPTOR")
unit_len =len(unit_summary)
if (unit_len == 0):
  sys.exit("error" + " - unit's detail list is empty" )
  
if unit_summary[0].getElementsByTagName("UNIT_TYPE")[0].firstChild.data == "video":
      v_disable = unit_summary[0].getElementsByTagName("ID")[0].firstChild.data
      v_util = unit_summary[0].getElementsByTagName("UTILIZATION")[0].firstChild.data
      if (int(v_disable) == 0 ) | ( int(v_util) != 65535 ):
         print " v_disable = " + str(v_disable)
         print " v_util = " + str(v_util)
         sys.exit("error" + " - unit isn't disabled in Resource Allocator" )
      
c.TestFreeCarmelParties( 2 ) 
c.TestOccupiedCarmelParties(int( num_without_dsbl))

c.DeleteAllConf()
c.WaitAllConfEnd(100)

c.TestFreeCarmelParties(int(num_of_resource))
 
#################SEND UNIT FIXED STATUS####################################################
#print "Send Set unit status to BOARD_ID:1, UNIT_ID:2,STATUS:0 (eOk) to influence keep alive indication..."
#c.SendXmlFile('Scripts/keep_allive/SetUnitStatus.xml')
#c.LoadXmlFile('Scripts/keep_allive/SetUnitStatus.xml') 
#c.ModifyXml("SET_UNIT_STATUS_FOR_KEEP_ALIVE","BOARD_ID",1)
#c.ModifyXml("SET_UNIT_STATUS_FOR_KEEP_ALIVE","UNIT_ID",2)
#c.ModifyXml("SET_UNIT_STATUS_FOR_KEEP_ALIVE","STATUS",0)
#c.Send()
#print "wait 50 seconds..."
#sleep (50)

#print "Monitor active alarms after the keep_alive_ind with the bad unit..."
#c.SendXmlFile("Scripts/GetActiveAlarms.xml")

#fault_elm_list = c.xmlResponse.getElementsByTagName("LOG_FAULT_ELEMENT")
#num_of_faults = len(fault_elm_list)

#for index in range(len(fault_elm_list)):
#    fault_desc = fault_elm_list[index].getElementsByTagName("DESCRIPTION")[0].firstChild.data
#    print fault_desc
#    if fault_desc == "UNIT_NOT_RESPONDING":
#        print "Test failed, UNIT_NOT_RESPONDING found in active alarms 30 the unit status has set to OK."
#        sys.exit(1)
#print "Test passed The UNIT_NOT_RESPONDING removed from the Active alarm"            
c.Disconnect()


