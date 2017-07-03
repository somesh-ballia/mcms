#!/mcms/python/bin/python

# #############################################################################
# Creating CP conference and VSW conference with participants with different H264 SampleAspect Ratio values
#
# Date: 13/8/09
# By  : Uri 

#
############################################################################


from McmsConnection import *
from HDFunctions import *

#---------------------------------------------------------------------------------------------------------
def CreateSimCapsWithAudioCodecs(connetion, newCapSetName,callRate):
    print
    print "Start CreateSimCapsWithAudioCodecs test"
    print

    connetion.LoadXmlFile('Scripts/SimAddCapSet.xml')
    connetion.ModifyXml("ADD_CAP_SET","NAME",newCapSetName, )
    connetion.ModifyXml("ADD_CAP_SET","CALL_RATE",callRate)
    connetion.Send()


#-------------------------Dial in X H323 party-------------------------------------------------------------------
def CheckPartyMonitoring(connetion, confid, partyNumber, dialInCapSet):
    partyname = confname+"_DialIn" + partyNumber
    party_id = connection.GetPartyId(confid, partyname)
    connection.LoadXmlFile('Scripts/TransParty.xml')
    connection.ModifyXml("GET","CONF_ID",confid)
    connection.ModifyXml("GET","PARTY_ID",party_id)
    print "Ask party monitoring from " + partyname + "  ..."
    connection.Send()

    num_retries = 30
    for retry in range(num_retries+1):
        status = connection.SendXmlFile('Scripts/TransParty.xml',"Status OK")
        comm_mode = connection.GetTextUnder("ONGOING_PARTY","H323_LOCAL_COMM_MODE")
	if comm_mode != "" :
        	if (dialInCapSet == "FULL_With_Siren22Stereo128K"):
			if (comm_mode.find("Siren22Stereo_128k") != -1):
		            	print "Communication Mode is OK"
	            	break
        	if (dialInCapSet == "FULL_With_Siren22Stereo96K"):
			if comm_mode.find("Siren22Stereo_96k") != -1 :
		            	print "Communication Mode is OK"
	            	break
        	if (dialInCapSet == "FULL_With_Siren22Mono"):
			if comm_mode.find("Siren22_64k") != -1 :
		            	print "Communication Mode is OK"
	            	break
        	if (dialInCapSet == "FULL_With_Siren14Stereo"):
			if comm_mode.find("Siren14Stereo_") != -1 :
		            	print "Communication Mode is OK"
	            	break
        if (retry == num_retries):
            print connection.xmlResponse.toprettyxml()
            connection.Disconnect()                
            connection.exit("Can not monitor conf:" + status)
        sys.stdout.write(".")
        sys.stdout.flush()


#-------------------------Dial in X H323 party-------------------------------------------------------------------
def ConnectDialInParty(connetion, confname, partyNumber, dialInCapSet, confid):
    partyname = confname+"_DialIn" + partyNumber
    connection.SimulationAddH323Party(partyname, confname,dialInCapSet)
    connection.SimulationConnectH323Party(partyname)
    print "Connecting H323 Dial in Party -" + dialInCapSet
    connection.WaitAllOngoingConnected(confid)
    connection.WaitAllOngoingNotInIVR(confid)

#---------------------------------------------------------------------------------------------------------
def TestConfrencesAudioCodec(connection, confname, confFile, dialIn1CapSet, dialIn2CapSet, dialIn3CapSet, dialIn4CapSet, profileId = "false"):
    if(profileId=="false"):
        connection.LoadXmlFile(confFile)
        connection.ModifyXml("RESERVATION","NAME",confname)
        print "Adding Conf " + confname + "  ..."
        connection.Send()
    else:
        connection.CreateConfFromProfile(confname, profileId)
        
    print "Wait untill Conf create...",
    num_retries = 30
    for retry in range(num_retries+1):
        status = connection.SendXmlFile('Scripts/TransConfList.xml',"Status OK")
        confid = connection.GetTextUnder("CONF_SUMMARY","ID")
        if confid != "":
            print
            break
        if (retry == num_retries):
            print connection.xmlResponse.toprettyxml()
            connection.Disconnect()                
            connection.exit("Can not monitor conf:" + status)
        sys.stdout.write(".")
        sys.stdout.flush()

    #connect parties
    ConnectDialInParty(confname, 1, dialIn1CapSet, confid)
    ConnectDialInParty(confname, 2, dialIn2CapSet, confid)
    ConnectDialInParty(confname, 3, dialIn3CapSet, confid)
    ConnectDialInParty(confname, 4, dialIn4CapSet, confid)
 
    print "Wait untill all parties are connected...",
    sleep(20)	

   #ask for party monitoring 
    CheckPartyMonitoring(confid, 1, dialIn1CapSet)
    CheckPartyMonitoring(confid, 2, dialIn2CapSet)
    CheckPartyMonitoring(confid, 3, dialIn3CapSet)
    CheckPartyMonitoring(confid, 4, dialIn4CapSet)

    print "Deleting Conf..." 
    connection.DeleteConf(confid)   
    connection.WaitAllConfEnd()

#---------------------------------------------------------------------------------------------------------


c = McmsConnection()
c.Connect()


print "---------------------------------------"
print "Test1: HDCP Conf"
print "--------------------------------------"

command_line = "Bin/McuCmd set ConfParty MAX_CP_RESOLUTION HD" + '\n'
os.system(command_line)

confName = "CHDCPMotion1920"
confFile = 'Scripts/HDCP/AddVideoCpConf1920kMotion.xml'

firstPartyCapSetName = "FULL_With_Siren22Stereo128K"
CreateSimCapsWithAudioCodecs(c,firstPartyCapSetName,"1920")

secondPartyCapSetName = "FULL_With_Siren22Stereo96K"
CreateSimCapsWithAudioCodecs(c,firstPartyCapSetName,"1920")

thirdPartyCapSetName = "FULL_With_Siren22Mono"
CreateSimCapsWithAudioCodecs(c,firstPartyCapSetName,"1920")

fourthPartyCapSetName = "FULL_With_Siren14Stereo"
CreateSimCapsWithAudioCodecs(c,firstPartyCapSetName,"1920")

TestConfrencesAudioCodec(c, confName, confFile, firstPartyCapSetName, secondPartyCapSetName, thirdPartyCapSetName, fourthPartyCapSetName, profId)


sleep(1)
c.Disconnect()
