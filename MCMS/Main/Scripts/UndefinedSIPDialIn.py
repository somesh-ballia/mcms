#!/mcms/python/bin/python

#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_No_Valgrind

import os
from McmsConnection import *

#------------------------------------------------------------------------------
def TestUndefinedDailIn(connection,num_of_parties,num_retries):
    confName = "undefConf"
    print "Adding Conf " + confName + " ..."
    connection.LoadXmlFile('Scripts/UndefinedDialIn/AddCpConf.xml' )
    connection.ModifyXml("RESERVATION","NAME",confName)
    connection.Send()
    
    print "Wait untill Conf create...",
    for retry in range(num_retries+1):
        connection.SendXmlFile('Scripts/UndefinedSipDialIn/TransConfList.xml',"Status OK")
        confid = connection.GetTextUnder("CONF_SUMMARY","ID")
        if confid != "":
            print
            break
        if (retry == num_retries):
            print connection.xmlResponse.toprettyxml()
            connection.Disconnect()                
            sys.exit("Can not monitor conf:" + status)
        sys.stdout.write(".")
        sys.stdout.flush()
    print "Created Conf " + str(confid)

    ### Add parties to EP Sim and connect them
    for x in range(num_of_parties):
        partyname = "Party"+str(x+1)
        connection.SimulationAddSipParty(partyname, confName)
        connection.SimulationConnectSipParty(partyname)
        
    sleep(3)    
    connection.WaitAllOngoingConnected(confid,num_retries)
    if(connection.IsProcessUnderValgrind("ConfParty")):
        print "  ConfParty under valgrind sleep another 9 seconds "
        sleep(9)

    # Check if all parties were added and save their IDs
    party_id_list = [0]*num_of_parties 
    connection.LoadXmlFile('Scripts/UndefinedSipDialIn/TransConf2.xml')
    connection.ModifyXml("GET","ID",confid)
    connection.Send()
    ongoing_party_list = c.xmlResponse.getElementsByTagName("ONGOING_PARTY")
    if len(ongoing_party_list) < num_of_parties:
        errMsg= "some parties are lost, find only " +str(len(ongoing_party_list)) + " parties in conf"
        sys.exit(errMsg )
    for index in range(num_of_parties):
        party_id_list[(num_of_parties - index) - 1 ]  = ongoing_party_list[index].getElementsByTagName("ID")[0].firstChild.data

    for index in range(num_of_parties):
        partyname = "Party"+str(index+1)
        #connection.SimulationDisconnectSipParty(partyname)
        print "Deleting " + partyname
        connection.SimulationDeleteSipParty(partyname)
        print "sleep 10 seconds to let party go down"
        sleep(10)
     
    connection.DeleteConf(confid)
    connection.WaitAllConfEnd()
    return

#------------------------------------------------------------------------------


## ---------------------- Test --------------------------



c = McmsConnection()
c.Connect()

TestUndefinedDailIn(c,
                    3, # num of parties
                    20)# retries

c.Disconnect()


