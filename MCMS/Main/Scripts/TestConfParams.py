#!/mcms/python/bin/python

# For list of profiles look at RunTest.sh
#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_8

#############################################################################
# Test Script which Create  Conf with 1  participant and 
#           Check the params of all the SET transactions.
# 
# Date: 30/01/06
# By  : Ron S.
#
#############################################################################

from McmsConnection import *

###------------------------------------------------------------------------------
def AddConfAndParty(connection,confFile,numRetries):
    
    long_char = "sababasababasababasababasababasababasababasababasababasababasababasababasababasababasababasababasababasababasababasababasababasababasababasababasababasababasababasababasababasababasababasababasababasababasababasababasababasababasababasababasaba"
    semi_long_char = "sababasababasababasababasababasababasababasababasababasababasababasababasababasababasababasababasabasababasababasababasababasababasababa"
    short_char = "sababa"
    
    confname = "Conf"
    connection.LoadXmlFile(confFile)
    connection.ModifyXml("RESERVATION","NAME",confname)
    print "Adding Conf:" + confname + "  ..."
 
    connection.ModifyXml("RESERVATION","NETWORK","h323")        #h323
    connection.ModifyXml("RESERVATION","MEDIA","video_audio")        #video_audio
    connection.ModifyXml("RESERVATION","VIDEO_SESSION","continuous_presence")        #continuous_presence
    connection.ModifyXml("RESERVATION","VIDEO_PROTOCOL","auto")        #auto
    connection.ModifyXml("RESERVATION","TRANSFER_RATE","2564")        #384   //2x64
    connection.ModifyXml("RESERVATION","AUDIO_RATE","g728")        #g728
    connection.ModifyXml("RESERVATION","VIDEO_FORMAT","auto")        #auto
    connection.ModifyXml("RESERVATION","FRAME_RATE","auto")        #auto
    connection.ModifyXml("RESERVATION","ATTENDED_MODE","none")        #none
    connection.ModifyXml("RESERVATION","RESTRICT_MODE","derestricted")        #derestricted
    connection.ModifyXml("RESERVATION","T120_RATE","none")        #none
    connection.ModifyXml("RESERVATION","ENTRY_TONE","false")        #false
    connection.ModifyXml("RESERVATION","EXIT_TONE","false")        #false
    connection.ModifyXml("RESERVATION","STAND_BY","false")        #false
    connection.ModifyXml("RESERVATION","PEOPLE_AND_CONTENT","false")        #false
    connection.ModifyXml("RESERVATION","ANNEX_N","false")        #false
    connection.ModifyXml("RESERVATION","ANNEX_P","false")        #false
    connection.ModifyXml("RESERVATION","ANNEX_F","false")        #false
    connection.ModifyXml("RESERVATION","OPERATOR_CONF","false")        #false
    connection.ModifyXml("RESERVATION","SAME_LAYOUT","false")        #false
    
    connection.ModifyXml("RESERVATION","DUO_VIDEO","false")        #false
    connection.ModifyXml("AUTO_TERMINATE","ON","false")        #false
    connection.ModifyXml("AUTO_TERMINATE","TIME_BEFORE_FIRST_JOIN",str(-5))        #5
    connection.ModifyXml("AUTO_TERMINATE","TIME_AFTER_LAST_QUIT",str(5))        #1
    connection.ModifyXml("RESERVATION","AUDIO_MIX_DEPTH",str(3))        #3
    connection.ModifyXml("RESERVATION","TALK_HOLD_TIME",str(150))        #150
    connection.ModifyXml("RESERVATION","MAX_PARTIES","automatic")        #automatic
    connection.ModifyXml("MEET_ME_PER_CONF","ON","false")        #false
    connection.ModifyXml("MEET_ME_PER_CONF","AUTO_ADD","false")        #false
    connection.ModifyXml("MEET_ME_PER_CONF","MIN_NUM_OF_PARTIES",str(0))        #0
    connection.ModifyXml("RESOURCE_FORCE","AUDIO_BOARD","automatic")        #automatic
    connection.ModifyXml("RESOURCE_FORCE","VIDEO_BOARD","automatic")        #automatic
    connection.ModifyXml("RESOURCE_FORCE","DATA_BOARD","automatic")        #automatic
    connection.ModifyXml("RESOURCE_FORCE","DATA_UNIT","automatic")        #automatic
    connection.ModifyXml("RESOURCE_FORCE","AUDIO_UNIT",str(1))        #1   
    connection.ModifyXml("RESOURCE_FORCE","VIDEO_UNIT","automatic")        #automatic
    connection.ModifyXml("RESERVATION","ADVANCED_AUDIO","auto")        #auto
    connection.ModifyXml("RESERVATION","H323_BIT_RATE",str(381))        #384
    connection.ModifyXml("RESERVATION","TERMINATE_AFTER_LEADER_EXIT","false")        #false
    connection.ModifyXml("RESERVATION","START_CONF_LEADER","false")        #false
    connection.ModifyXml("MEETING_ROOM","ON","false")        #false
    connection.ModifyXml("MEETING_ROOM","LIMITED_SEQ","off")        #off
    connection.ModifyXml("RESERVATION","REPEATED_ID",str(0))        #0
    connection.ModifyXml("RESERVATION","VISUAL_CONCERT","false")        #false
    connection.ModifyXml("DURATION","HOUR",str(1))        # 0 - 99
    connection.ModifyXml("DURATION","MINUTE",str(20))        # 0 - 59
    connection.ModifyXml("DURATION","SECOND",str(10))        # 0 - 59
    
    connection.ModifyXml("RESERVATION","LOCK","false")        #false
    connection.ModifyXml("RESERVATION","LSD_RATE","none")        #none
    connection.ModifyXml("CASCADE","CASCADE_ROLE","none")        #none
    connection.ModifyXml("LECTURE_MODE","ON","false")        #false
    connection.ModifyXml("LECTURE_MODE","TIMER","false")        #false
    connection.ModifyXml("LECTURE_MODE","INTERVAL",str(15))        #15
    connection.ModifyXml("LECTURE_MODE","AUDIO_ACTIVATED","true")        #true
    connection.ModifyXml("LECTURE_MODE","LECTURE_NAME","[Auto Select]")        #[Auto Select]
    connection.ModifyXml("LECTURE_MODE","LECTURE_MODE_TYPE","lecture_none")        #lecture_none
    connection.ModifyXml("LECTURE_MODE","LECTURE_ID",str(-1))        #-1
    connection.ModifyXml("RESERVATION","LAYOUT","1x1")        #1x1
    connection.ModifyXml("FORCE","LAYOUT","1x1")        #1x1
    connection.ModifyXml("CELL","ID",str(1))        #1
    connection.ModifyXml("CELL","FORCE_STATE","auto")        #auto
    connection.ModifyXml("CELL","SOURCE_ID",str(-1))        #-1
    connection.ModifyXml("RESERVATION","DUAL_VIDEO_MODE","none")        #none
    connection.ModifyXml("RESERVATION","ENTRY_QUEUE","false")        #false
    connection.ModifyXml("RESERVATION","MEET_ME_PER_ENTRY_QUEUE","false")        #false
    connection.ModifyXml("RESERVATION","MUTE_INCOMING_PARTIES","false")        #false
    connection.ModifyXml("RESERVATION","ON_HOLD","false")        #false
    connection.ModifyXml("RESERVATION","ROLL_CALL","false")        #false
    connection.ModifyXml("RESERVATION","INVITE_PARTY","false")        #false
    connection.ModifyXml("RESERVATION","ADVANCED_VIDEO","auto")        #auto
    connection.ModifyXml("RESERVATION","INTERLACED_MODE","none")        #none
 
    connection.ModifyXml("RESERVATION","VIDEO_PLUS","false")        #false
    connection.ModifyXml("BACKGROUND_COLOR","RED",str(86))       # 0 - 255
    connection.ModifyXml("BACKGROUND_COLOR","GREEN",str(86))        # 0 - 255
    connection.ModifyXml("BACKGROUND_COLOR","BLUE",str(83))        # 0 - 255
    connection.ModifyXml("VISUAL_EFFECTS","LAYOUT_BORDER","true")        #true
    connection.ModifyXml("LAYOUT_BORDER_COLOR","RED",str(83))        # 0 - 255
    connection.ModifyXml("LAYOUT_BORDER_COLOR","GREEN",str(83))        # 0 - 255
    connection.ModifyXml("LAYOUT_BORDER_COLOR","BLUE",str(83))        # 0 - 255    
    connection.ModifyXml("VISUAL_EFFECTS","SPEAKER_NOTATION","true")        #true
    connection.ModifyXml("SPEAKER_NOTATION_COLOR","RED",str(83))        # 0 - 255
    connection.ModifyXml("SPEAKER_NOTATION_COLOR","GREEN",str(83))        # 0 - 255
    connection.ModifyXml("SPEAKER_NOTATION_COLOR","BLUE",str(83))        # 0 - 255
    connection.ModifyXml("RESERVATION","COP","false")        #false
    connection.ModifyXml("RESERVATION","PROFILE","false")        #false
    connection.ModifyXml("RESERVATION","AD_HOC","false")        #false
    connection.ModifyXml("RESERVATION","AD_HOC_PROFILE_ID",str(-1))        #-1
    connection.ModifyXml("RESERVATION","AUTO_LAYOUT","false")        #false
    connection.ModifyXml("RESERVATION","HSD_RATE","dynamically")        #dynamically
    connection.ModifyXml("RESERVATION","ENCRYPTION","false")        #false
    connection.ModifyXml("RESERVATION","CONF_CONTROL","h234")        #h243
    connection.ModifyXml("RESERVATION","CONF_PROTOCOL","h234")        #h243
    connection.ModifyXml("RESERVATION","EXTERNAL_MASTER","false")        #false
    connection.ModifyXml("RESERVATION","VIDEO","true")        #true
    connection.ModifyXml("RESERVATION","QCIF_FRAME_RATE","auto")        #auto
    connection.ModifyXml("RESERVATION","COUGH_DELAY",str(0))        #0
    connection.ModifyXml("RESERVATION","GUEST_OPER","false")        #false
    connection.ModifyXml("RESERVATION","WEB_RESERVED_UID",str(0))        #0
    connection.ModifyXml("RESERVATION","WEB_OWNER_UID",str(0))        #0
    connection.ModifyXml("RESERVATION","WEB_DB_ID",str(0))        #0
    connection.ModifyXml("RESERVATION","WEB_RESERVED","false")        #false
    connection.ModifyXml("RESERVATION","CONFERENCE_TYPE","standard")        #standard
    connection.ModifyXml("RESERVATION","COP_NUM_OF_PORTS","single")        #single
    connection.ModifyXml("RESERVATION","SILENCE_IT","false")        #false
    connection.ModifyXml("RESERVATION","IS_RECORDING_LINK","false")        #false
    connection.ModifyXml("RESERVATION","ENABLE_RECORDING","false")        #false
    connection.ModifyXml("RESERVATION","START_REC_POLICY","immediately")        #immediately
    connection.ModifyXml("RESERVATION","REC_LINK_ID",str(-1))        #-1
    connection.ModifyXml("RESERVATION","VIDEO_QUALITY","auto")        #auto
    connection.ModifyXml("RESERVATION","ENTRY_QUEUE_TYPE","normal")        #normal
    connection.ModifyXml("END_TIME_ALERT_TONE_EX","ON","false")        #false
    connection.ModifyXml("END_TIME_ALERT_TONE_EX","TIME",str(5))        #5
 
    connection.Send("")
    sleep(5)

    print "Wait untill Conf:" + confname + " will be createtd...",
    for retry in range(numRetries+1):
        connection.SendXmlFile('Scripts/TransConfList.xml',"Status OK")
        confid = connection.GetTextUnder("CONF_SUMMARY","ID")
        startTime = connection.GetTextUnder("CONF_SUMMARY","START_TIME")  
#        print startTime       
        
        if confid == "":
            print connection.xmlResponse.toprettyxml()
            connection.Disconnect()                
            sys.exit("Can not monitor conf")
            break
        

        if (retry == numRetries):
            print
            break
        sys.stdout.write(".")
        sys.stdout.flush()
        
    print "Create conf with id " + str(confid) 

#    video_ses = connection.GetTextUnder("CONF_SUMMARY","VIDEO_SESSION")
#    print video_ses
#    if(video_ses != 'continuous_presence'):
#        print 'test failed : ' + 'video_session'
    
   
    #print "Delete Conference..."
    connection.DeleteConf(confid)
         
    #print "Wait until no conferences"
    connection.WaitAllConfEnd(50)
    
    return


##--------------------------------------- TEST ---------------------------------

c = McmsConnection()
c.Connect()

AddConfAndParty(c,
                 'Scripts/AddVideoCpConf.xml',
                 20) #num of retries

c.Disconnect()
##------------------------------------------------------------------------------
    
    
    
    
