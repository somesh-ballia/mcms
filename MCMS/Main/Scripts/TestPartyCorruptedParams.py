#!/mcms/python/bin/python

# For list of profiles look at RunTest.sh
#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_8

#############################################################################
# Test Script which Create  Conf with 1  participant and 
#           Check the params of the participant.
# 
# Date: 06/02/06
# By  : Ron S.

#############################################################################

from McmsConnection import *

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
    connection.AddParty(confid, partyname, partyip, partyFile)
    partyid = connection.GetPartyId(confid, partyname)
# updating the participant's params 
#    connection.LoadXmlFile('Scripts/TestPartyCorruptedParams/UpdateParty.xml')
#    print "Updating Party:" + partyname + ", with ip= " + partyip
#    connection.ModifyXml("PARTY","NAME",partyname)
#    connection.ModifyXml("PARTY","IP",partyip)
#    connection.ModifyXml("UPDATE","ID",confid)    
#    connection.Send("")     
#    sleep(5)
    
    
    connection.ModifyXml("PARTY","INTERFACE","h323")        #h323
    connection.ModifyXml("PARTY","CONNECTION","dial_out")        #dial_out
    connection.ModifyXml("PARTY","MEET_ME_METHOD","party")        #party
    connection.ModifyXml("PARTY","NUM_TYPE","taken_from_service")        #taken_from_service
    connection.ModifyXml("PARTY","BONDING","auto")        #auto
    connection.ModifyXml("PARTY","MULTI_RATE","auto")        #auto
    connection.ModifyXml("PARTY","NET_CHANNEL_NUMBER","auto")        #auto
    connection.ModifyXml("PARTY","VIDEO_PROTOCOL","h267")        #auto
    connection.ModifyXml("PARTY","CALL_CONTENT","framed")        #framed
    connection.ModifyXml("ALIAS","ALIAS_TYPE","323_id")        #323_id
    connection.ModifyXml("PARTY","SIGNALING_PORT",str(-1720))        #1720
    connection.ModifyXml("PARTY","VOLUME",str(-8))        #5
    connection.ModifyXml("PARTY","AUTO_DETECT","false")        #false
    connection.ModifyXml("PARTY","RESTRICT","false")        #false
    connection.ModifyXml("PARTY","ENHANCED_VIDEO","false")        #false
    connection.ModifyXml("PARTY","VIDEO_BIT_RATE","automatic")        #automatic
    connection.ModifyXml("IP_QOS","QOS_ACTION","from_service")        #from_service
    connection.ModifyXml("IP_QOS","QOS_DIFF_SERV","precedence")        #precedence
    connection.ModifyXml("IP_QOS","QOS_IP_AUDIO",str(84))        #5
    connection.ModifyXml("IP_QOS","QOS_IP_VIDEO",str(-994))        #4
    connection.ModifyXml("IP_QOS","QOS_TOS","delay")        #delay
    connection.ModifyXml("PARTY","RECORDING_PORT",semi_long_char)        #no
    connection.ModifyXml("PARTY","LAYOUT_TYPE","no")        #conference
    connection.ModifyXml("PARTY","PERSONAL_LAYOUT","1x1")        #1x1
    connection.ModifyXml("PARTY","VIP","false")        #false
    connection.ModifyXml("PARTY","LISTEN_VOLUME",str(-7))        #5
    connection.ModifyXml("PARTY","AGC","true")        #true
    connection.ModifyXml("PARTY","SIP_ADDRESS_TYPE","uri_type")        #uri_type
    connection.ModifyXml("PARTY","WEB_USER_ID","0")        #0
    connection.ModifyXml("PARTY","UNDEFINED","false")        #false
    connection.ModifyXml("PARTY","DEFAULT_TEMPLATE","false")        #false
    connection.ModifyXml("PARTY","NODE_TYPE","terminal")        #terminal
    connection.ModifyXml("PARTY","ENCRYPTION_EX","auto")        #auto
    connection.ModifyXml("PARTY","H323_PSTN","false")        #false
    connection.ModifyXml("PARTY","IS_RECORDING_LINK_PARTY","false")        #false
    connection.ModifyXml("PARTY","IDENTIFICATION_METHOD","called_phone_num")        #called_phone_number
    
    connection.Send("")     
#    connection.Send()

#    qos_IP_audio = connection.GetTextUnder("IP_QOS","QOS_IP_AUDIO")
#    print qos_IP_audio
#    if(qos_IP_audio != 5):        
#        print 'test failed'    


    connection.WaitAllPartiesWereAdded(confid,1,numRetries)
    
    
    connection.LoadXmlFile('Scripts/TransConf2.xml')
    connection.ModifyXml("GET","ID",confid)
    connection.Send()
    partyid = connection.GetTextUnder("PARTY","ID")
    connection.WaitAllOngoingConnected(confid,numRetries)  
    
    sleep(5)
    
    party_id_list = []
    connection.LoadXmlFile('Scripts/TransConf2.xml')
    connection.ModifyXml("GET","ID",confid)
    connection.Send()    
    ongoing_party_list = connection.xmlResponse.getElementsByTagName("ONGOING_PARTY_LIST")
    if len(ongoing_party_list) != 1:
        sys.exit("some parties are lost...")
#    partyid = ongoing_party_list[0].getElementsByTagName("ID")[0].firstChild.data
#    print partyid
#    volume = ongoing_party_list[0].getElementsByTagName("VOLUME")[0].firstChild.data
#    print volume
#    listen_volume = ongoing_party_list[0].getElementsByTagName("LISTEN_VOLUME")[0].firstChild.data
#    print listen_volume
#    layout_type = ongoing_party_list[0].getElementsByTagName("LAYOUT_TYPE")[0].firstChild.data
#    print layout_type
    
#    if(volume != str(5)):
#        print 'test failed : ' + 'VOLUME'
#    if(listen_volume != str(5)):
#        print 'test failed : ' + 'LISTEN_VOLUME'
#    if(layout_type != 'conference'):
#        print 'test failed : ' + 'LAYOUT_TYPE'

        
    sleep(5)
    
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
                 'Scripts/AddVideoParty.xml',

                 20) #num of retries

c.Disconnect()
##------------------------------------------------------------------------------
    
    
