#!/mcms/python/bin/python
#-LONG_SCRIPT_TYPE
# For list of profiles look at RunTest.sh
#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_1
#*PROCESSES_NOT_FOR_VALGRIND=Authentication Resource Faults CertMngr GideonSim EndpointsSim Logger QAAPI MplApi CSApi EncryptionKeyServer

#*export MPL_SIM_FILE="VersionCfg/MPL_SIM_BREEZE.XML" 
#*export SYSTEM_CARDS_MODE_FILE="VersionCfg/ SystemCardsMode_breeze.txt" 


#############################################################################
# Test Script For VSW HD1080 based on HD.py script 
# Date: 06/07/08
# By  : Keren
#############################################################################
from McmsConnection import *
from HDFunctions import *
import string 
#------------------------------------------------------------------------------

def TestHD(connection,num_retries):
    
    print "Adding HD Profile"
    prof_id = AddHdProfile(connection,"HD1080_profile_6144","6144", "Scripts/HD/XML/AddVswHd1080Profile.xml")

    print "Adding Conf..."
    confName = "VSW_HD1080_conf_6144"
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
    #------Dial out another H323 party   
    partyname = confName+"_Party_2"
    partyip =  "1.2.3.2" 
    connection.AddVideoParty(conf_id, partyname, partyip)
    print "Connecting another H323 Dial out Party" 
    connection.WaitAllOngoingConnected(conf_id)
    connection.WaitAllOngoingNotInIVR(conf_id)

    #-------Dial in H323 party
#    partyname = confName+"_Party_3"
#    connection.SimulationAddH323Party(partyname, confName)
#    connection.SimulationConnectH323Party(partyname)
#    print "Connecting H323 Dial in Party -"
#    connection.WaitAllOngoingConnected(conf_id)
#    connection.WaitAllOngoingNotInIVR(conf_id)

    #-------Dial in SIP party
    partyname = confName+"_Party_4"
    connection.SimulationAddSipParty(partyname, confName)
    connection.SimulationConnectSipParty(partyname)
    print "Connecting SIP Dial in Party"
    connection.WaitAllOngoingConnected(conf_id)
    connection.WaitAllOngoingNotInIVR(conf_id)
    
#    num_of_parties = 4
    num_of_parties = 3        
    for x in range(num_of_parties):
        connection.ChangeDialOutAudioSpeaker(conf_id,x)
        
    sleep(3)
    # delete all parties
    partyname = confName+"_Party_1"  
    connection.SimulationDeleteH323Party(partyname)
    sleep(4)
    partyname = confName+"_Party_2"  
    connection.SimulationDeleteH323Party(partyname)
    sleep(4)
#    partyname = confName+"_Party_3"  
#    connection.SimulationDeleteSipParty(partyname)
#    sleep(4)
    partyname = confName+"_Party_4"  
    connection.SimulationDeleteSipParty(partyname)
    sleep(4)
    connection.DeleteConf(conf_id)
    sleep(4)
    connection.DelProfile(prof_id)

## ---------------------- Test --------------------------
c = McmsConnection()
c.Connect()

TestHD(c,20)# retries
c.Disconnect()

#------------------------------------------------------------------------------  
    
