#!/mcms/python/bin/python

# For list of profiles look at RunTest.sh
#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_8

#############################################################################
# Test Script which Add profile and checks the profile parameters : 
#                 TestProfileCorruptedParams.py
#
# Date: 1/02/06
# By  : Ron S.

#############################################################################

from McmsConnection import *

###------------------------------------------------------------------------------
def CheckProfileParamsTemp(connection,AddProfile,                    
                       numRetries):


    long_char = "sababasababasababasababasababasababasababasababasababasababasababasababasababasababasababasababasababasababasababasababasababasababasababasababasababasababasababasababasababasababasababasababasababasababasababasababasababasababasababasababasaba"
    short_char = "sababa"

    print "Starting test CheckProfileParams ... "
    
#add a new profile
    print "Adding new Profile..."
#    connection.SendXmlFile(AddProfile, "Status OK")
#    profId = connection.GetTextUnder("RESERVATION","ID")
#    my_name = connection.GetTextUnder("RESERVATION","NAME")
#    print "Profile, named: " + my_name + " ,ID = " + profId + ", is added"
    
#update the profile General params   
#    print "Updating profile: " + my_name + " General params"
#    connection.LoadXmlFile(UpdateProfile)
    connection.LoadXmlFile(AddProfile)
#    connection.ModifyXml("RESERVATION","ID",profId)
#    connection.ModifyXml("RESERVATION","NAME",my_name)
    connection.ModifyXml("RESERVATION","MAX_PARTIES",str(10))                 # 0 - 860
    connection.ModifyXml("MEET_ME_PER_CONF","MIN_NUM_OF_PARTIES",str(12))     # 0 - 30
    connection.ModifyXml("AUTO_TERMINATE","ON","true")
    connection.ModifyXml("AUTO_TERMINATE","TIME_BEFORE_FIRST_JOIN",str(10))    # 1 - 60
    connection.ModifyXml("AUTO_TERMINATE","TIME_AFTER_LAST_QUIT",str(-122))      # 1 - 60
    connection.ModifyXml("RESERVATION","TRANSFER_RATE",str(381))               #   384 - 1920
    connection.ModifyXml("RESERVATION","ENCRYPTION","true")
    
    connection.ModifyXml("RESERVATION","AUTO_LAYOUT","true")
    connection.ModifyXml("RESERVATION","SAME_LAYOUT","true")    
    
    connection.ModifyXml("BACKGROUND_COLOR","RED",str(233))     # 0 - 255
    connection.ModifyXml("BACKGROUND_COLOR","GREEN",str(200))   # 0 - 255 
    connection.ModifyXml("BACKGROUND_COLOR","BLUE",str(200))    # 0 - 255
    connection.ModifyXml("RESERVATION","LAYOUT_BORDER","true")
    connection.ModifyXml("RESERVATION","SPEAKER_NOTATION","true")
    
    connection.ModifyXml("VISUAL_EFFECTS","LAYOUT_BORDER","true")        #true
    connection.ModifyXml("VISUAL_EFFECTS","SPEAKER_NOTATION","true")        #true
    connection.ModifyXml("SPEAKER_NOTATION_COLOR","RED",str(183))        # 0 - 255
    connection.ModifyXml("SPEAKER_NOTATION_COLOR","GREEN",str(183))        # 0 - 255
    connection.ModifyXml("SPEAKER_NOTATION_COLOR","BLUE",str(183))        # 0 - 255
    connection.ModifyXml("RESERVATION","NETWORK","h323")           #h323
    connection.ModifyXml("RESERVATION","MEDIA","video_audio")             #video_audio
    connection.ModifyXml("RESERVATION","VIDEO_SESSION","continuous_presence")     #continuous_presence
    connection.ModifyXml("RESERVATION","VIDEO_PROTOCOL","auto")        #auto    

    connection.ModifyXml("RESERVATION","AUDIO_RATE","auto")        #auto
    connection.ModifyXml("RESERVATION","VIDEO_FORMAT","auto")        #auto
    connection.ModifyXml("RESERVATION","FRAME_RATE","auto")        #auto
    connection.ModifyXml("RESERVATION","ATTENDED_MODE","ivr")        #ivr
    connection.ModifyXml("RESERVATION","RESTRICT_MODE","derestricted")        #derestricted
    connection.ModifyXml("RESERVATION","T120_RATE","none")        #none
    connection.ModifyXml("RESERVATION","ENTRY_TONE","false")        #false
    connection.ModifyXml("RESERVATION","EXIT_TONE","false")        #false
    connection.ModifyXml("RESERVATION","END_TIME_ALERT_TONE",str(5))        #5
    connection.ModifyXml("RESERVATION","STAND_BY","false")        #false
    connection.ModifyXml("RESERVATION","PEOPLE_AND_CONTENT","false")        #false
    connection.ModifyXml("RESERVATION","ANNEX_N","false")        #false
    connection.ModifyXml("RESERVATION","ANNEX_P","false")        #false
    connection.ModifyXml("RESERVATION","ANNEX_F","false")        #false 
    connection.ModifyXml("RESERVATION","OPERATOR_CONF","false")        #false
    connection.ModifyXml("RESERVATION","SAME_LAYOUT","false")        #false
    connection.ModifyXml("RESERVATION","DUO_VIDEO","false")        #false
    connection.ModifyXml("AUTO_TERMINATE","ON","true")        #true    
    
    connection.ModifyXml("RESERVATION","AUDIO_MIX_DEPTH",str(5))        #5
    connection.ModifyXml("RESERVATION","TALK_HOLD_TIME",str(150))        #150
    connection.ModifyXml("MEET_ME_PER_CONF","ON","true")        #true
    connection.ModifyXml("MEET_ME_PER_CONF","AUTO_ADD","true")        #true
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
    
    connection.ModifyXml("DURATION","HOUR",str(1))        #0 - 99
    connection.ModifyXml("DURATION","MINUTE",str(20))        #0
    connection.ModifyXml("DURATION","SECOND",str(10))        #0
    connection.ModifyXml("RESERVATION","LOCK","false")        #false
    connection.ModifyXml("RESERVATION","LSD_RATE","none")        #none
    connection.ModifyXml("CASCADE","CASCADE_ROLE","none")        #none
    connection.ModifyXml("LECTURE_MODE","ON","false")        #false
    connection.ModifyXml("LECTURE_MODE","TIMER","false")        #false
    connection.ModifyXml("LECTURE_MODE","INTERVAL",str(1554))        #15
    connection.ModifyXml("LECTURE_MODE","AUDIO_ACTIVATED","false")        #false
