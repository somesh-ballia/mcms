#!/mcms/python/bin/python

# For list of profiles look at RunTest.sh
#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_1



#############################################################################
# Test Script which Add Problem party to conf
#
# Date: 17/09/06
# By  : Ron S.

#############################################################################

from McmsConnection import *

###------------------------------------------------------------------------------
def AddConfAndParty(connection,confFile,partyFile,numRetries):
    
 
 
   
    confname = "Conf"
    connection.LoadXmlFile(confFile)
    connection.ModifyXml("RESERVATION","NAME",confname)
    print "Adding Conf:" + confname + "  ..."
    connection.Send()

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


    sleep(2)
    print "Create SimH323ErrorBitRate " 
    connection.SendXmlFile('Scripts/SimH323ErrorBitRate.xml',"")
    sleep(2)

    '''
    any secondery party should be added between the 2 SimH323ErrorBitRate
    '''


# Add H323 party to EP Sim and connect him
    partyname2 = "Party"+str(2)
    connection.SimulationAddH323Party(partyname2, confname)       
    connection.SimulationConnectH323Party(partyname2)
 
    connection.WaitAllPartiesWereAdded(confid,1,numRetries)   
    
    sleep(15)
    
    
    
#   connection.WaitAllOngoingConnected(confid,numRetries)      
    
    sleep(5)
    print "Create SimH323ErrorBitRate " 
    connection.LoadXmlFile('Scripts/SimH323ErrorBitRate.xml')
    connection.ModifyXml("H323_PARTY_BITRATE_ERROR","PARTY_H323_ERROR_BITRATE",0)
    connection.Send()
    sleep(10)
 
    sleep(15)
    # Add H323 party to EP Sim and connect him -this will change the status of party1 to connected
    partyname = "Party"+str(1)
    connection.SimulationAddH323Party(partyname, confname)
    connection.SimulationConnectH323Party(partyname)
    
    sleep(10)
     
 #   sleep(300)
    connection.DeleteConf(confid);
    connection.WaitAllConfEnd()
#    connection.Disconnect()
  
##--------------------------------------- TEST ---------------------------------

c = McmsConnection()
c.Connect()

AddConfAndParty(c,
                 'Scripts/AutoTerminateConf/AddConf.xml',
                 'Scripts/AutoTerminateConf/AddParty.xml',
                 20) #num of retries

c.Disconnect()
##------------------------------------------------------------------------------

