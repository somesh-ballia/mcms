#!/mcms/python/bin/python

# For list of profiles look at RunTest.sh
#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_8

#############################################################################
# Test Script which Create  Conf with 1  participant and 
#  then delete the participant and check if the conf is 
#  auto terminate
# 
# Date: 06/03/06
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


     
# adding the first participant 
    connection.LoadXmlFile(partyFile)
    partyname = "Party1" 
    partyip =  "1.2.3.1"
    print "Adding Party:" + partyname + ", with ip= " + partyip
    connection.ModifyXml("PARTY","NAME",partyname)
    connection.ModifyXml("PARTY","IP",partyip)
    connection.ModifyXml("ADD_PARTY","ID",confid)
    connection.Send() 
    
    connection.WaitAllPartiesWereAdded(confid,1,numRetries)
    connection.WaitAllOngoingConnected(confid,numRetries)  
    
    sleep(5)
 
    party_id_list = []
    connection.LoadXmlFile('Scripts/TransConf2.xml')
    connection.ModifyXml("GET","ID",confid)
    connection.Send()    
    ongoing_party_list = connection.xmlResponse.getElementsByTagName("ONGOING_PARTY")
    if len(ongoing_party_list) != 1:
        sys.exit("some parties are lost...")
    for index in range(1):    
        party_id_list.append(ongoing_party_list[index].getElementsByTagName("ID")[0].firstChild.data)
           
    print "delete party: " + partyname    
    connection.LoadXmlFile('Scripts/AutoTerminateConf/DeleteParty.xml')
    connection.ModifyXml("DELETE_PARTY","ID",confid)
    connection.ModifyXml("DELETE_PARTY","PARTY_ID",party_id_list[0])
    connection.Send()

    sleep(65)
    numRetries = 60
    
    for retry in range(numRetries+1):
         sys.stdout.write(".")
         sys.stdout.flush()
         connection.SendXmlFile('Scripts/TransConfList.xml',"Status OK")
         if connection.GetTextUnder("CONF_SUMMARY","ID") == "":
             print 'conf : ' + confname + ' auto terminated' 
             break
         if(retry == numRetries):
             connection.Disconnect()
             sys.exit("Failed to auto terminated!!!")
         sleep(1)                
    
##--------------------------------------- TEST ---------------------------------

c = McmsConnection()
c.Connect()

AddConfAndParty(c,
                 'Scripts/AutoTerminateConf/AddConf.xml',
                 'Scripts/AutoTerminateConf/AddParty.xml',
                 20) #num of retries

c.Disconnect()
##------------------------------------------------------------------------------

 
