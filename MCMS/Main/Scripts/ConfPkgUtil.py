#!/usr/bin/python

# For list of profiles look at RunTest.sh
#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_4

from McmsConnection import *
from string import *

#-------------------------------------------------------
# Get svc streams of users in conf event pack of svc conf
# if pprint the returned map, result will like below:
# {u'Party1': {'bit-rate':   [u'192',      u'384',      u'768'],
#              'frame-rate': [u'300',      u'300',      u'300'],
#              'height':     [u'180',      u'360',      u'720'],
#              'src-id':     [u'67174480', u'67174481', u'67174482'],
#              'width':      [u'320',      u'640',      u'1280']},
#  u'Party2': {'bit-rate':   [u'192',      u'384',      u'768'],
#              'frame-rate': [u'300',      u'300',      u'300'],
#              'height':     [u'180',      u'360',      u'720'],
#              'src-id':     [u'67174544', u'67174545', u'67174546'],
#              'width':      [u'320',      u'640',      u'1280']},
#  u'Party3': {'bit-rate':   [u'192',      u'384',      u'768'],
#              'frame-rate': [u'300',      u'300',      u'300'],
#              'height':     [u'180',      u'360',      u'720'],
#              'src-id':     [u'67174608', u'67174609', u'67174610'],
#              'width':      [u'320',      u'640',      u'1280']}}
def GetUserVideoStreamsMap(notify):
    userVideoMap = {}
    usersList = notify.getElementsByTagName("user")
    for user in usersList:
        #print user.getElementsByTagName("display-text")[0].firstChild.data
        display_text = user.getElementsByTagName("display-text")[0].firstChild.data
        mediaList = user.getElementsByTagName("media") 
        userVideoMap[display_text] = {"src-id":[], "width":[], "height":[], "frame-rate":[], "bit-rate":[]}
        for media in mediaList:
            #print media.getElementsByTagName("type")[0].firstChild.data 
            if media.getElementsByTagName("type")[0].firstChild.data == "video":
                #print media.getElementsByTagName("src-id")[0].firstChild.data
                userVideoMap[display_text]["src-id"].append(media.getElementsByTagName("src-id")[0].firstChild.data)
                userVideoMap[display_text]["width"].append(media.getElementsByTagName("mrc:max-resolution")[0].getAttribute("width"))
                userVideoMap[display_text]["height"].append(media.getElementsByTagName("mrc:max-resolution")[0].getAttribute("height"))
                userVideoMap[display_text]["frame-rate"].append(media.getElementsByTagName("mrc:frame-rate")[0].getAttribute("max"))
                userVideoMap[display_text]["bit-rate"].append(media.getElementsByTagName("mrc:bit-rate")[0].getAttribute("max"))
    
    return userVideoMap
            
#--------------------------------------------------------
def CheckMediaValidity(endPoint, type):
    mediaList = endPoint.getElementsByTagName("media")
    for i in range(len(mediaList)):
        media = mediaList[i]
        mediaId = media.getAttribute("id")
        mediaType = media.getElementsByTagName("type")[0].firstChild.data
        if(mediaType == type):
            return True
        mediaStatus = media.getElementsByTagName("status")[0]
        
    print "Media type "+type+" was not found."
    return False        
    
#--------------------------------------------------------
def CheckActionValidity(action, actionName):
    actionEntity = action.firstChild.data
#    print "action gotted : " +  actionName 
#    print "action excepted : " +  actionEntity 
    if(actionEntity):
        if(not actionEntity == actionName):
            print "action gotted : " +  actionName 
            print "action excepted : " +  actionEntity            
            return False
    return True
   
#--------------------------------------------------------
def CheckActionsValidity(notifyContent, UserPosition, actionName, position):
    result = True
    
    usersList = notifyContent.getElementsByTagName("user")
    for i in range(len(usersList)):
        if(not i == UserPosition):
            continue
        else:    
         #   user = usersList[i]
         #   userEntity = user.getAttribute("entity")
         #   print userEntity
            actionsList = notifyContent.getElementsByTagName("action")
            '''            
            if(position==0):
                if(numActions != len(actionsList)):
                    print "Found "+str(len(actionsList))+" actions in NOTIFY, while expecting to find "+str(numActions)
                    result = False
            '''        
            for i in range(len(actionsList)):
                if(not i == position):
                    continue
                else: 
                  #  print "the action num is : " + str(i)
                    action = actionsList[i]                      
                    result = CheckActionValidity(action, actionName)
                    if(not result):
                        result = False
 
    if(not result):
        sys.exit("Invalid Actions...")

