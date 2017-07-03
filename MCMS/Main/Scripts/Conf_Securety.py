#!/mcms/python/bin/python

# #############################################################################
# Creating CP conference and check secure mode
#
# Date: 23/01/08
# By  : Natalya D

#
############################################################################

from McmsConnection import *
from datetime import *
from ConfPkgUtil import *



##------------------------------------------------------------------------
def IsPartyDisconnected(c,confid,party_number, num_retries=30):
    c.LoadXmlFile('Scripts/TransConf2.xml')
    c.ModifyXml("GET","ID",confid)
    c.Send()
    for retry in range(num_retries+1):
        ongoing_parties = c.xmlResponse.getElementsByTagName("ONGOING_PARTY_STATUS")
        ongoing_party_list = c.xmlResponse.getElementsByTagName("ONGOING_PARTY")
	num_of_parties = len(ongoing_party_list)
        if(party_number == num_of_parties):
 	   print num_of_parties
           return 0
           break
        sys.stdout.write(".")
        sys.stdout.flush()
        c.Send()
        sleep(1)
    print num_of_parties
    return 1

##------------------------------------------------------------------------

def FormatDate(stringDate): 
    # stringDate is in the format of 2006-01-23T17:15:05 and split it to year, month, day, hour, min, second
      ss = stringDate.split('-')
      Year = ss[0]
      Month = ss[1]
      
      ss2 = ss[2].split('T')
      Day = ss2[0]
      
      ss3 = ss2[1].split(':')
      Hour =ss3[0] 
      Minute = ss3[1]
      Sec = ss3[2]
      
      return Year,  Month,  Day, Hour, Minute,  Sec
#------------------------------------------------------------------------------
def CreateSecureConf(confName,num_of_parties,num_retries):
	c.SendXmlFile('Scripts/IvrChangeToChairAndEnableSecure/UpdateIvrServiceWithSecureConf.xml' )
	c.CreateConf(confName, 'Scripts/ContentPresentation/AddCpConf.xml')
	confid = c.WaitConfCreated(confName,num_retries)
	return confid
 
#------------------------------------------------------------------------------
def AddPartiesDialIn(confid, confName,num_of_parties,num_retries,expected_status="Status OK"):
	 for x in range(num_of_parties):
	        partyname = confName+"Party"+str(x+1)
	        c.SimulationAddH323Party(partyname, confName)
	 for y in range(num_of_parties):
	        partyname = confName+"Party"+str(y+1) 
                c.SimulationConnectH323PartyExpStatus(partyname,expected_status)
          	sleep(2)        
 

#------------------------------------------------------------------------------
def AddPartyDialIn(confid, confName,partyname):
         c.SimulationAddH323Party(partyname, confName)
         c.SimulationConnectH323Party(partyname)
       	 sleep(2)        
    
#------------------------------------------------------------------------------
def AddPartiesDialOut(confid, confName,num_of_parties,num_retries,expected_status="Status OK"):
    	 for x in range(num_of_parties):
            partyname = "Party" + str(x+1)
            partyip =  "1.2.3." + str(x+1)
            print "Adding Party " + partyname + ", with ip= " + partyip
            c.AddParty(confid, partyname, partyip, 'Scripts/AddVideoParty1.xml', expected_status)
       	    sleep(2)        


#------------------------------------------------------------------------------
def AddPartyDialOut(confid, confName,partyname,expected_status="Status OK"):
            partyip =  "1.2.3.6" 
            print "Adding Party " + partyname + ", with ip= " + partyip
            c.AddParty(confid, partyname, partyip, 'Scripts/AddVideoParty1.xml', expected_status)
       	    sleep(2)        

#------------------------------------------------------------------------------
def SetCherPersonAndSecure(confid, partyId,PartyDbgName):
    
   	c.LoadXmlFile('Scripts/IvrChangeToChairAndChangePWs/SetAsChair.xml' )
	c.ModifyXml("SET_LEADER","ID",confid)
	c.ModifyXml("SET_LEADER","PARTY_ID",partyId)
	c.Send()
	print PartyDbgName + "became chair person"
	

#------------------------------------------------------------------------------
def SendDtmf(dtmf ,PartyDbgName): 
	print "Send Dtmf : " + dtmf+ "to " + PartyDbgName
	c.SimulationH323PartyDTMFWithoutDelimiter(PartyDbgName, dtmf)

