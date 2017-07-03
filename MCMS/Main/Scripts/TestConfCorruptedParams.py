#!/mcms/python/bin/python

# For list of profiles look at RunTest.sh
#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_8

#############################################################################
# Test Script which Create  Conf with 1  participant and 
#           Check the params of all the SET transactions.
# 
# Date: 30/01/06
# By  : Ron S.

#############################################################################

from McmsConnection import *
from datetime import *

###------------------------------------------------------------------------------
def AddConfAndParty(connection,confFile,partyFile,numRetries):
    
    long_char = "sababasababasababasababasababasababasababasababasababasababasababasababasababasababasababasababasababasababasababasababasababasababasababasababasababasababasababasababasababasababasababasababasababasababasababasababasababasababasababasababasaba"
    semi_long_char = "sababasababasababasababasababasababasababasababasababasababasababasababasababasababasababasababasabasababasababasababasababasababasababa"
    short_char = "sababa"
    
    confname = "Conf"
    connection.CreateConf(confname, confFile)
    
    print "Wait untill Conf:" + confname + " will be createtd...",
    for retry in range(numRetries+1):
        connection.SendXmlFile('Scripts/TransConfList.xml',"Status OK")
        confid = connection.GetTextUnder("CONF_SUMMARY","ID")
        startTime = connection.GetTextUnder("CONF_SUMMARY","START_TIME")         
        
        if confid != "":
            print
            break

        if (retry == numRetries):
            print connection.xmlResponse.toprettyxml()
            connection.Disconnect()                
            sys.exit("Can not monitor conf:" + status)
        sys.stdout.write(".")
        sys.stdout.flush()
        
    print "Create conf with id " + str(confid) 
     
# adding the first participant 
    connection.LoadXmlFile(partyFile)
    partyname = "Party1" 
    partyip =  "1.2.3.1"
    print "Adding Party:" + partyname + ", with ip= " + partyip
    connection.AddParty(confid, partyname, partyip, partyFile)
    connection.WaitAllPartiesWereAdded(confid,1,numRetries)
    
    partyid = connection.GetPartyId(confid, partyname)
    connection.WaitAllOngoingConnected(confid,numRetries)  
    
    sleep(10)
    
# set end time   

    Hours = 220        # 0 - 99
    Minutes = 23    # 0 - 60
    Seconds = 0    # 0 - 60
    
#    time format : 2006-02-06T17:15:05
    startYear,  startMonth,  startDay, startHour, startMinute,  startSec = FormatDate(startTime) 
    endHour = int(startHour) + Hours
    endMinute = int(startMinute) + Minutes
    endDay = int(startDay) + 0
    endTime = startYear + "-" + startMonth + "-" + str(endDay) + "T" + str(endHour) + ":" + str(endMinute) + ":" + startSec
    print "the start time is: " + startTime
    print "the end time is: " + endTime
       
    print "Setting end time ... "
    connection.LoadXmlFile("Scripts/TestConfCorruptedParams/SetEndTime.xml")
    connection.ModifyXml("SET_END_TIME","ID",confid)
    connection.ModifyXml("SET_END_TIME","END_TIME",endTime)
    connection.Send()
    sleep(50)
    
      
         
# set lecture mode        
    print "Setting lecture mode ... "
    connection.LoadXmlFile("Scripts/TestConfCorruptedParams/SetLectureMode.xml")
    connection.ModifyXml("SET_LECTURE_MODE","ID",confid)
    connection.ModifyXml("LECTURE_MODE","ON","true")    
    connection.ModifyXml("LECTURE_MODE","TIMER","true")
    connection.ModifyXml("LECTURE_MODE","INTERVAL",str(15))        #15
    connection.ModifyXml("LECTURE_MODE","AUDIO_ACTIVATED","true")
#    connection.ModifyXml("LECTURE_MODE","LECTURE_NAME","dfgdfgdfgdfg")
    connection.ModifyXml("LECTURE_MODE","LECTURE_MODE_TYPE","lecture_presentation")        #lecture_presentation    
    connection.ModifyXml("LECTURE_MODE","LECTURE_ID",str(-1))     #-1

    connection.Send()
#    sleep(10)

# set billing data        
    print "Setting billing data ... "
    connection.LoadXmlFile("Scripts/TestConfCorruptedParams/SetBillingData.xml")
    connection.ModifyXml("SET_BILLING_DATA","ID",confid)
    connection.ModifyXml("SET_BILLING_DATA","BILLING_DATA",long_char)
    connection.Send()
