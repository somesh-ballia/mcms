#!/mcms/python/bin/python

# For list of profiles look at RunTest.sh
#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_8

#############################################################################
# Test Script which Add meeting room and checks the meeting room parameters : 
#                 TestMRCorruptedParams.py
#
# Date: 1/02/06
# By  : Ron S.

#############################################################################

from McmsConnection import *

###------------------------------------------------------------------------------
def CheckMRParams(connection,num_of_mr,mrCreateFile,timeout):


    long_char = "sababasababasababasababasababasababasababasababasababasababasababasababasababasababasababasababasababasababasababasababasababasababasababasababasababasababasababasababasababasababasababasababasababasababasababasababasababasababasababasababasaba"
    semi_long_char = "sababasababasababasababasababasababasababasababasababasababasababasababasababasababasababasababasabasababasababasababasababasababasababa"
    short_char = "sababa"

 #Add The Meeting Room Reservations    
    mrName = "MR1"
    connection.LoadXmlFile(mrCreateFile)
    connection.ModifyXml("RESERVATION","NAME",mrName)
    print "Adding New Meeting Room : " + mrName + "  ..."
 
    connection.ModifyXml("VISUAL_EFFECTS","LAYOUT_BORDER","true")        #true
    connection.ModifyXml("VISUAL_EFFECTS","SPEAKER_NOTATION","true")        #true
    connection.ModifyXml("RESERVATION","NETWORK","h323")           #h323
    connection.ModifyXml("RESERVATION","MEDIA",semi_long_char)             #video_audio
    connection.ModifyXml("RESERVATION","VIDEO_SESSION",semi_long_char)     #continuous_presence
    connection.ModifyXml("RESERVATION","VIDEO_PROTOCOL","auto")        #auto
    connection.ModifyXml("RESERVATION","TRANSFER_RATE",str(384))        #384
    connection.ModifyXml("RESERVATION","AUDIO_RATE","auto")        #auto
    connection.ModifyXml("RESERVATION","VIDEO_FORMAT","auto")        #auto
    connection.ModifyXml("RESERVATION","FRAME_RATE","auto")        #auto
    connection.ModifyXml("RESERVATION","ATTENDED_MODE","ivr")        #ivr
#    connection.ModifyXml("RESERVATION","AV_MSG","IVR1")        #IVR1
    connection.ModifyXml("RESERVATION","RESTRICT_MODE",semi_long_char)        #derestricted
    connection.ModifyXml("RESERVATION","T120_RATE","none")        #none
    connection.ModifyXml("RESERVATION","ENTRY_TONE","false")        #false
    connection.ModifyXml("RESERVATION","EXIT_TONE","false")        #false
#    connection.ModifyXml("RESERVATION","END_TIME_ALERT_TONE",str(-5))        #5
    connection.ModifyXml("RESERVATION","STAND_BY","false")        #false
    connection.ModifyXml("RESERVATION","PEOPLE_AND_CONTENT","false")        #false
    connection.ModifyXml("RESERVATION","ANNEX_N","false")        #false
    connection.ModifyXml("RESERVATION","ANNEX_P","false")        #false
    connection.ModifyXml("RESERVATION","ANNEX_F","false")        #false 
    connection.ModifyXml("RESERVATION","OPERATOR_CONF","false")        #false
    connection.ModifyXml("RESERVATION","SAME_LAYOUT","false")        #false
    connection.ModifyXml("RESERVATION","DUO_VIDEO","false")        #false
    connection.ModifyXml("AUTO_TERMINATE","ON","true")        #true
    connection.ModifyXml("AUTO_TERMINATE","TIME_BEFORE_FIRST_JOIN",str(-10))        #10
    connection.ModifyXml("AUTO_TERMINATE","TIME_AFTER_LAST_QUIT",str(1))        #1
    connection.ModifyXml("RESERVATION","AUDIO_MIX_DEPTH",str(5))        #5
    connection.ModifyXml("RESERVATION","TALK_HOLD_TIME",str(150))        #150
    connection.ModifyXml("RESERVATION","MAX_PARTIES","automatic")        #automatic
    connection.ModifyXml("MEET_ME_PER_CONF","ON","true")        #true
    connection.ModifyXml("MEET_ME_PER_CONF","AUTO_ADD","true")        #true
    connection.ModifyXml("MEET_ME_PER_CONF","MIN_NUM_OF_PARTIES",str(-5))        #0
    connection.ModifyXml("RESERVATION","CHAIR_MODE","none")        #none
    connection.ModifyXml("RESERVATION","ADVANCED_AUDIO","auto")        #auto
    connection.ModifyXml("RESERVATION","H323_BIT_RATE",str(384))        #384
    connection.ModifyXml("RESERVATION","TERMINATE_AFTER_LEADER_EXIT","false")        #false
    connection.ModifyXml("RESERVATION","START_CONF_LEADER","false")        #false
    connection.ModifyXml("MEETING_ROOM","ON","true")        #true
    connection.ModifyXml("MEETING_ROOM","LIMITED_SEQ","off")        #off
    connection.ModifyXml("MEETING_ROOM","MR_STATE","passive")        #passive
    connection.ModifyXml("RESERVATION","REPEATED_ID",str(0))        #0
    connection.ModifyXml("RESERVATION","VISUAL_CONCERT","false")        #false
    connection.ModifyXml("DURATION","HOUR",str(1))        # 0 - 99
    connection.ModifyXml("DURATION","MINUTE",str(20))        # 0 - 59
    connection.ModifyXml("DURATION","SECOND",str(10))        # 0 - 59
    connection.ModifyXml("RESERVATION","LOCK","false")        #false
    connection.ModifyXml("RESERVATION","LSD_RATE","dynamic")        #dynamic
    connection.ModifyXml("CASCADE","CASCADE_ROLE","none")        #none
    connection.ModifyXml("LECTURE_MODE","ON","false")        #false
    connection.ModifyXml("LECTURE_MODE","TIMER","false")        #false
    connection.ModifyXml("LECTURE_MODE","INTERVAL",str(15))        #15
    connection.ModifyXml("LECTURE_MODE","AUDIO_ACTIVATED","false")        #false
    connection.ModifyXml("LECTURE_MODE","LECTURE_NAME","[None]")        #[None]
    connection.ModifyXml("LECTURE_MODE","LECTURE_MODE_TYPE","lecture_none")        #lecture_none
    connection.ModifyXml("LECTURE_MODE","LECTURE_ID",str(-1))        #-1
    connection.ModifyXml("RESERVATION","LAYOUT","1x1")        #1x1
