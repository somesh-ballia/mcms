#!/mcms/python/bin/python

# Re-Write date = 1/8/13
# Re-Write name = Uri A.

##===========================================================================================================##
#*Script_Info_Status="Active"
#*Script_Info_In_NightTest="Yes"
#*Script_Info_Name="Add20ConferenceNew.py"
#*Script_Info_Group="ConfParty"
#*Script_Info_Programmer="General ConfParty"
#*Script_Info_Version="V1"
#*Script_Info_Description="Add and delete 20 confrences"
##===========================================================================================================##


# For list of profiles look at RunTest.sh
#*PROCESSES_FOR_VALGRIND=ConfParty Resource CDR 
#-LONG_SCRIPT_TYPE

from McmsConnection import *
from ConfUtils.ConfCreation import *

McmsUtilClass = McmsConnection() # MCMS connection and XML_API file changes class 
McmsUtilClass.Connect()
ConfActionsClass = ConfCreation() # conf util class

# 1. Add conf x 20
num_of_conf = 20
ConfActionsClass.CreateConferences(McmsUtilClass, num_of_conf, 'Scripts/ConfTamplates/AddConfTemplate.xml')

# 2. delete conf x 20
ConfActionsClass.DeleteAllConferences(McmsUtilClass)

# 3. All conferences are deleted (no conf left in on-going)
delete_timeout = 20
ConfActionsClass.WaitAllConferencesAreDeleted(McmsUtilClass, delete_timeout)

McmsUtilClass.Disconnect()