#    sleep(10)

# set conf contact info       
    print "Setting conf contact info ... "
    connection.LoadXmlFile("Scripts/TestConfCorruptedParams/SetConfContactInfo.xml")
    connection.ModifyXml("SET_CONF_CONTACT_INFO","ID",confid)
    connection.ModifyXml("CONTACT_INFO_LIST","CONTACT_INFO",#"CONTACT_INFO"
                          long_char)
    connection.ModifyXml("CONTACT_INFO_LIST","CONTACT_INFO_2","CONTACT_INFO_2")
    connection.ModifyXml("CONTACT_INFO_LIST","CONTACT_INFO_3","CONTACT_INFO_3")
    connection.ModifyXml("CONTACT_INFO_LIST","CONTACT_INFO_4","CONTACT_INFO_4")
    connection.Send()
#    sleep(10)



# set audio layout mode        
    print "Setting audio layout mode ... "
    connection.LoadXmlFile("Scripts/TestConfCorruptedParams/SetAudioLayout.xml")
    connection.ModifyXml("SET_AUTO_LAYOUT","ID",confid)
    connection.ModifyXml("SET_AUTO_LAYOUT","AUTO_LAYOUT","true")
    connection.Send()
#    sleep(10)


# set video layout mode        
    print "Setting video layout mode ... "
    connection.LoadXmlFile("Scripts/TestConfCorruptedParams/SetVideoLayout.xml")
    connection.ModifyXml("SET_VIDEO_LAYOUT","ID",confid)
    connection.ModifyXml("FORCE","LAYOUT","1x1")        #1x1
    connection.ModifyXml("CELL","ID",str(1))
    connection.ModifyXml("CELL","FORCE_STATE","auto")
    connection.ModifyXml("CELL","FORCE_ID",str(-1))        #-1
#    connection.ModifyXml("CELL","FORCE_NAME",long_char)
    connection.ModifyXml("CELL","SOURCE_ID",str(-1))        #-1
    connection.Send()
#    sleep(10)

# set visual effect mode        
    print "Setting visual effect ... "
    connection.LoadXmlFile("Scripts/TestConfCorruptedParams/SetVisualEffect.xml")
    connection.ModifyXml("SET_VISUAL_EFFECT","ID",confid)
    connection.ModifyXml("BACKGROUND_COLOR","RED",str(55))
    connection.ModifyXml("BACKGROUND_COLOR","GREEN",str(55))
    connection.ModifyXml("BACKGROUND_COLOR","BLUE",str(155))
    connection.ModifyXml("VISUAL_EFFECTS","LAYOUT_BORDER","true")
    connection.ModifyXml("VISUAL_EFFECTS","SPEAKER_NOTATION","true")
    connection.Send()
#    sleep(10)

# set audio block mode        
    print "Setting audio block ... "
    connection.LoadXmlFile("Scripts/TestConfCorruptedParams/SetAudioBlock.xml")
    connection.ModifyXml("SET_AUDIO_BLOCK","ID",confid)
    connection.ModifyXml("SET_AUDIO_BLOCK","PARTY_ID",partyid)
    connection.ModifyXml("SET_AUDIO_BLOCK","AUDIO_BLOCK","true")
    connection.Send()
#    sleep(10)
    
# set audio volume        
    print "Setting audio volume ... "
    connection.LoadXmlFile("Scripts/TestConfCorruptedParams/SetAudioVolume.xml")
    connection.ModifyXml("SET_AUDIO_VOLUME","ID",confid)
    connection.ModifyXml("SET_AUDIO_VOLUME","PARTY_ID",partyid)    
    connection.ModifyXml("SET_AUDIO_VOLUME","VOLUME",str(2))
    connection.Send()
#    sleep(10)
    
# set listen audio volume        
    print "Setting listen audio volume ... "
    connection.LoadXmlFile("Scripts/TestConfCorruptedParams/SetListenAudioVolume.xml")
    connection.ModifyXml("SET_LISTEN_AUDIO_VOLUME","ID",confid)
    connection.ModifyXml("SET_LISTEN_AUDIO_VOLUME","PARTY_ID",partyid)    
    connection.ModifyXml("SET_LISTEN_AUDIO_VOLUME","LISTEN_VOLUME",str(8))        # 0 - 10
    connection.Send()
#    sleep(10)

