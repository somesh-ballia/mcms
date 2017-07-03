#!/mcms/python/bin/python

# For list of profiles look at RunTest.sh
#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_No_Valgrind
#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_4

# #############################################################################
#
# Re-Write date = 26/8/13
# Re-Write name = Uri A.
#
############################################################################

# script actions
# 1. create a conf and wait for its to be created
# 2. Add one H323 Party and one SIP party, wait for the parties to be connected.
# 3. monitor the parties
# 4. Disconnect H323 party and monitor it disconnected
# 5. Disconnect the SIP party and monitor it disconnected
# 6. Delete the conf.



from McmsConnection import *
# import Python classes

#------------------------------------------------------------------------------
def MonitorIPPartys(connection,num_retries):
    confName = "undefConf"
    connection.CreateConf(confName, "Scripts/ConfTamplates/AddConfTemplate.xml")
    confid = connection.WaitConfCreated(confName,num_retries)
    
    IsConfPartyUnderValgrind = False
    if (connection.IsProcessUnderValgrind("ConfParty")):
        IsConfPartyUnderValgrind = True
          
    #Add H323 party
    partyname1 = "Party1"
    connection.AddVideoParty(confid, partyname1, "1.2.3.1")
    #Add SIP Party
    partyname2 = "Party2"
    connection.AddVideoParty(confid, partyname2, "1.2.3.2",True)
    connection.WaitAllOngoingConnected(confid,num_retries)
    H323Partyid = connection.GetPartyId(confid, partyname1)
    SipPartyid = connection.GetPartyId(confid, partyname2)
    
    connection.WaitAllPartiesWereAdded(confid, 2,num_retries)
    if (IsConfPartyUnderValgrind):
    	sleep(2)

    print "Monitoring H323 Party "
    connection.MonitorParty(confid,H323Partyid)
    
    # when ConfParty under valgrind the disconnect is blocked because the party is still disconnected in DB
    # we will sleep for 2 seconds, to let the party start connecting 
    if (IsConfPartyUnderValgrind):
    	sleep(2)
    connection.DisconnectParty(confid,H323Partyid)
    connection.WaitPartyDisConnected(confid,H323Partyid,num_retries)

    print "Monitoring H323 Disconnected Party "
    connection.MonitorParty(confid,H323Partyid)
    # when ConfParty under valgrind the disconnect is blocked because the party is still disconnected in DB
    # we will sleep for 2 seconds, to let the party start connecting 
    if (IsConfPartyUnderValgrind):
    	sleep(1)
    
    print "Monitoring SIP Party "
    connection.MonitorParty(confid,SipPartyid)
    
    connection.DisconnectParty(confid,SipPartyid)
    connection.WaitAllOngoingDisConnected(confid, num_retries)    
    
    print "Monitoring SIP Disconnected Party "
    connection.MonitorParty(confid,SipPartyid)
    
    connection.DeleteConf(confid)
    connection.WaitAllConfEnd(num_retries)
    
    
#------------------------------------------------------------------------------
c = McmsConnection()
c.Connect()

MonitorIPPartys(c, 30)

c.Disconnect()


