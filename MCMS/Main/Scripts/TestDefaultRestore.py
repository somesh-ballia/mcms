#!/mcms/python/bin/python

#############################################################################
# Test Script which Making sure that the restore defaualt is working prperly
# Date: 11/07/06
# By  : Udi B.
#############################################################################

# For list of profiles look at RunTest.sh
#*PROCESSES_FOR_VALGRIND= ConfParty

from McmsConnection import *

#------------------------------------------------------------------------------
def TestRestoreDefault(connection,timeout):
    connection.SendXmlFile("Scripts/GetProfileList.xml")
    profileList =connection.xmlResponse.getElementsByTagName("PROFILE_SUMMARY")

    expectedProfileListSize=2
    print "Make sure profile list is: "+str(expectedProfileListSize)
    if expectedProfileListSize != len(profileList):
       sys.exit("Profile list size is: " + str(len(profileList)) +  ", expecting : " + str(expectedProfileListSize))

    profileExpectedName="Factory_Video_Profile"
    print "Make sure profile name is: " + profileExpectedName
    profileName=profileList[0].getElementsByTagName("NAME")[0].firstChild.data
    ProfId = profileList[0].getElementsByTagName("ID")[0].firstChild.data
    if (profileExpectedName != profileName):
        sys.exit("Profile name is: " +profileName+", while expecting:"+ profileExpectedName)
    mrNameToIdTable = {'DefaultEQ':0,
                       'DefaultFactory':0,
                       'Maple_Room':0,
                       'Oak_Room':0,
                       'Juniper_Room':0,
                       'Fig_Room':0,
                       }
    tmpNid=0
    for key in mrNameToIdTable.keys():
        mrNameToIdTable[key],tmpNid = connection.WaitMRCreated(key)

    for mrId in mrNameToIdTable.values():
        connection.DelReservation(int(mrId), 'Scripts/AddRemoveMrNew/DeleteMR.xml')
                
    
    
## ---------------------- Test --------------------------

c = McmsConnection()
c.Connect()
TestRestoreDefault(c,20)

c.Disconnect()

