#!/mcms/python/bin/python

# For list of profiles look at RunTest.sh
#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_1


#############################################################################
# Test Script For NxM (VSW) in RMX2000C
# Date: 19/10/09
# By  : Olga
#############################################################################

from ResourceUtilities import *
from HDFunctions import *
import string 


def ConnectVideoParty(connection, conf_id, num_of_parties, num_retries):
    for x in range(num_of_parties):
	#------Dial out H323 party    
	partyname = "Party_"+str(x+1)
        partyip = "1.2.3." + str(x+1) 
	connection.AddVideoParty(conf_id, partyname, partyip)
        print "Connecting H323 Dial out Party " + partyname

    connection.WaitAllPartiesWereAdded(conf_id, num_of_parties, num_retries*2)

    connection.WaitAllOngoingConnected(conf_id)
    connection.WaitAllOngoingNotInIVR(conf_id)

#------------------------------------------------------------------------------

def TestNxM(connection,num_retries):
    
    print "Adding HD Profile 1080"
    prof_id_hd1080 = AddHdProfile(connection,"HD1080_profile_1920","1920", "Scripts/HD/XML/AddHdProfile.xml")

    print "Adding HD Profile H264"
    prof_id_h264 = AddHdProfile(connection,"H264_profile_768k","768", "Scripts/HD/XML/AddVswHd1080Profile.xml", "h264cif")

    print "Adding HD Profile H263"
    prof_id_h263 = AddHdProfile(connection,"H263_profile_384k","384", "Scripts/HD/XML/AddVswHd1080Profile.xml", "h263cif")

    print "Adding HD Profile SD"
    prof_id_sd = AddHdProfile(connection,"SD_profile_1024k","1024", "Scripts/HD/XML/AddVswHd1080Profile.xml", "sd")

    print "Adding HD Profile H720p30"
    prof_id_hd720p30 = AddHdProfile(connection,"HD720p30_profile_1920","1920", "Scripts/HD/XML/AddVswHd1080Profile.xml", "hd_720p30")

    print "Adding HD Profile H720p60"
    prof_id_hd720p60 = AddHdProfile(connection,"HD720p60_profile_4096","4096", "Scripts/HD/XML/AddVswHd1080Profile.xml", "hd_720p60")

    prof_id_list = [ prof_id_hd1080, prof_id_h264, prof_id_h263, prof_id_sd, prof_id_hd720p30, prof_id_hd720p60]
    num_of_profiles = 6
    conf_id_list=[1,2,3,4,5,6]

    for p in range(num_of_profiles):
        confName = "VSW_"+str(prof_id_list[p])

        print "Adding Conf " + confName + " ..."
        connection.LoadXmlFile('Scripts/AddCpConf.xml')
        connection.ModifyXml("RESERVATION","NAME",confName)
        connection.ModifyXml("RESERVATION","AD_HOC_PROFILE_ID",prof_id_list[p]) 
        connection.ModifyXml("RESERVATION","MAX_PARTIES",str(28))
        connection.Send()

	conf_id_list[p] = connection.WaitConfCreated(confName,num_retries)
    
    	#connect parties
    	num_of_parties = 3    
    	print "Start connecting Parties..."
	ConnectVideoParty(connection, conf_id_list[p], num_of_parties, num_retries)


    confName = "VSW_min_parties_56"
    connection.CreateConfFromProfile(confName, prof_id_list[1])
    conf_id  = connection.WaitConfCreated(confName,num_retries)
    ConnectVideoParty(connection, conf_id, 1, num_retries)


#     connection.TestResourcesReportPortsType("audio", 80, "TOTAL")
    connection.TestResourcesReportPortsType("video", (num_of_profiles*num_of_parties+1), "OCCUPIED")

    sleep(10)

    connection.DeleteConf(conf_id)

    #remove the profile
    for p in range(num_of_profiles):
	connection.DeleteConf(conf_id_list[p])
    	connection.DelProfile(prof_id_list[p])



## ---------------------- Test --------------------------	

c = ResourceUtilities()
c.Connect()

TestNxM(c,20)# retries

c.Disconnect()

#------------------------------------------------------------------------------  
    