#--------------------------------------------------------
def CheckEndPointValidity(endPoint):
    epEntity = endPoint.getAttribute("entity")
    epStatus = endPoint.getElementsByTagName("status")[0]
    result = CheckMediaValidity(endPoint, "audio")
    return result

#--------------------------------------------------------
def CheckUserValidity(user):
    userEntity = user.getAttribute("entity")
    if(userEntity):
        notifyState = user.getAttribute("state")
        if(not notifyState == 'deleted'):
            endPoint = user.getElementsByTagName("endpoint")[0]
            return CheckEndPointValidity(endPoint)
    return True
    
    
#--------------------------------------------------------
def CheckUsersValidity(notifyContent, numParties):
    usersList = notifyContent.getElementsByTagName("user")
    if(numParties != len(usersList)):
        print "Found "+str(len(usersList))+" users in NOTIFY, while expecting to find "+str(numParties)
        return False
    
    for i in range(len(usersList)):
        user = usersList[i]
        result = CheckUserValidity(user)
        if(not result):
            return False
    return True
        

#--------------------------------------------------------
def CheckNotifyValidity(notifyContent, entity, state, numParties, version):
    result = True
    notifyEntity = notifyContent.getElementsByTagName("conference-info")[0].getAttribute("entity")
    notifyState = notifyContent.getElementsByTagName("conference-info")[0].getAttribute("state")
    notifyVersion = int(notifyContent.getElementsByTagName("conference-info")[0].getAttribute("version"))
    if (notifyEntity != entity):
        print "Entity does not match:" + notifyEntity
        result = False
    if (notifyState != state):
        print "State does not match:" + notifyState
        result = False
    if(notifyState != "full"):
        if(notifyVersion != version):
            print "Error in version number: found "+str(notifyVersion)+" while expecting to find "+str(version)
            result = False
    if(result & numParties>0):
        result = CheckUsersValidity(notifyContent, numParties)
    if(not result):
        sys.exit("Invalid Notify...")
'''    
#--------------------------------------------------------
def CheckConfContactInfo(notify,confInfo):
    contactInfoList = notify.getElementsByTagName("contact-info")
    for i in range(len(contactInfoList)):
        contactInfo = contactInfoList[i]
    
    CheckContactInfo(contactInfo, confInfo, "conference")            
'''    
#--------------------------------------------------------
def CheckConfContactInfo(notify,confInfo):
    exDataList = notify.getElementsByTagName("ex-data")
    if(len(exDataList) == 0):
        sys.exit("No Conf ex-data exist...")
    for i in range(len(exDataList)):
        exData = exDataList[i]
        
    contactInfoList = exData.getElementsByTagName("contact-info")
    '''
    for i in range(len(contactInfoList)):
        contactInfo = contactInfoList[i]
        break    
    '''
    CheckContactInfo(contactInfoList[0], confInfo, "conference")            

#--------------------------------------------------------
def CheckPartyType(notify, position, netType, mediaType, state ,userInfo , status='' ):
    result = False
    usersList = notify.getElementsByTagName("user")
    for i in range(len(usersList)):
        if(not i == position):
            continue
        else:
            user = usersList[i]
            
            contactInfo = user.getElementsByTagName("contact-info")[0]
            CheckContactInfo(contactInfo, userInfo, "user")            
            
            userEntity = user.getAttribute("entity")
            #Test network type
            if netType == 'sip':
                if ";user=" not in userEntity:
                    result = True
                else:
                    result = False
            if ";user="+netType in userEntity:
                result = True
            #Test state
            userState = user.getAttribute("state")
            if(not userState == state):
                print "Wrong state: " + userState
                result = False
            else:
                #only if state is 'full' check media
                if(userState == 'full'):
                    #Test media types
                    if(result):
                        endPoint = user.getElementsByTagName("endpoint")[0]
                        epStatus = endPoint.getElementsByTagName("status")[0].firstChild.data
                        if(not epStatus == status):
                            print "Wrong EP status: " + epStatus
                            result = False
                        else:                            
                            if mediaType == 'video':
                                result = CheckMediaValidity(endPoint, "video")
                            if result & ((mediaType == 'audio') | (mediaType == 'video')):
                                result = CheckMediaValidity(endPoint, "audio")
    if(not result):
        sys.exit("Invalid Party Type...")
        
#--------------------------------------------------------
def CheckMuteViaFocus(notify, party):
    result = False
    usersList = notify.getElementsByTagName("user")
    for i in range(len(usersList)):
        user = usersList[i]
        userEntity = user.getAttribute("entity")
        if(party == userEntity):
            status = user.getElementsByTagName("status")[0].firstChild.data
            if(status == 'muted-via-focus'):
                result = True
    if(not result):
        sys.exit("Muted via focus is wrong...")