#    connection.ModifyXml("FORCE","LAYOUT","1x1")        #1x1
#    connection.ModifyXml("CELL","ID",str(1))        #1
#    connection.ModifyXml("CELL","FORCE_STATE","auto")        #auto
#   connection.ModifyXml("CELL","FORCE_ID",str(-1))        #-1
#    connection.ModifyXml("CELL","SOURCE_ID",str(-1))        #-1
    connection.ModifyXml("RESERVATION","DUAL_VIDEO_MODE","dynamic")        #dynamic
    connection.ModifyXml("RESERVATION","ENTRY_QUEUE","none")        #none
    connection.ModifyXml("RESERVATION","MEET_ME_PER_ENTRY_QUEUE","true")        #true
    connection.ModifyXml("RESERVATION","MUTE_INCOMING_PARTIES","false")        #false
    connection.ModifyXml("RESERVATION","ON_HOLD","false")        #false
    connection.ModifyXml("RESERVATION","ROLL_CALL","false")        #false
    connection.ModifyXml("RESERVATION","INVITE_PARTY","false")        #false
    connection.ModifyXml("RESERVATION","ADVANCED_VIDEO","auto")        #auto
    connection.ModifyXml("RESERVATION","INTERLACED_MODE","none")        #none
    connection.ModifyXml("RESERVATION","VIDEO_PLUS","true")        #true
    connection.ModifyXml("BACKGROUND_COLOR","RED",str(86))       # 0 - 255
    connection.ModifyXml("BACKGROUND_COLOR","GREEN",str(86))        # 0 - 255
    connection.ModifyXml("BACKGROUND_COLOR","BLUE",str(83))        # 0 - 255
    connection.ModifyXml("VISUAL_EFFECTS","LAYOUT_BORDER","true")        #true
    connection.ModifyXml("VISUAL_EFFECTS","SPEAKER_NOTATION","true")        #true
    connection.ModifyXml("SPEAKER_NOTATION_COLOR","RED",str(83))        # 0 - 255
    connection.ModifyXml("SPEAKER_NOTATION_COLOR","GREEN",str(83))        # 0 - 255
    connection.ModifyXml("SPEAKER_NOTATION_COLOR","BLUE",str(83))        # 0 - 255
    connection.ModifyXml("RESERVATION","COP","false")        #false
    connection.ModifyXml("RESERVATION","PROFILE","false")        #false
    connection.ModifyXml("RESERVATION","AD_HOC","false")        #false
    connection.ModifyXml("RESERVATION","AD_HOC_PROFILE_ID",str(-1))        #-1
    connection.ModifyXml("RESERVATION","AUTO_LAYOUT","false")        #false
