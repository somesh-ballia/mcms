#!/mcms/python/bin/python

# For list of profiles look at RunTest.sh
#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_1
#*export MPL_SIM_FILE="VersionCfg/MPL_SIM_BARAK_4.XML"
#*export SYSTEM_CARDS_MODE_FILE="VersionCfg/SystemCardsMode_mpmPlus.txt"


#############################################################################
# Test Script For VSW High Profile based on VSW-H261.py script 
# Date: 4 April 2011
# By  : Marina
#############################################################################
from McmsConnection import *
from HDFunctions import *
import string 
#------------------------------------------------------------------------------

def TestHD(connection,num_retries):
    
    print "Adding VSW H261 Profile"
    prof_id = AddHdProfile(connection,"VSW_H261_profile_384","384", "Scripts/HD/XML/AddVSWH261Profile.xml")

    print "Adding Conf..."
    confName = "VSW_H261_conf_384"
    connection.CreateConfFromProfile(confName, prof_id)
    print "Wait untill Conf create...",
    conf_id = connection.WaitConfCreated(confName,num_retries)
    
    #connect parties
    print "Start connecting Parties..."
    #num_of_parties = 4
    confLayoutType = "1x1" 
    
    #------Dial out H323 party    
    partyname = confName+"_Party_1"
    partyip =  "1.2.3.1" 
    connection.AddVideoParty(conf_id, partyname, partyip)
    print "Connecting H323 Dial out Party" 
    connection.WaitAllOngoingConnected(conf_id)
    connection.WaitAllOngoingNotInIVR(conf_id)
    #------Dial out SIP party   
    partyname = confName+"_Party_2"
    partyip =  "1.2.3.2" 
    connection.AddVideoParty(conf_id, partyname, partyip, "true")
    print "Connecting SIP Dial out Party" 
    connection.WaitAllOngoingConnected(conf_id)
    connection.WaitAllOngoingNotInIVR(conf_id)

    #-------Dial in H323 party
    partyname = confName+"_Party_3"
    connection.SimulationAddH323Party(partyname, confName)
    connection.SimulationConnectH323Party(partyname)
    print "Connecting H323 Dial in Party -"
    connection.WaitAllOngoingConnected(conf_id)
    connection.WaitAllOngoingNotInIVR(conf_id)

    #-------Dial in SIP party
    #partyname = confName+"_Party_4"
    #connection.SimulationAddSipParty(partyname, confName)
    #connection.SimulationConnectSipParty(partyname)
    #print "Connecting SIP Dial in Party"
    #connection.WaitAllOngoingConnected(conf_id)
    #connection.WaitAllOngoingNotInIVR(conf_id)
    
    num_of_parties = 4    
    for x in range(num_of_parties):
        connection.ChangeDialOutAudioSpeaker(conf_id,x)
        


## ---------------------- Test --------------------------
c = McmsConnection()
c.Connect()

TestHD(c,20)# retries
c.Disconnect()

#------------------------------------------------------------------------------  
    