#    connection.ModifyXml("LECTURE_MODE","LECTURE_NAME",long_char)        #[None]
    connection.ModifyXml("LECTURE_MODE","LECTURE_MODE_TYPE","lecture_none")        #lecture_none
    
    
    connection.ModifyXml("LECTURE_MODE","LECTURE_ID",str(0))        #0
    connection.ModifyXml("RESERVATION","LAYOUT","1x1")        #1x1
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
    connection.ModifyXml("BACKGROUND_COLOR","RED",str(86))       #86
    connection.ModifyXml("BACKGROUND_COLOR","GREEN",str(86))        #86 
    connection.ModifyXml("BACKGROUND_COLOR","BLUE",str(83))        #83
    connection.ModifyXml("VISUAL_EFFECTS","LAYOUT_BORDER","true")        #true
    connection.ModifyXml("VISUAL_EFFECTS","SPEAKER_NOTATION","true")        #true
    connection.ModifyXml("SPEAKER_NOTATION_COLOR","RED",str(83))        #
    connection.ModifyXml("SPEAKER_NOTATION_COLOR","GREEN",str(83))        #83
    connection.ModifyXml("SPEAKER_NOTATION_COLOR","BLUE",str(83))        #
    connection.ModifyXml("RESERVATION","COP","false")        #false
    connection.ModifyXml("RESERVATION","AD_HOC","false")        #false
    connection.ModifyXml("RESERVATION","AD_HOC_PROFILE_ID",str(-1))        #-1
    connection.ModifyXml("RESERVATION","AUTO_LAYOUT","false")        #false
      
    connection.ModifyXml("RESERVATION","HSD_RATE","dynamically")        #dynamically
    connection.ModifyXml("RESERVATION","ENCRYPTION","false")        #false
    connection.ModifyXml("RESERVATION","CONF_CONTROL","h243")        #h243
    connection.ModifyXml("RESERVATION","CONF_PROTOCOL","h243")        #h243
    connection.ModifyXml("RESERVATION","EXTERNAL_MASTER","true")        #true
    connection.ModifyXml("RESERVATION","VIDEO","false")        #false
    connection.ModifyXml("RESERVATION","QCIF_FRAME_RATE","auto")        #auto
    connection.ModifyXml("RESERVATION","COUGH_DELAY",str(50))        #0
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
    connection.ModifyXml("RESERVATION","VTX","false")        #false
    connection.ModifyXml("RESERVATION","CASCADE_EQ","false")        #false
    connection.ModifyXml("RESERVATION","SIP_FACTORY","false")        #false
    connection.ModifyXml("RESERVATION","SIP_FACTORY_AUTO_CONNECT","false")        #false
    
    connection.Send("")
    
    sleep(5)
    
#    print connection.GetTextUnder("PROFILE_SUMMARY","ID")
#    print profid
#    if connection.GetTextUnder("PROFILE_SUMMARY","ID") == "":
#        connection.Disconnect()
#        sys.exit("Failed add profile!!!")

#    connection.SendXmlFile('Scripts/TestProfileCorruptedParams/ProfileSummaryList.xml',"Status OK") 
#    prof_list = connection.xmlResponse.getElementsByTagName("PROFILE_SUMMARY")  
#    print len(prof_list)
#    profID = prof_list[0].getElementsByTagName("ID")[0].firstChild.data
#    print profID


#    sleep(10)
#    print "here1"
    profId = connection.GetTextUnder("RESERVATION","ID")
    my_name = connection.GetTextUnder("RESERVATION","NAME")
    print "Profile, named: " + my_name + " ,ID = " + profId + ", is added"
    
    connection.SendXmlFile('Scripts/TestProfileCorruptedParams/ProfileSummaryList.xml',"Status OK")
#    profid = connection.GetTextUnder("PROFILE_SUMMARY","ID")
#    print profid
    if connection.GetTextUnder("PROFILE_SUMMARY","ID") == "":
        connection.Disconnect()
        sys.exit("Failed add profile!!!")
    
#    TRANSFER_RATE = connection.GetTextUnder("PROFILE_SUMMARY","TRANSFER_RATE")
#    print TRANSFER_RATE
#    if(TRANSFER_RATE != str(64)):
#        print 'test failed : ' + 'TRANSFER_RATE'
    
#Remove the profile
    connection.DelProfile(profId, "Scripts/TestProfileCorruptedParams/RemoveNewProfile.xml")


##--------------------------------------- TEST ---------------------------------

c = McmsConnection()
c.Connect()

CheckProfileParamsTemp(c,                     
                  'Scripts/TestProfileCorruptedParams/CreateNewProfile.xml',              
                   20) #num of retries

c.Disconnect()
##------------------------------------------------------------------------------