#    connection.ModifyXml("RESERVATION","NUMERIC_ID",str(86383))        #86383
#    connection.ModifyXml("RESERVATION","BILLING_DATA","just_checking")        #
#    connection.ModifyXml("CONTACT_INFO_LIST","CONTACT_INFO","just_checking")        #
#    connection.ModifyXml("CONTACT_INFO_LIST","CONTACT_INFO_2","just_checking")        #
#   connection.ModifyXml("CONTACT_INFO_LIST","CONTACT_INFO_3","just_checking")        #
    connection.ModifyXml("RESERVATION","HSD_RATE","dynamically")        #dynamically
    connection.ModifyXml("RESERVATION","ENCRYPTION","false")        #false
    connection.ModifyXml("RESERVATION","CONF_CONTROL","h234")        #h243
    connection.ModifyXml("RESERVATION","CONF_PROTOCOL","h234")        #h243
    connection.ModifyXml("RESERVATION","EXTERNAL_MASTER","true")        #true
    connection.ModifyXml("RESERVATION","VIDEO","false")        #false
    connection.ModifyXml("RESERVATION","QCIF_FRAME_RATE","auto")        #auto
    connection.ModifyXml("RESERVATION","COUGH_DELAY",str(0))        #0
    connection.ModifyXml("RESERVATION","GUEST_OPER","false")        #false
    connection.ModifyXml("RESERVATION","WEB_RESERVED_UID",str(0))        #0
    connection.ModifyXml("RESERVATION","WEB_OWNER_UID",str(0))        #0
    connection.ModifyXml("RESERVATION","WEB_DB_ID",str(0))        #0
    connection.ModifyXml("RESERVATION","WEB_RESERVED","false")        #false
    connection.ModifyXml("RESERVATION","CONFERENCE_TYPE","unknown")        #unknown
    connection.ModifyXml("RESERVATION","COP_NUM_OF_PORTS","single")        #single
    connection.ModifyXml("RESERVATION","SILENCE_IT","false")        #false
    connection.ModifyXml("RESERVATION","IS_RECORDING_LINK","false")        #false
    connection.ModifyXml("RESERVATION","ENABLE_RECORDING","false")        #false
    connection.ModifyXml("RESERVATION","START_REC_POLICY","immediately")        #immediately
    connection.ModifyXml("RESERVATION","REC_LINK_ID",str(0))        #0
    connection.ModifyXml("RESERVATION","VIDEO_QUALITY","auto")        #auto
    connection.ModifyXml("RESERVATION","ENTRY_QUEUE_TYPE","normal")        #normal
    connection.ModifyXml("END_TIME_ALERT_TONE_EX","ON","false")        #false
    connection.ModifyXml("END_TIME_ALERT_TONE_EX","TIME",str(0))        #0
#    connection.ModifyXml("RESERVATION","VTX","false")        #false
#    connection.ModifyXml("RESERVATION","CASCADE_EQ","false")        #false
#    connection.ModifyXml("RESERVATION","SIP_FACTORY","false")        #false
#    connection.ModifyXml("RESERVATION","SIP_FACTORY_AUTO_CONNECT","false")        #false
 
    connection.Send("")
    print "Wait untill the MR :" + mrName + " will be createtd..."
   
    sleep(5)

#make sure all Meeting rooms were added
    print "Wait untill the MR :" + mrName + " will be createtd..."
    for retry in range(timeout+1):
        connection.SendXmlFile('Scripts/TestMRCorruptedParams/TransMrList.xml',"Status OK") 
        mr_list = connection.xmlResponse.getElementsByTagName("MEETING_ROOM_SUMMARY")
        if len(mr_list) == 1 :
            mrId = mr_list[0].getElementsByTagName("ID")[0].firstChild.data
            print
            break
        if (retry == timeout+1):
            print connection.xmlResponse.toprettyxml()
            connection.Disconnect()                
            sys.exit("Can not monitor conf:" + status)
            sys.stdout.write(".")
            sys.stdout.flush()

 #Make Sure All Mr's alive
    MakeSureAllMrIsAlive(connection,num_of_mr)

 #Monitor the delta and make sure it is empty
    connection.SendXmlFile('Scripts/TestMRCorruptedParams/TransMrList.xml',"Status OK")

    
    objToken =connection.xmlResponse.getElementsByTagName("MEETING_ROOM_SUMMARY_LS")[0].getElementsByTagName("OBJ_TOKEN")[0].firstChild.data
    
 #send the objToken and make sure that we get it back in the same value
    responsToken=CheckObjToken(connection,objToken)
    if responsToken != objToken:
        print "Error: Monitoring the Delta of the Mr list failed"
        connection.Disconnect()                
        sys.exit("Obj token are not equal")
    print "Delta monitoring is passed, the list did not changed..."



 #print "delete all Meeting Rooms"
    DeleteAllMeetingRooms(connection)
    
    #print "waiting until all Meeting Rooms end"
    WaitAllMrEnd(connection,timeout)
    return