#--------------------------------------------------------
def CheckUnMuteViaFocus(notify, party):
    result = False
    usersList = notify.getElementsByTagName("user")
    for i in range(len(usersList)):
        user = usersList[i]
        userEntity = user.getAttribute("entity")
        if(party == userEntity):
            status = user.getElementsByTagName("status")[0].firstChild.data
            if(status == 'connected'):
                result = True
    if(not result):
        sys.exit("UnMute is wrong...")

#--------------------------------------------------------        
def CheckContactInfo(notify, info, entity):
    contactInfo = notify.firstChild.data
#    print "the contact info is(from notify msg) : " + contactInfo
#    print "the contact info is(eccepted) : " + info
    if(not contactInfo==info):
        sys.exit("The contact info of the " + entity +" is wrong...")
     
#--------------------------------------------------------        
def CheckActiveSpeaker(notify, party, speakerInfo):
    result = False
#    activeSpeaker = notify.getElementsByTagName("active-speaker")[0].firstChild.data
    activeSpeaker = notify.getElementsByTagName("active-speaker")[0]
    activeSpeakerEntity = activeSpeaker.getAttribute("entity")
    
    contactInfo = activeSpeaker.getElementsByTagName("contact-info")[0]
    CheckContactInfo(contactInfo, speakerInfo, "active-speaker")
    
    if(party == activeSpeakerEntity):
        result = True
    if(not result):
        sys.exit("Active speaker is wrong...")
       
#==================================================================================
#  mute functions
#==================================================================================

#------------------------------------------------------------------------------
def WaitPartyAudioAppearsAsMuted(connection, confid, partyid, num_retries=30):
    print "Wait until party Appears with Video Mute: " + str(partyid) 
    connection.LoadXmlFile('Scripts/TransConf2.xml')
    connection.ModifyXml("GET","ID",confid)
    for retry in range(num_retries+1):
        connection.Send()
        ongoing_parties = connection.xmlResponse.getElementsByTagName("ONGOING_PARTY")
        if(ongoing_parties[partyid].getElementsByTagName("AUDIO_MUTE")[0].firstChild.data == "true"):
            break
        if(retry == num_retries):
            print connection.xmlResponse.toprettyxml()
            connection.Disconnect()
            sys.exit("Party not Video Muted: "+ str(partyid)) 
        sys.stdout.write(".")
        sys.stdout.flush()
        sleep(1)    
#------------------------------------------------------------------------------
def WaitPartyAudioAppearsAsUnMuted(connection, confid, partyid, num_retries=30):
    print "Wait until party Appears with Video Mute: " + str(partyid) 
    connection.LoadXmlFile('Scripts/TransConf2.xml')
    connection.ModifyXml("GET","ID",confid)
    for retry in range(num_retries+1):
        connection.Send()
        ongoing_parties = connection.xmlResponse.getElementsByTagName("ONGOING_PARTY")
        if(ongoing_parties[partyid].getElementsByTagName("AUDIO_MUTE")[0].firstChild.data == "false"):
            break
        if(retry == num_retries):
            print connection.xmlResponse.toprettyxml()
            connection.Disconnect()
            sys.exit("Party not Video Muted: "+ str(partyid)) 
        sys.stdout.write(".")
        sys.stdout.flush()
        sleep(1)    
                  
#------------------------------------------------------------------------------
def MutePartyVideoTest(connection,confid, partyid):
    print "Conference ID: "+ confid + " Muting Party Video: PartyId: " + str(partyid)
    connection.LoadXmlFile('Scripts/MuteVideo/MutePartyVideo.xml')
    connection.ModifyXml("SET_AUDIO_VIDEO_MUTE","ID",confid)
    connection.ModifyXml("SET_AUDIO_VIDEO_MUTE","PARTY_ID",partyid) 
    connection.ModifyXml("SET_AUDIO_VIDEO_MUTE","VIDEO_MUTE","true")  
    connection.Send()
    WaitPartyVideoAppearsAsMuted(connection, confid, partyid)
    #WaitNoOneSeesPartyInLayout(connection, confid, partyid)
    return

#------------------------------------------------------------------------------
def MuteParty(connection, partyName):
    print " Muting Party : partyName: " + partyName
    connection.LoadXmlFile('Scripts/SimMuteParty.xml')
    connection.ModifyXml("MUTE","PARTY_NAME",partyName)
    connection.Send()
    #WaitPartyVideoAppearsAsMuted(connection, confid, partyid)
    #WaitNoOneSeesPartyInLayout(connection, confid, partyid)
    return

