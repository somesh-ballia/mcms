#!/mcms/python/bin/python

# #############################################################################
# Creating CP conference and VSW conference with participants with different H264 SampleAspect Ratio values
#
# Date: 30/4/07
# By  : Keren 

#
############################################################################


from McmsConnection import *
from HDFunctions import *

#---------------------------------------------------------------------------------------------------------
def CreateSimCapsWithDifferentH264(connetion, newCapSetName,callRate, h264Mode,aspectRatioValue):
    print
    print "Start TestCreateSimCapsWithDifferentH264 test"
    print

    connetion.LoadXmlFile('Scripts/SimAddCapSet.xml')
    connetion.ModifyXml("ADD_CAP_SET","NAME",newCapSetName, )
    connetion.ModifyXml("ADD_CAP_SET","CALL_RATE",callRate)
    connetion.ModifyXml("VIDEO_H264_DETAILS","VIDEO_MODE_H264", h264Mode)
    connetion.ModifyXml("VIDEO_H264_DETAILS","H264_ASPECT_RATIO",aspectRatioValue)
    connetion.Send()
#---------------------------------------------------------------------------------------------------------
def TestConfrencesVideoModes(connection, confname, confFile, dialIn1CapSet, dialIn2CapSet, dialIn3CapSet, profileId = "false"):
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
    print "Connecting Dial out Party..."
    num_of_parties = 5   
    #------Dial out H323 party    
    partyname = confname+"_DialOut1"
    partyip =  "1.2.3.1" 
    connection.AddVideoParty(confid, partyname, partyip)
    print "Connecting H323 Dial out Party" 
    connection.WaitAllOngoingConnected(confid)
    connection.WaitAllOngoingNotInIVR(confid)
    #------Dial out SIP party   
    partyname = confname+"_DialOut2"
    partyip =  "1.2.3.2" 
    connection.AddVideoParty(confid, partyname, partyip, "true")
    print "Connecting WaitAllOngoingNotInIVRSIP Dial out Party" 
    connection.WaitAllOngoingConnected(confid)
    connection.WaitAllOngoingNotInIVR(confid)

    #-------Dial in 1 H323 party
    partyname = confname+"_DialIn1"
    connection.SimulationAddH323Party(partyname, confname,dialIn1CapSet)
    connection.SimulationConnectH323Party(partyname)
    print "Connecting H323 Dial in Party -" + dialIn1CapSet
    connection.WaitAllOngoingConnected(confid)
    connection.WaitAllOngoingNotInIVR(confid)

     #-------Dial in 2 H323 party
    partyname = confname+"_DialIn2"
    connection.SimulationAddH323Party(partyname, confname,dialIn2CapSet)
    print "Connecting H323 Dial in Party -" + dialIn2CapSet
    connection.SimulationConnectH323Party(partyname)
    print "Wait Party Connected: " + dialIn2CapSet
    connection.WaitAllOngoingConnected(confid)
    connection.WaitAllOngoingNotInIVR(confid)
    
   #-------Dial in 3 H323 party
    partyname = confname+"_DialIn3"
    connection.SimulationAddH323Party(partyname, confname,dialIn3CapSet)
    connection.SimulationConnectH323Party(partyname)
    print "Connecting H323 Dial in Party -" + dialIn3CapSet
    connection.WaitAllOngoingConnected(confid)
    connection.WaitAllOngoingNotInIVR(confid)
    
    
    sleep(1)
    print "Start disconnecting Parties..."
    listPartyIDs = connection.GetPartyIDs(confid)
    print listPartyIDs
    
    i = 1
    for x in listPartyIDs:
        connection.DeleteParty(confid, x)
        print "current number of connected parties = " + str(num_of_parties-i)
        i += 1
        sleep(1)
  

    print "Deleting Conf..." 
    connection.DeleteConf(confid)   
    connection.WaitAllConfEnd()
#---------------------------------------------------------------------------------------------------------