#------------------------------------------------------------------------------
def ReconnectParty(confid,partyid,expected_status="Status OK"):
	print "Reconnect party ... "
        c.LoadXmlFile('Scripts/ReconnectParty/TransSetConnect.xml')
        c.ModifyXml("SET_CONNECT","ID",confid)
        c.ModifyXml("SET_CONNECT","CONNECT","true")
        c.ModifyXml("SET_CONNECT","PARTY_ID",partyid)
        c.Send(expected_status)

#------------------------------------------------------------------------------
def UpdateParty(confid, partyid,expected_status="Status OK"):
        print "Updating party ... "
	partyip =  "5.6.7.8"
	partyname = "Party11" 
        c.LoadXmlFile('Scripts/UpdateParty/UpdateParty.xml')
        c.ModifyXml("UPDATE_PARTY", "ID", confid)
        c.ModifyXml("PARTY","NAME",partyname)
        c.ModifyXml("PARTY","ID",partyid)
        c.ModifyXml("PARTY","IP",partyip)
        c.Send(expected_status)
        sleep(2)

#------------------------------------------------------------------------------
def MutePartyVideoAudioTest(confid, partyid,Mute_mode,expected_status="Status OK"):
    print "Muting " + Mute_mode +" ... " 
    c.LoadXmlFile('Scripts/MuteVideo/MutePartyVideo.xml')
    c.ModifyXml("SET_AUDIO_VIDEO_MUTE","ID",confid)
    c.ModifyXml("SET_AUDIO_VIDEO_MUTE","PARTY_ID",partyid) 
    c.ModifyXml("SET_AUDIO_VIDEO_MUTE",Mute_mode,"true")
    c.Send(expected_status)
    return


#------------------------------------------------------------------------------
def SeAudioBlock(confid, partyid,expected_status="Status OK"):
    print "Setting audio block ... "
    c.LoadXmlFile("Scripts/TestConfCorruptedParams/SetAudioBlock.xml")
    c.ModifyXml("SET_AUDIO_BLOCK","ID",confid)
    c.ModifyXml("SET_AUDIO_BLOCK","PARTY_ID",partyid)
    c.ModifyXml("SET_AUDIO_BLOCK","AUDIO_BLOCK","true")
    c.Send(expected_status)
    sleep(2)
#------------------------------------------------------------------------------
def UnMutePartyVideoAudioTest(confid, partyid,Mute_mode,expected_status="Status OK"):
    print "Unmuting " + Mute_mode +" ... " 
    c.LoadXmlFile('Scripts/MuteVideo/MutePartyVideo.xml')
    c.ModifyXml("SET_AUDIO_VIDEO_MUTE","ID",confid)
    c.ModifyXml("SET_AUDIO_VIDEO_MUTE","PARTY_ID",partyid) 
    c.ModifyXml("SET_AUDIO_VIDEO_MUTE",Mute_mode,"false")
    c.Send(expected_status)
    return

#------------------------------------------------------------------------------    
def ChenageVideoLayout(confid,expected_status="Status OK"):
        print "Change Layout ... "
	confLayoutType = "2x2"
	c.ChangeConfLayoutTypeExpStatus(confid, confLayoutType,expected_status)
	
#------------------------------------------------------------------------------    
def SetEndTim(confid,startTime,expected_status="Status OK"):
         print "Setting end time ... "
	 Hours = 1        # 0 - 99
	 Minutes = 23    # 0 - 60
	 Seconds = 0    # 0 - 60
	 startYear,  startMonth,  startDay, startHour, startMinute,  startSec = FormatDate(startTime) 
	 endHour = (int(startHour) + Hours)% 24
         endDay = int(startDay) + 0
         endTime = startYear + "-" + startMonth + "-"+ str(endDay) + "T" + str(endHour) + ":" + str(Minutes) + ":" + startSec
         print "the start time is: " + startTime
         print "the end time is: " + endTime
         c.LoadXmlFile("Scripts/TestConfCorruptedParams/SetEndTime.xml")
         c.ModifyXml("SET_END_TIME","ID",confid)
         c.ModifyXml("SET_END_TIME","END_TIME",endTime)
         c.Send(expected_status)

#------------------------------------------------------------------------------    
def SetAGC(confid,partyid,expected_status="Status OK"):
        print "Setting AGC ... "
        c.LoadXmlFile("Scripts/TestConfCorruptedParams/SetAGC.xml")
        c.ModifyXml("SET_AGC","ID",confid)
        c.ModifyXml("SET_AGC","PARTY_ID",partyid)    
        c.ModifyXml("SET_AGC","AGC","true")
        c.Send(expected_status)