# set party visual name        
    print "Setting party visual name ... "
    connection.LoadXmlFile("Scripts/TestConfCorruptedParams/SetPartyVisualName.xml")
    connection.ModifyXml("SET_PARTY_VISUAL_NAME","ID",confid)
    connection.ModifyXml("SET_PARTY_VISUAL_NAME","PARTY_ID",partyid)    
    connection.ModifyXml("SET_PARTY_VISUAL_NAME","NAME",long_char)
    connection.Send()
#    sleep(10)

# set audio video mute        
    print "Setting audio video mute ... "
    connection.LoadXmlFile("Scripts/TestConfCorruptedParams/SetAudioVideoMute.xml")
    connection.ModifyXml("SET_AUDIO_VIDEO_MUTE","ID",confid)
    connection.ModifyXml("SET_AUDIO_VIDEO_MUTE","PARTY_ID",partyid)    
    connection.ModifyXml("SET_AUDIO_VIDEO_MUTE","AUDIO_MUTE","true")
    connection.ModifyXml("SET_AUDIO_VIDEO_MUTE","VIDEO_MUTE","true")    
    connection.Send()
#    sleep(10)
    
# set AGC        
    print "Setting AGC ... "
    connection.LoadXmlFile("Scripts/TestConfCorruptedParams/SetAGC.xml")
    connection.ModifyXml("SET_AGC","ID",confid)
    connection.ModifyXml("SET_AGC","PARTY_ID",partyid)    
    connection.ModifyXml("SET_AGC","AGC","true")
    connection.Send()
#    sleep(60)

# set party contact info        
    print "Setting party contact info ... "
    connection.LoadXmlFile("Scripts/TestConfCorruptedParams/SetPartyContactInfo.xml")
    connection.ModifyXml("SET_PARTY_CONTACT_INFO","ID",confid)
    connection.ModifyXml("SET_PARTY_CONTACT_INFO","PARTY_ID",partyid)    
    connection.ModifyXml("CONTACT_INFO_LIST","CONTACT_INFO","CONTACT_INFO")
    connection.ModifyXml("CONTACT_INFO_LIST","CONTACT_INFO_2","CONTACT_INFO_2")
    connection.ModifyXml("CONTACT_INFO_LIST","CONTACT_INFO_3","CONTACT_INFO_3")
    connection.ModifyXml("CONTACT_INFO_LIST","CONTACT_INFO_4","CONTACT_INFO_4")    
    connection.Send()
#    sleep(10)

# set connect (party)        
#    print "Setting connect (party) ... "
#    connection.LoadXmlFile("Scripts/TestConfCorruptedParams/SetConnect.xml")
#    connection.ModifyXml("SET_CONNECT","ID",confid)
#    connection.ModifyXml("SET_CONNECT","PARTY_ID",partyid) 
#    connection.ModifyXml("SET_CONNECT","CONNECT","true")       
#    connection.Send()
#    sleep(10)




# set party video layout        
    print "Setting party video layout ... "
    connection.LoadXmlFile("Scripts/TestConfCorruptedParams/SetPartyVideoLayout2.xml")
    connection.ModifyXml("SET_PARTY_VIDEO_LAYOUT_EX","ID",confid)
    connection.ModifyXml("SET_PARTY_VIDEO_LAYOUT_EX","PARTY_ID",partyid)    
    connection.ModifyXml("SET_PARTY_VIDEO_LAYOUT_EX","LAYOUT_TYPE","personal")
    connection.ModifyXml("FORCE","LAYOUT","2x2")
    connection.ModifyXml("CELL","ID",str(1))
    connection.ModifyXml("CELL","FORCE_STATE","auto")
    connection.ModifyXml("CELL","FORCE_ID",str(-1))
#    connection.ModifyXml("CELL","FORCE_NAME",long_char)
    connection.ModifyXml("CELL","SOURCE_ID",str(-1))

    connection.Send()
    sleep(10)


    #print "Delete Conference..."
    connection.DeleteConf(confid)
         
    #print "Wait until no conferences"
    connection.WaitAllConfEnd(50)
    
    return

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

##--------------------------------------- TEST ---------------------------------

c = McmsConnection()
c.Connect()

AddConfAndParty(c,
                 'Scripts/AddVideoCpConf.xml',
                 'Scripts/AddVideoParty.xml',

                 20) #num of retries

c.Disconnect()
##------------------------------------------------------------------------------
