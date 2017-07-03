#!/mcms/python/bin/python


# For list of profiles look at RunTest.sh

#*PROC-disabled-ESSES_PROFILE_FOR_SCRIPT=Profile_No_Valgrind

#*export MPL_SIM_FILE="VersionCfg/MPL_SIM_BREEZE.XML" 
#*export SYSTEM_CARDS_MODE_FILE="VersionCfg/ SystemCardsMode_breeze.txt" 

#############################################################################
# Test Script which is checking the changing of the active speaker
#
# Date: 21/1/05
# By  : Udi B.
#############################################################################


from McmsConnection import *
import string 

#------------------------------------------------------------------------------
def TestChangeSpeaker(connection,num_of_parties,num_retries):
    #Create conf with 3 paries
    confName = "undefConf"
    connection.CreateConf(confName, 'Scripts/SpeakerChange/AddCpConf.xml')
    
    confid = connection.WaitConfCreated(confName,num_retries)

    ### Add parties to EP Sim and connect them
    partiesIdToName=dict()
    for x in range(num_of_parties):
        partyname = "Party"+str(x+1)
        connection.SimulationAddH323Party(partyname, confName)
        connection.SimulationConnectH323Party(partyname)
        #Get the party id
        currPartyID = connection.GetCurrPartyID(connection,confid,x,num_retries)
        if (currPartyID < 0):
            connection.Disconnect()                
            sys.exit("Error:Can not find partry id of party: "+partyname)
        print "found party id ="+str(currPartyID)
        partiesIdToName[currPartyID] = partyname
    
    connection.WaitAllOngoingConnected(confid,num_retries*num_of_parties)

    #Change Speakers
    ChangeAudioSpeakers(connection,partiesIdToName,confid,num_retries*5)

    for key in partiesIdToName:
        #print "Party id is:"+str(key) + " , and party name is"+partiesIdToName[key]
        connection.DeleteParty(confid,str(key))

    connection.WaitUntillPartyDeleted(confid,num_retries*num_of_parties)
    connection.DeleteConf(confid)
    connection.WaitAllConfEnd()
    return

#------------------------------------------------------------------------------
def ChangeAudioSpeakers(connection,partiesIdToName,confid,num_retries):
    
    for partyId in partiesIdToName:
        #changing the speaker
        connection.LoadXmlFile('Scripts/SpeakerChange/SimAudioSpeaker.xml')
        connection.ModifyXml("AUDIO_SPEAKER","PARTY_NAME", partiesIdToName[partyId])
        connection.Send()
        
        #make sure the party was changed
        connection.LoadXmlFile('Scripts/SpeakerChange/TransConf2.xml')
        connection.ModifyXml('GET','ID',confid)
    
        print "Checking that audio active Speaker is party: "+ partiesIdToName[partyId]+ ",partyId="+ str(partyId),
        for retry in range(num_retries+1):
            connection.Send()
            audioSourceIdList = connection.xmlResponse.getElementsByTagName('AUDIO_SOURCE_ID')
            activePartyId= audioSourceIdList[0].firstChild.data
            if (activePartyId != ""  and (string.atoi(activePartyId) == partyId) ) :
                print
                print partiesIdToName[partyId]+ " with id: "+str(partyId)+ " is the current audio active speaker"
                break
            if (retry == num_retries):
                print
                connection.Disconnect()                
                sys.exit("Setting Party:"+ partiesIdToName[partyId]+" as audio active speaker Failed!!!")
            sys.stdout.write(".")
            sys.stdout.flush()
            sleep(1)

#------------------------------------------------------------------------------
def DelSimParty(partyName):
    print "Deleting SIM party "+partyName+"..."
    c.LoadXmlFile("Scripts/SpeakerChange/SimDel323Party.xml")
    c.ModifyXml("H323_PARTY_DEL","PARTY_NAME",partyName)
    c.Send()
    return


#------------------------------------------------------------------------------

## ---------------------- Test --------------------------
c = McmsConnection()
c.Connect()

TestChangeSpeaker(c,
                   3, # num of parties
                   20)# retries

c.Disconnect()


#------------------------------------------------------------------------------