c = McmsConnection()
c.Connect()
print "---------------------------------------"
print "Test1: CP Conf"
print "---------------------------secondPartyCapSetName-----------"
confName = "Conf512Sharpness"
confFile = 'Scripts/SD/AddVideoCpConf512kSharpness.xml'

firstPartyCapSetName = "1_SD30_SAR3"
firstPartyH264Mode = "cif"
firstPartySAR = 3
CreateSimCapsWithDifferentH264(c,firstPartyCapSetName,"384",firstPartyH264Mode,firstPartySAR)

secondPartyCapSetName = "1_SD30_SAR255"
secondPartyH264Mode = "sd30"
secondPartySAR = 255
CreateSimCapsWithDifferentH264(c,secondPartyCapSetName,"512",secondPartyH264Mode,secondPartySAR)

thirdPartyCapSetName = "1_HD_SAR13"
thirdPartyH264Mode = "hd720"
thirdPartySAR = 13
CreateSimCapsWithDifferentH264(c,thirdPartyCapSetName,"512",thirdPartyH264Mode,thirdPartySAR)

TestConfrencesVideoModes(c, confName, confFile, firstPartyCapSetName, secondPartyCapSetName, thirdPartyCapSetName)
   
sleep(1)
print "---------------------------------------"
print "Test2: HDCP Conf"
print "--------------------------------------"

command_line = "Bin/McuCmd set ConfParty MAX_CP_RESOLUTION HD" + '\n'
os.system(command_line)

confName = "CHDCPMotion1920"
confFile = 'Scripts/HDCP/AddVideoCpConf1920kMotion.xml'

firstPartyCapSetName = "2_HD_SAR5"
firstPartyH264Mode = "hd720"
firstPartySAR = 5
CreateSimCapsWithDifferentH264(c,firstPartyCapSetName,"1920",firstPartyH264Mode,firstPartySAR)

secondPartyCapSetName = "2_HD_SAR255"
secondPartyH264Mode = "hd720"
secondPartySAR = 255
CreateSimCapsWithDifferentH264(c,secondPartyCapSetName,"1920",secondPartyH264Mode,secondPartySAR)

thirdPartyCapSetName = "2_HD_SAR13"
thirdPartyH264Mode = "hd720"
thirdPartySAR = 13
CreateSimCapsWithDifferentH264(c,thirdPartyCapSetName,"1920",thirdPartyH264Mode,thirdPartySAR)

TestConfrencesVideoModes(c, confName, confFile, firstPartyCapSetName, secondPartyCapSetName, thirdPartyCapSetName)
   

print "---------------------------------------"
print "Test3: VSW Conf"
print "--------------------------------------"

confName = "ConfVSWHD"
profId = AddHdProfile(c,"HD_profile_1920","1920", "Scripts/HD/XML/AddHdProfile.xml")
    

confFile = ''

firstPartyCapSetName = "3_HD_SAR13"
firstPartyH264Mode = "hd720"
firstPartySAR = 13
CreateSimCapsWithDifferentH264(c,firstPartyCapSetName,"1920",firstPartyH264Mode,firstPartySAR)

secondPartyCapSetName = "3_HD_SAR255"
secondPartyH264Mode = "hd720"
secondPartySAR = 255
CreateSimCapsWithDifferentH264(c,secondPartyCapSetName,"1920",secondPartyH264Mode,secondPartySAR)

thirdPartyCapSetName = "3_HD_SAR7"
thirdPartyH264Mode = "hd720"
thirdPartySAR = 7
CreateSimCapsWithDifferentH264(c,thirdPartyCapSetName,"1920",thirdPartyH264Mode,thirdPartySAR)

TestConfrencesVideoModes(c, confName, confFile, firstPartyCapSetName, secondPartyCapSetName, thirdPartyCapSetName,profId)
   
sleep(1)
print "--------------------------------------"




sleep(1)
c.Disconnect()


