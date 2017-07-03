#!/mcms/python/bin/python
#-LONG_SCRIPT_TYPE

#*Script_Info_Name="MultipleService_MaxCifPartiesTest.py"
#*Script_Info_Group="ConfParty"
#*Script_Info_Programmer="Ron"
#*Script_Info_Version="V7.1"
#*Script_Info_Description="Max CIF participants in Multiple Service system (180 parties)"


# For list of profiles look at RunTest.sh
#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_1
#*export RESOURCE_SETTING_FILE="VersionCfg/Resource_Setting_180Video.xml"


from SysCfgUtils import *
from ResourceUtilities import *
from AmosCapacity_functions import *

#------------------------------------------------------------------------------

def TryAddPartyToService( c, partyFile, partyName, partyIp, serviceName, confId,num_retries=60):

    success = False

    ## send add party
    c.LoadXmlFile(partyFile)
    print "Adding Party " + partyName + " (ip " + partyIp + ") to confID " + str(confId)
    c.ModifyXml("PARTY","NAME",partyName)
    c.ModifyXml("PARTY","IP",partyIp)
    c.ModifyXml("PARTY","SERVICE_NAME",serviceName)
    c.ModifyXml("ADD_PARTY","ID",confId)

    c.Send()

    ## sleep(1) #for party to be connected
    for retry in range(num_retries):

        ## get conf
        c.LoadXmlFile('Scripts/TransConf2.xml')
        c.ModifyXml("GET","ID",confId)
        c.Send()

        ongoing_parties = c.xmlResponse.getElementsByTagName("ONGOING_PARTY_STATUS")
        ongoing_party_list = c.xmlResponse.getElementsByTagName("ONGOING_PARTY")
     
        party_index = 0
        for party in ongoing_parties:
            ## find party name in party list
            current_party_name = ongoing_party_list[party_index].getElementsByTagName("NAME")[0].firstChild.data
            ## print "current_party_name = " + current_party_name + " , partyName = " + partyName
            if partyName == current_party_name:
                ## wait for party to be connected
                status = party.getElementsByTagName("DESCRIPTION")[0].firstChild.data
                if status == "connected":
                    success = True
                    break
                else:
                    sleep(1/100)
            party_index =  party_index + 1
        if success == True:
            break

    return success

#------------------------------------------------------------------------------
    
def TryAddConf128k(c, profileID, confName,confFile,retires = 60):

    success = False
    print "Adding Conf " + confName + " ..."
    c.LoadXmlFile(confFile)
    c.ModifyXml("RESERVATION","NAME",confName)
    c.ModifyXml("RESERVATION","DISPLAY_NAME",confName)
    c.ModifyXml("RESERVATION","AD_HOC_PROFILE_ID",profileID) 
    c.Send()

    print "Wait untill Conf \'" + confName + "\' is created...",
    for retry in range(retires+1):
        c.LoadXmlFile('Scripts/TransConfList.xml')
        c.Send()
        ongoing_conf_list = c.xmlResponse.getElementsByTagName("CONF_SUMMARY")
        for index in range(len(ongoing_conf_list)):  
            if(confName == ongoing_conf_list[index].getElementsByTagName("NAME")[0].firstChild.data):
                confid = ongoing_conf_list[index].getElementsByTagName("ID")[0].firstChild.data
                if confid != "":
                    success = True
                    break
        if success == True:
            break

    return success

#------------------------------------------------------------------------------

def MaxCapacityPartiesTest(c, profId, num_of_confrences=1,
                        num_of_h323_video_dial_out_per_conf=0,
                        confFile='Scripts/AddVoipConf.xml',
                        h323VoipDialOutPartyFile='Scripts/AddVoipParty1.xml',
                        h323VoipDialInPartyFile='Scripts/SimAdd323Party.xml',
                        h323VideoDialOutPartyFile='Scripts/AddVideoParty1.xml',
                        h323VideoDialInPartyFile='Scripts/SimAdd323Party.xml'):

    ## constants
    num_retries = 10

    start_time = time()
    last_party_succeeded = True    
    
    number_of_parties_connected = 0

    confId = 0
    confName = "empty"
    
    for conf_index in range(0,num_of_confrences):
        ## create new conf
        confName = "Conf_"+str(conf_index+1)
        add_conf_ok = TryAddConf128k(c, profId, confName, confFile, num_retries)
        if add_conf_ok == False:
            print "Failed to add conf " + confName
            return number_of_parties_connected
        confId = GetConfId(c,confName,num_retries)
          
        ## connect h323 video dial out parties
        if num_of_h323_video_dial_out_per_conf>0:
            print "connecting h323 video dial out parties"
        for h323_video_party_out_index in range(0,num_of_h323_video_dial_out_per_conf):
            
            partyName = "Party_" + str(number_of_parties_connected+1)
            partyIp = GetIpAdressString(c,number_of_parties_connected+1) 

	    if h323_video_party_out_index%2 == 0:
		serviceName = "Default IP Service"
	    else:
		serviceName = "ip2"

            last_party_succeeded = TryAddPartyToService(c, h323VideoDialOutPartyFile, partyName, partyIp, serviceName, confId)
 
            if last_party_succeeded == False:
                print "Failed to add party " + partyName
                return number_of_parties_connected
            number_of_parties_connected = number_of_parties_connected + 1


    print "Result: succeeded to connect " + str(number_of_parties_connected) + " participants"
    
    end_time = time()
    total_time = end_time-start_time
    print "All Parties connected in " + ('%.03f' %(total_time)) + " seconds"

    sleep(2)

    return number_of_parties_connected