#------------------------------------------------------------------------------
def MutePartyAudioTest(connection,confid, partyid):
    print "Conference ID: "+ confid + " Muting Party Video: PartyId: " + str(partyid)
    connection.LoadXmlFile('Scripts/MuteVideo/MutePartyVideo.xml')
    connection.ModifyXml("SET_AUDIO_VIDEO_MUTE","ID",confid)
    connection.ModifyXml("SET_AUDIO_VIDEO_MUTE","PARTY_ID",partyid) 
    connection.ModifyXml("SET_AUDIO_VIDEO_MUTE","AUDIO_MUTE","true")  
    connection.Send()
    WaitPartyVideoAppearsAsMuted(connection, confid, partyid)
    #WaitNoOneSeesPartyInLayout(connection, confid, partyid)
    return

#------------------------------------------------------------------------------
def MutePartyVideoAudioTest(connection,confid, partyid):
    print "Conference ID: "+ confid + " Muting Party Video: PartyId: " + str(partyid)
    connection.LoadXmlFile('Scripts/MuteVideo/MutePartyVideo.xml')
    connection.ModifyXml("SET_AUDIO_VIDEO_MUTE","ID",confid)
    connection.ModifyXml("SET_AUDIO_VIDEO_MUTE","PARTY_ID",partyid) 
    connection.ModifyXml("SET_AUDIO_VIDEO_MUTE","AUDIO_MUTE","true")
#   connection.ModifyXml("SET_AUDIO_VIDEO_MUTE","VIDEO_MUTE","true")   
    connection.Send()
    WaitPartyAudioAppearsAsMuted(connection, confid, partyid)
    #WaitNoOneSeesPartyInLayout(connection, confid, partyid)
    return

#------------------------------------------------------------------------------
def UnMutePartyVideoAudioTest(connection,confid, partyid):
    print "Conference ID: "+ confid + " Muting Party Video: PartyId: " + str(partyid)
    connection.LoadXmlFile('Scripts/MuteVideo/MutePartyVideo.xml')
    connection.ModifyXml("SET_AUDIO_VIDEO_MUTE","ID",confid)
    connection.ModifyXml("SET_AUDIO_VIDEO_MUTE","PARTY_ID",partyid) 
    connection.ModifyXml("SET_AUDIO_VIDEO_MUTE","AUDIO_MUTE","false")
#    connection.ModifyXml("SET_AUDIO_VIDEO_MUTE","VIDEO_MUTE","false")   
    connection.Send()
    WaitPartyAudioAppearsAsUnMuted(connection, confid, partyid)
    #WaitNoOneSeesPartyInLayout(connection, confid, partyid)
    return

#------------------------------------------------------------------------------
def CreateConfWithInfo(connection,confname, confFile):  
    """Create a new conference.
    
    confName - conference name
    fileName - the xml file which will be used to define the conference.
    """
    connection.LoadXmlFile(confFile)
    connection.ModifyXml("CONTACT_INFO_LIST","CONTACT_INFO",confname+"@plcm.com")
    
    print "Adding Conf " + confname + " ..."
    connection.ModifyXml("RESERVATION","NAME",confname)
    connection.Send()

#------------------------------------------------------------------------------
def AddPartyWithInfo(connection,confid, partyname, partyIp, partyFile,expected_status="Status OK"):
    """Add a new party.
    
    confid - destination conference.
    partyname - party name.
    partyIp - ip address for new party
    partyFile - xml file which will be used to define the party
    """
    print "Adding Party..." + partyname
    connection.LoadXmlFile(partyFile)
    connection.ModifyXml("PARTY","NAME",partyname)
    connection.ModifyXml("PARTY","IP",partyIp)
    connection.ModifyXml("ADD_PARTY","ID",confid)
    connection.ModifyXml("CONTACT_INFO_LIST","CONTACT_INFO",partyname+"@"+partyIp)
    connection.Send(expected_status)    

#------------------------------------------------------------------------------
def AddSIPPartyWithInfo(connection,confid, partyname, partyIp, partySipAdd, partyFile):
    """Add a new SIP party.
    
    confid - destination conference.
    partyname - party name.
    partyIp - ip address for new party
    partyFile - xml file which will be used to define the party
    """
    print "Adding SIP Party..." + partyname
    connection.LoadXmlFile(partyFile)
    connection.ModifyXml("PARTY","NAME",partyname)
    connection.ModifyXml("PARTY","IP",partyIp)
    connection.ModifyXml("ADD_PARTY","ID",confid)
    connection.ModifyXml("PARTY","SIP_ADDRESS",partySipAdd)
    connection.ModifyXml("CONTACT_INFO_LIST","CONTACT_INFO",partyname+"@"+partyIp)
    connection.Send()   
 
#==================================================================================
