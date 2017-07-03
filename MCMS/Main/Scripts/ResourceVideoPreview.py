#!/mcms/python/bin/python

# For list of profiles look at RunTest.sh
#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_1


#############################################################################
# Test Script for a checking the Video Preview feature in RMX2000C
# Date: 27/12/09
# By  : Olga
#############################################################################

# 1. Create a conference and connect 5 video parties
# 2. Try to start a video preview for each participant: 4 should be started successful and one to be failed
# 3. Stop video preview of all participants
# 4. Start video preview for first two participants, and open for each of them IN annd OUT direction - SUCCESS
# 5. Start video preview for a third participant - FAILED
# 6. Stop the "out" direction for a first participants video preview
# 7. Start "in" video preview for a first participant - FAILED
# 8. Start video preview for a third participant - SUCCESS
# 9. Stop video preview of all participants
# 10.Remove the conference


from ResourceUtilities import *
from HDFunctions import *
import string 

#------Dial out H323 party    
def ConnectVideoParty(connection, conf_id, num_of_parties, party_id_list, num_retries):
    for x in range(num_of_parties):
	partyname = "Party_"+str(x+1)
        partyip = "1.2.3." + str(x+1) 
	connection.AddVideoParty(conf_id, partyname, partyip)
        print "Connecting H323 Dial out Party " + partyname
	sleep(3)
        party_id_list[x] = connection.GetPartyId(conf_id, partyname)

    connection.WaitAllPartiesWereAdded(conf_id, num_of_parties, num_retries*2)

    connection.WaitAllOngoingConnected(conf_id)
    connection.WaitAllOngoingNotInIVR(conf_id)

#------------------------------------------------------------------------------

def TestVideoPreview(connection,num_retries):
    
    profId = connection.AddProfile("profile")

# 1. Create a conference and connect 5 video parties

    confName = "Conf_"+str(profId)
    print "Adding Conf " + confName + " ..."
    connection.LoadXmlFile('Scripts/AddCpConf.xml')
    connection.ModifyXml("RESERVATION","NAME",confName)
    connection.ModifyXml("RESERVATION","AD_HOC_PROFILE_ID",profId) 
    connection.ModifyXml("RESERVATION","MAX_PARTIES",str(28))
    connection.Send()
    conf_id = connection.WaitConfCreated(confName,num_retries)
    
    #connect parties
    num_of_parties = 5    
    party_id_list=[1,2,3,4,5]
    print "Start connecting Parties..."
    ConnectVideoParty(connection, conf_id, num_of_parties, party_id_list, num_retries)
    sleep(5)

# 2. Try to start a video preview for each participant: 4 should be started successful and one to be failed

    print "Start a video preview..."
    connection.LoadXmlFile('Scripts/VideoPreview/StartVideoPreview.xml')
    connection.ModifyXml("START_PREVIEW","ID",conf_id)
    expected_status = "Status OK"
    for p in range(num_of_parties):
	ip_adr = "172.22.169."+str(p+8)
	connection.ModifyXml("START_PREVIEW","PARTY_ID",party_id_list[p]) 
#    	connection.ModifyXml("START_PREVIEW","VIDEO_DIRECTION","in")
    	connection.ModifyXml("START_PREVIEW","CLIENT_IP",ip_adr)
	if(p > 3):
	    expected_status = "Failure Status"
    	connection.Send(expected_status)

    sleep(3)

# 3. Stop video preview of all participants

    print "Stop all video preview..."
    connection.LoadXmlFile('Scripts/VideoPreview/StopVideoPreview.xml')
    connection.ModifyXml("STOP_PREVIEW","ID",conf_id)
    expected_status = "Status OK"

    for p in range(num_of_parties):
	ip_adr = "172.22.169."+str(p+8)
	connection.ModifyXml("STOP_PREVIEW","PARTY_ID",party_id_list[p]) 
    	connection.Send(expected_status)

    sleep(3)

# 4. Start video preview for first two participants, and open for each of them  IN annd OUT direction - SUCCESS

    print "Start a video preview for 2 parties when each of them has in & out direction"
    connection.LoadXmlFile('Scripts/VideoPreview/StartVideoPreview.xml')
    connection.ModifyXml("START_PREVIEW","ID",conf_id)
    expected_status = "Status OK"

    for p in range(num_of_parties-1):
	ip_adr = "172.22.169."+str(p+8)
    	connection.ModifyXml("START_PREVIEW","CLIENT_IP",ip_adr)
	if(p > 1):
	    connection.ModifyXml("START_PREVIEW","VIDEO_DIRECTION","in")
	    connection.ModifyXml("START_PREVIEW","PARTY_ID",party_id_list[p-2])
	else:
	    connection.ModifyXml("START_PREVIEW","PARTY_ID",party_id_list[p])
    	connection.Send(expected_status)

# 5. Start video preview for a third participant - FAILED

    ip_adr = "172.22.169."+str(13)
    connection.ModifyXml("START_PREVIEW","CLIENT_IP",ip_adr)
    connection.ModifyXml("START_PREVIEW","PARTY_ID",party_id_list[2])
    connection.Send("Failure Status")


# 6. Stop the "out" direction for a first participants video preview

    ip_adr = "172.22.169."+str(8)
    print "Stop the OUT direction for a first participants video preview"
    connection.LoadXmlFile('Scripts/VideoPreview/StopVideoPreview.xml')
    connection.ModifyXml("STOP_PREVIEW","ID",conf_id)
    connection.ModifyXml("STOP_PREVIEW","PARTY_ID",party_id_list[0]) 
    connection.Send("Status OK")

# 7. Start "in" video preview for a first participant - FAILED

    print "Start IN video preview for a first participant "
    ip_adr = "172.22.169."+str(13)
    connection.LoadXmlFile('Scripts/VideoPreview/StartVideoPreview.xml')
    connection.ModifyXml("START_PREVIEW","ID",conf_id)
    connection.ModifyXml("START_PREVIEW","CLIENT_IP",ip_adr)
    connection.ModifyXml("START_PREVIEW","VIDEO_DIRECTION","in")
    connection.ModifyXml("START_PREVIEW","PARTY_ID",party_id_list[0])
    connection.Send("Failure Status")

# 8. Start video preview for a third participant - SUCCESS

    print "Start video preview of third participants "
    ip_adr = "172.22.169."+str(13)
    connection.LoadXmlFile('Scripts/VideoPreview/StartVideoPreview.xml')
    connection.ModifyXml("START_PREVIEW","ID",conf_id)
    connection.ModifyXml("START_PREVIEW","CLIENT_IP",ip_adr)
    connection.ModifyXml("START_PREVIEW","VIDEO_DIRECTION","in")
    connection.ModifyXml("START_PREVIEW","PARTY_ID",party_id_list[2])
    connection.Send("Status OK")

    sleep(10)

# 9. Stop video preview of all participants

    print "Stop all video preview..."
    connection.LoadXmlFile('Scripts/VideoPreview/StopVideoPreview.xml')
    connection.ModifyXml("STOP_PREVIEW","ID",conf_id)
    for p in range(num_of_parties):
	ip_adr = "172.22.169."+str(p+8)
	connection.ModifyXml("STOP_PREVIEW","PARTY_ID",party_id_list[p]) 
    	connection.Send("Status OK")


# 10.Remove the conference
    connection.DeleteConf(conf_id)
    connection.DelProfile(profId)



## ---------------------- Test --------------------------
c = ResourceUtilities()
c.Connect()

TestVideoPreview(c,20)# retries

c.Disconnect()

#------------------------------------------------------------------------------  
    