#------------------------------------------------------------------------------


def TestResourceServicesReportPortsType(c, serviceName, testedResourcePortType, expectedResult,key):

    c.SendXmlFile("Scripts/ResourceFullCapacity/ResourceMonitorCarmelServices.xml")

    rsrc_report_service_list = c.xmlResponse.getElementsByTagName("RSRC_SERVICES_REPORT")

    for index in range(len(rsrc_report_service_list)):
	if serviceName == rsrc_report_service_list[index].getElementsByTagName("SERVICE_NAME")[0].firstChild.data:
	    rsrc_report_rmx_list = rsrc_report_service_list[index].getElementsByTagName("RSRC_REPORT_RMX")
	    for i in range(len(rsrc_report_rmx_list)):
	        resourceType = c.GetTextUnder("RSRC_REPORT_RMX","RSRC_REPORT_ITEM",i)
	        if resourceType!="":
	            if resourceType==testedResourcePortType:
	     	        result = c.GetTextUnder("RSRC_REPORT_RMX",key,i)
		        if (expectedResult!=int(result)):
			    print "Failed test for Resource Port TYPE = " + resourceType + ". Result = " + str(result) + ", expected result = " + str(expectedResult)
			    print c.xmlResponse.toprettyxml(encoding="utf-8")
			    sys.exit("Unexpected resource report")
	                    c.Disconnect()
	    		print "Verified that " + testedResourcePortType + " in resource report of service \"" + serviceName + "\" is " + str(expectedResult) + " for " + key

                 
#------------------------------------------------------------------------------

profId = 0

for yesno in range(0,2):
    print "================================================================"
    r = SyscfgUtilities()
    r.Connect()

    if yesno == 1:
	print "Updating system cfg flag of MULTIPLE_SERVICES to YES"
	r.UpdateSyscfgFlag("MCMS_PARAMETERS_USER","MULTIPLE_SERVICES","YES","user")
    else:
	print "Updating system cfg flag of MULTIPLE_SERVICES to NO"
	r.UpdateSyscfgFlag("MCMS_PARAMETERS_USER","MULTIPLE_SERVICES","NO","user")

    r.Disconnect()
    os.environ["CLEAN_CFG"]="NO"
    os.system("Scripts/Startup.sh")
    print "================================================================"

    c = ResourceUtilities()
    c.Connect()

    if yesno == 0:
	profId = c.AddProfileWithRate("ProfRate128",128)

    confs = 2
    h323_video_out = 90 # participants per conf

    MaxCapacityPartiesTest(c, profId, confs, h323_video_out)

    print "================================================================"
    c.TestResourcesReportPortsType("video",180,"TOTAL")
    c.TestResourcesReportPortsType("video",180,"OCCUPIED")
    c.TestResourcesReportPortsType("video",0,"FREE")

    if yesno == 1:
	serviceName1 = "Default IP Service"
	serviceName2 = "ip2"
	print "================================================================"
	TestResourceServicesReportPortsType(c, serviceName1, "video",90,"TOTAL")
	TestResourceServicesReportPortsType(c, serviceName1, "video",90,"OCCUPIED")
	TestResourceServicesReportPortsType(c, serviceName1, "video",0,"FREE")
	print "================================================================"
	TestResourceServicesReportPortsType(c, serviceName2, "video",90,"TOTAL")
	TestResourceServicesReportPortsType(c, serviceName2, "video",90,"OCCUPIED")
	TestResourceServicesReportPortsType(c, serviceName2, "video",0,"FREE")
	print "================================================================"

    sleep(1)
    c.DeleteAllConf(1)
    c.Disconnect()


