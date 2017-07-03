#!/mcms/python/bin/python

#############################################################################
# This Script do:
#  1. Add FACTORY
#  2. Add SIP participant
#  3. Remove SIP  participant from the ongoing list
#  4. Remove SIP  participant from EP SIM
#  5. Remove the ongoing conf
#  6. Remove the Factory
#
# Date: 23/01/06
# By  : Ron S.
# Re-Write date = 03/10/13
# Re-Write name = Uri A.
#############################################################################

#-LONG_SCRIPT_TYPE
# For list of profiles look at RunTest.sh
#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_4

from McmsConnection import *

###------------------------------------------------------------------------------
def FactoryAddRemoveSipPartyTest(connection,AddProfile,AddFactoryFile,                         
                         RemoveFactoryFile,
                         numRetries):

    print "Starting test FactoryAddRemoveSipPartyTest ... "
    
 #add a new profile
    profId = connection.AddProfile("profile1", AddProfile)

    #Adding a new Factory
    factoryName = "fac1"
    print "Adding a new Factory:" + factoryName + " ..."
    connection.LoadXmlFile(AddFactoryFile)
    connection.ModifyXml("RESERVATION","NAME",factoryName)
    connection.ModifyXml("RESERVATION","AD_HOC_PROFILE_ID",str(profId))
    connection.Send()

  #make sure that the Factory was added
    FacId = ""
    print "Wait untill Factory: " + factoryName + " will be createtd..."
    for retry in range(numRetries+1):
        connection.SendXmlFile('Scripts/AddDeleteFactoryWithSipParty/TransMrList.xml',"Status OK") 
        mr_list = connection.xmlResponse.getElementsByTagName("MEETING_ROOM_SUMMARY")
        if len(mr_list) > 0 : 
            for index in range(len(mr_list)):
                if factoryName == mr_list[index].getElementsByTagName("NAME")[0].firstChild.data:
                    FacId = mr_list[index].getElementsByTagName("ID")[0].firstChild.data        
            print
            break
        if (retry == numRetries):
            print connection.xmlResponse.toprettyxml()
            connection.Disconnect()                
            sys.exit("Can not monitor conf:")
        sys.stdout.write(".")
        sys.stdout.flush()
    print "Factory Id is: "+ FacId
    

# Add SIP party to EP Sim and connect him
    
    partyname = "Party1"
    connection.SimulationAddSipParty(partyname, factoryName)
    connection.SimulationConnectSipParty(partyname)
    
    confid = c.GetTextUnder("CONF_SUMMARY","ID")
    
    print "Wait until Conf is created..."
    for retry in range(numRetries):
        connection.SendXmlFile('Scripts/CreateConfWith4DialInSipParties/TransConfList.xml')
        confid = connection.GetTextUnder("CONF_SUMMARY","ID")
        confname = connection.GetTextUnder("CONF_SUMMARY","NAME")
        if confid != "":
            print
            break
        if (retry+1 == numRetries):
            print c.xmlResponse.toprettyxml()
            c.Disconnect()                
            sys.exit("Cannot monitor conf:" + status)
        sys.stdout.write(".")
        sys.stdout.flush()
        sleep(1)
    print "Conf:" + confname + " was Created"   
    sleep(2)
    connection.WaitAllOngoingConnected(confid,numRetries)
    sleep(2)
    
# Check if the SIP party was added and save his ID
    partyid = connection.GetPartyId(confid, partyname)
   
# Disconnect and Delete the SIP party  
    connection.SimulationDeleteSipParty(partyname)
  
    connection.DeleteConf(confid);
    
    sleep(2)
    
    #Remove the Factory 
    print "Remove the Factory..."
    connection.LoadXmlFile(RemoveFactoryFile)
    connection.ModifyXml("TERMINATE_MEETING_ROOM","ID",FacId)
    connection.Send()

    sleep(2)
    
    #Remove the profile
    connection.DelProfile(profId, "Scripts/AddDeleteFactoryWithSipParty/RemoveNewProfile.xml")
    
    return

#------------------------------------------------------------------------------
def CheckObjToken(connection,objToken):
    #send the objToken and make sure that we get it back in the same value
    connection.LoadXmlFile('Scripts/AddDeleteFactoryWithSipParty/TransMrList.xml') 
    connection.ModifyXml("GET_MEETING_ROOM_LIST","OBJ_TOKEN",objToken)
    connection.Send()
    responsToken = connection.xmlResponse.getElementsByTagName("MEETING_ROOM_SUMMARY_LS")[0].getElementsByTagName("OBJ_TOKEN")[0].firstChild.data
    return responsToken


##--------------------------------------- TEST ---------------------------------

c = McmsConnection()
c.Connect()

FactoryAddRemoveSipPartyTest(c,                     
                     'Scripts/AddDeleteFactoryWithSipParty/CreateNewProfile.xml',
                     'Scripts/AddDeleteFactoryWithSipParty/AddFactory.xml',                      
                     'Scripts/AddDeleteFactoryWithSipParty/RemoveFactory.xml',                                                 
                      20) #num of retries

c.Disconnect()
##------------------------------------------------------------------------------
