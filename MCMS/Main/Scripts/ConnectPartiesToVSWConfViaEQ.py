#!/mcms/python/bin/python

# For list of profiles look at RunTest.sh
#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_1

#*export MPL_SIM_FILE="VersionCfg/MPL_SIM_BREEZE.XML" 
#*export SYSTEM_CARDS_MODE_FILE="VersionCfg/SystemCardsMode_breeze.txt" 


#############################################################################
# Test Script For moving party from EQ VSW to Ongoing VSW conference
# Date: 07/12/09
# By  : Keren
#############################################################################

from HDFunctions import *

#------------------------------------------------------------------------------
def ConnectXSimulationParties(connection,eqName,num_parties,num_retries):
    # Create a new undefined party and add it to the EPSim
    for x in range(num_parties):
        partyName = "Party" + str(x+1)
        connection.SimulationAddH323Party(partyName, eqName)
        #connect the undefined party
        connection.SimulationConnectH323Party(partyName)
        
    sleep(1)
    return 
#------------------------------------------------------------------------------
def DtmfXSimulationParties(connection,adHocConfNumericId,num_parties):
     
    #Send the DTMF which represent the new Ad-hoc conf
    print "Sending DTMF: " + adHocConfNumericId + "to Conf"
    for x in range(num_parties):
        partyName = "Party" + str(x+1)
        sleep(2)
        connection.SimulationH323PartyDTMF(partyName, adHocConfNumericId)
    return
#------------------------------------------------------------------------------
def TestConnectPartiesToVswConfViaEQ(connection,num_retries):
    
    print "Adding HD Profile 1080"
    prof_id_hd1080 = AddHdProfile(connection,"HD1080_profile_1920","1920", "Scripts/HD/XML/AddHdProfile.xml")
    
    #Add a new EQ service
    print "Adding a new Entry Queue Service...."
    connection.SendXmlFile("Scripts/CreateAdHoc3BlastUndefParties/AddEqService.xml","Status OK")

    # send a new Reservation of EntryQueue
    eqName = "eqRsrv"
    connection.CreateAdHocEQ(eqName, prof_id_hd1080, "Scripts/CreateAdHoc3BlastUndefParties/CreateNewEq.xml",'false')

    #Wait untill eq was added
    eqId, eqNID = connection.WaitMRCreated(eqName,num_retries)
    print "eqNID = " + eqNID
    
    #create VSW conference 
    confName = "VSW_HD1080_CONF"
    print "Adding Conf " + confName + " ..."
    connection.LoadXmlFile('Scripts/AddCpConf.xml')
    connection.ModifyXml("RESERVATION","NAME",confName)
    connection.ModifyXml("RESERVATION","AD_HOC_PROFILE_ID",prof_id_hd1080) 
    #connection.ModifyXml("RESERVATION","MAX_PARTIES",str(28))
    connection.Send()
    confId = connection.WaitConfCreated(confName,num_retries)
    confNumericId = connection.GetConfNumericId(confId)
    print "confNumericId = " + confNumericId
    
    num_parties = 3
    ConnectXSimulationParties(connection,eqName,num_parties,num_retries)
   
    eqConfId = connection.WaitUntillEQorMRAwakes(eqName,num_parties,num_retries,True)
       
    DtmfXSimulationParties(connection,confNumericId,num_parties)
    sleep(2)
    
    #delete the conf
    connection.DeleteConf(confId)
    
    #delete the eq conf
    connection.DeleteConf(eqConfId)

    connection.WaitAllConfEnd(num_retries)
  
 
## ---------------------- Test --------------------------
c = McmsConnection()
c.Connect()

TestConnectPartiesToVswConfViaEQ(c,
                    30)# retries

c.Disconnect()