#------------------------------------------------------------------------------    
def TestPartyManipilationInSecureMode(confid,confName,expected_status="Status OK"):
        print "Add dialOut Function"
        AddPartyDialOut(confid,confName,"TestDialOut",expected_status)
        print "Add dialIn Function"
        AddPartyDialIn(confid,confName, "TempTestDialIn") 
        c.DisconnectPartyExpStatus(confid,2,expected_status)
        c.DeleteParty(confid,2,expected_status)
        ReconnectParty(confid,1,expected_status)
        UpdateParty(confid, 3,expected_status)
        MutePartyVideoAudioTest(confid, 0,"AUDIO_MUTE",expected_status)
        MutePartyVideoAudioTest(confid, 0,"VIDEO_MUTE",expected_status)
        UnMutePartyVideoAudioTest(confid, 0,"AUDIO_MUTE",expected_status)
        UnMutePartyVideoAudioTest(confid, 0,"VIDEO_MUTE",expected_status)
        SeAudioBlock(confid, 3,expected_status)
  
   
#------------------------------------------------------------------------------    
def TestConfManipilationInSecureMode(confid,expected_status="Status OK"):
	c.SendXmlFile('Scripts/TransConfList.xml',"Status OK")
	confid = c.GetTextUnder("CONF_SUMMARY","ID")
	startTime = c.GetTextUnder("CONF_SUMMARY","START_TIME")         
   	ChenageVideoLayout(confid,expected_status)
	SetAGC(confid, 3,expected_status)
	SetEndTim(confid,startTime, expected_status)

#------------------------------------------------------------------------------    
def PrepareEnvForTest(confName,num_of_parties,num_retries):
	confid = CreateSecureConf(confName, 1,5)
	AddPartiesDialOut(confid,confName, 2,5 )
	AddPartiesDialIn(confid,confName, 2, 5)
	c.WaitAllOngoingConnected(confid,num_retries*num_of_parties,3)
	c.WaitAllPartiesWereAdded(confid,num_of_parties,num_retries*num_of_parties)
	print "Sleep 10"
	sleep(10)
	c.DisconnectParty(confid,1)
	SetCherPersonAndSecure(confid, 0,"DIAL_OUT#10001")
	return confid

#------------------------------------------------------------------------------    
def TestConfAndPartyInUnsecuremode(confid,confName,num_of_parties,num_retries):
	if(IsPartyDisconnected(c, confid,num_of_parties +1,10)!= 1):
           print "Error: DialInParty is not in the correct state: "  
           c.ScriptAbort("Failed delete conference!!!")  

        TestPartyManipilationInSecureMode(confid,confName,"Status OK")
        sleep(3)

        if(IsPartyDisconnected(c, confid,num_of_parties+1)!= 0):
          print "Error: DialInParty is not in the correct state: "
          c.ScriptAbort("Failed delete conference!!!")
  
        TestConfManipilationInSecureMode(confid)


#---------------------------------------------------------------------------------------------------------
c = McmsConnection()
c.Connect()       
confName = "ConfTest1"
num_of_parties = 4
num_retries = 5
confid = PrepareEnvForTest(confName,num_of_parties,num_retries)
SendDtmf("*71","DIAL_OUT#10001")
print "-----The Conference in Secure mode----"
TestPartyManipilationInSecureMode(confid,confName,"This action is forbidden in secured conference")
TestConfManipilationInSecureMode(confid,"This action is forbidden in secured conference")
SendDtmf("#71","DIAL_OUT#10001")
print "-----The Conference in Unsecure mode----"
TestConfAndPartyInUnsecuremode(confid,confName,num_of_parties,num_retries)

print "Deleting Conf..." 
c.DeleteConf(confid)   
c.WaitAllConfEnd() 

print "-----Delete Secured conference----"
confName = "ConfTest2"
num_of_parties = 2
num_retries = 5
confid = CreateSecureConf(confName, 1,5)
AddPartiesDialOut(confid,confName, 2,5 )
c.WaitAllOngoingConnected(confid,num_retries*num_of_parties,3)
c.WaitAllPartiesWereAdded(confid,num_of_parties,num_retries*num_of_parties)
print "Sleep 6"
sleep(6)
SetCherPersonAndSecure(confid, 0,"DIAL_OUT#10006")
SendDtmf("*71","DIAL_OUT#10006")
c.DeleteConf(confid)  
sleep(4) 
c.DeleteConf(confid, "Conference name or ID does not exist")   
c.Disconnect()
   
