#!/mcms/python/bin/python

###################################
# Re-Write date = 25/8/13
# Re-Write name = Uri A.
###################################

#-LONG_SCRIPT_TYPE
# For list of profiles look at RunTest.sh
#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_4

# import Python classes
from CapabilitiesSetsDefinitions import *
from McmsConnection import *
from ConfUtils.ConfCreation import *

confName = "VoipConf"
displayName = confName
for arg in sys.argv: 
    if (arg.startswith("DISPLAY_NAME=")==True):
        arg=arg.replace("DISPLAY_NAME=","")
        print arg
        displayName=arg
        print displayName

McmsUtilClass = McmsConnection() # MCMS connection and XML_API file changes class 
McmsUtilClass.Connect()
ConfActionsClass = ConfCreation() # conf util class

# 1. add conf
# 2. add parties
# 3. connect parties
# 4. check parties connected
# 5. delete conf + parties


# 2. Create Conf
ConfActionsClass.CreateConfNameAndDisplayName(McmsUtilClass, confName, displayName ,'Scripts/ConfTamplates/AddConfTemplate.xml', "AUDIO")
confid = ConfActionsClass.WaitConfCreated(McmsUtilClass, confName)

# 3. Adding parties
numParties = ConfActionsClass.ConnectH323Parties(McmsUtilClass, confid, 3, "VoipParty", "AUDIO")

# 4. All parties are connected
ConfActionsClass.WaitAllOngoingConnected(McmsUtilClass, confid)


sleep(1)

# 5. delete Conf
ConfActionsClass.DeleteConf(McmsUtilClass, confid)
ConfActionsClass.WaitAllConferencesAreDeleted(McmsUtilClass)

# 6. Disconnect python connection to RMX simulation
McmsUtilClass.Disconnect()