#------------------------------------------------------------------------------
def CheckObjToken(connection,objToken):
    #send the objToken and make sure that we get it back in the same value
    connection.LoadXmlFile('Scripts/TestMRCorruptedParams/TransMrList.xml') 
    connection.ModifyXml("GET_MEETING_ROOM_LIST","OBJ_TOKEN",objToken)
    connection.Send()
    responsToken = connection.xmlResponse.getElementsByTagName("MEETING_ROOM_SUMMARY_LS")[0].getElementsByTagName("OBJ_TOKEN")[0].firstChild.data
    return responsToken

#------------------------------------------------------------------------------
def MonitorSingleMr(connection,mrId,fieldName,fieldValue,retires):
    print "Monitoring mr id " + mrId+" and checks if "+fieldName+" is equal to "+fieldValue
    for retry in range(retires+1):    
        connection.LoadXmlFile('Scripts/TestMRCorruptedParams/GetMr.xml')
        connection.ModifyXml("GET_MEETING_ROOM","ID",mrId)
        connection.Send()
        #get the changed value
        mrRsrvList = connection.xmlResponse.getElementsByTagName("RESERVATION")
        mrFiledNameVal = mrRsrvList[0].getElementsByTagName(fieldName)[0].firstChild.data
        if mrFiledNameVal == fieldValue:
            break
        if retry == retires:
            print "Error: Field " +fieldName +" got: " +mrFiledNameVal + " while we expected " +fieldValue 
            connection.Disconnect()
            sys.exit("Monitoring singke Mr Failed")
    print "Monitoring of single Mr passed and "+fieldName+" = " + fieldValue
#------------------------------------------------------------------------------
def UpdateMrField(connection,mrName,mrId,fieldName,fieldValue,status="Status OK"):
    print "Updating the field: " + fieldName+" to "+ fieldValue + " in mr id " + mrId
    connection.LoadXmlFile('Scripts/TestMRCorruptedParams/UpdateMr.xml')
    connection.ModifyXml("RESERVATION","NAME",mrName)
    connection.ModifyXml("RESERVATION","ID",mrId)
    connection.ModifyXml("RESERVATION",fieldName,fieldValue)
    connection.Send(status)
    
#------------------------------------------------------------------------------
def MakeSureAllMrIsAlive(connection,num_of_mr):
    #get the mr list and make sure all mr's are there
    connection.SendXmlFile('Scripts/TestMRCorruptedParams/TransMrList.xml',"Status OK") 
    mr_list = connection.xmlResponse.getElementsByTagName("MEETING_ROOM_SUMMARY")
    if len(mr_list) != num_of_mr :
        print "Error: Not all Mr's were found in the list, found only "+ str(len(mr_list))+" Mr's"
        connection.Disconnect()                
        sys.exit("Can not find Mrs")
    print "All " + str(num_of_mr) + " Mr's are still alive in monitoring list"
#------------------------------------------------------------------------------
def DeleteAllMeetingRooms(connection):
    print "Delete all Meeting Rooms..."
    connection.SendXmlFile('Scripts/TestMRCorruptedParams/TransMrList.xml',"Status OK")
    mr_list = connection.xmlResponse.getElementsByTagName("MEETING_ROOM_SUMMARY")
    for x in range(len(mr_list)):
        mr_id = mr_list[x].getElementsByTagName("ID")[0].firstChild.data
        connection.DelReservation(mr_id, 'Scripts//TestMRCorruptedParams/DeleteMR.xml')
    return

#------------------------------------------------------------------------------
def WaitAllMrEnd(connection,retires = 20):
    print "Waiting until all conferences MR end",
    for retry in range(retires+1):
        sleep(1)
        sys.stdout.write(".")
        sys.stdout.flush()
        connection.SendXmlFile('Scripts/TestMRCorruptedParams/TransMrList.xml',"Status OK")
        if connection.GetTextUnder("MEETING_ROOM_SUMMARY","ID") == "":
            print
            break
        if retry == retires:
            connection.Disconnect()
            sys.exit("Failed delete conference!!!")
    return


## ---------------------- Test --------------------------

c = McmsConnection()
c.Connect()
CheckMRParams(c, #the connection class
              1, # num of Meeting Rooms
              'Scripts/TestMRCorruptedParams/CreateMR.xml', #Meeting Room Script
 #             'Scripts/TestMRCorruptedParams/updateMR.xml',
              20) #num mof retries
c.Disconnect()

##------------------------------------------------------------------------------
    
