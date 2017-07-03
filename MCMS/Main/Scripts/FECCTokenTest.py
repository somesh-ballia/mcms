#!/mcms/python/bin/python

# For list of profiles look at RunTest.sh
#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_1

#############################################################################
# Test Script which is checking FECC token req/Release
#
# Date: 25/1/05
# By  : Udi B.
#############################################################################


from McmsConnection import *
import string

#------------------------------------------------------------------------------
def TestChangeFeccToken(connection,num_of_parties,num_retries):

    #Create conf with 3 paries
    confName = "FECCUndefConf"
    print "Adding Conf " + confName + " ..."
    connection.LoadXmlFile('Scripts/FECCTokenTest/AddCpConf.xml' )
    connection.ModifyXml("RESERVATION","NAME",confName)
    connection.Send()
    
    print "Wait untill Conf create...",
    for retry in range(num_retries+1):
        connection.SendXmlFile('Scripts/FECCTokenTest/TransConfList.xml',"Status OK")
        confid = connection.GetTextUnder("CONF_SUMMARY","ID")
        if confid != "":
            print
            break
        if (retry == num_retries):
            print connection.xmlResponse.toprettyxml()
            connection.Disconnect()                
            sys.exit("Can not monitor conf")
        sys.stdout.write(".")
        sys.stdout.flush()
    print "Created Conf " + str(confid)

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
    connection.WaitAllOngoingNotInIVR(confid)

    #Change Fecc
    ChangeFeccToken(connection,partiesIdToName,confid,num_retries)

    sleep(3)
    for key in partiesIdToName:
        connection.DeleteParty(confid,str(key))
        connection.SimulationDeleteH323Party(partiesIdToName[key])
         
    connection.WaitUntillPartyDeleted(confid,num_retries*num_of_parties)
    connection.DeleteConf(confid)
    connection.WaitAllConfEnd()

#------------------------------------------------------------------------------
def ChangeFeccToken(connection,partiesIdToName,confId,num_retries):
    print "testing Conf id:"+confId
    
    print "Insert Party Name: "
    for dictPartyId in partiesIdToName:
        partyName=partiesIdToName[dictPartyId]
        #send a specified party req
        print partyName+ " is requesting the token..."
        connection.LoadXmlFile('Scripts/FECCTokenTest/SimFeccTokenRequest.xml')
        connection.ModifyXml('FECC_TOKEN_REQUEST','PARTY_NAME',partyName)
        connection.Send()

        #mmonitor the FECC token
        connection.LoadXmlFile('Scripts/FECCTokenTest/TransConf2.xml')
        connection.ModifyXml('GET','ID',confId)
        for retry in range(num_retries):
            connection.Send()
            partyID = connection.GetTextUnder("CONFERENCE","LSD_SOURCE_ID")
            if ( int(partyID) == dictPartyId):
                print partyName + " is holding the FECC token..."
                #Realease the FECC token
                print "Realisng the FECC token from "+partyName
                connection.LoadXmlFile('Scripts/FECCTokenTest/SimFeccTokenRelease.xml')
                connection.ModifyXml('FECC_TOKEN_RELEASE','PARTY_NAME',partyName)
                connection.Send()
                break;
            if (retry == num_retries):
                connection.Disconnect()                
                sys.exit("Error: " + partyName+" did not get the FECC token !!!!")
            sleep(1)
        print
   
## ---------------------- Test --------------------------
c = McmsConnection()
c.Connect()

TestChangeFeccToken(c,
                   3, # num of parties
                   20)# retries

c.Disconnect()


