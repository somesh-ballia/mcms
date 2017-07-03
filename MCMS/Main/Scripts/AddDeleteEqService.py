#!/mcms/python/bin/python

# Re-Write date = 8/8/13
# Re-Write name = Uri A.

# For list of profiles look at RunTest.sh
#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_4
#-LONG_SCRIPT_TYPE

from McmsConnection import *
from ConfUtils.EqUtils import *
from IvrUtils.IvrUtils import *

McmsUtilClass = McmsConnection() # MCMS connection and XML_API file changes class 
McmsUtilClass.Connect()
EqActionsClass = EqUtils() # Eq util class
IvrActionsClass = IvrUtils() # Ivr util class


#--------------------  script flow start here --------------------------------
# 1. get EQ services list
num_of_Eqs_before = 0
num_of_Eqs_before = EqActionsClass.GetEqListNumber(McmsUtilClass)

# 2. add new EQ service
EqName = "EQ2"
EqActionsClass.AddEq(McmsUtilClass, IvrActionsClass, EqName)

sleep(3)

# 3. Check if EQ has been added
EqId = 0
found, EqId = EqActionsClass.GetEqByName(McmsUtilClass, EqName)
if (found == "false"):
    ScriptAbort("\nFailed to add EQ!!! Abort (check by name)!")

# 3.5, 3.6 to update Eq and to check changes is not currently implemented

# 4. Delete EQ
EqActionsClass.DeleteEqById(McmsUtilClass, EqId)

sleep(3)

# 5. Check EQ is deleted
num_of_Eqs_after = 0
num_of_Eqs_after = EqActionsClass.GetEqListNumber(McmsUtilClass)

if num_of_Eqs_before != num_of_Eqs_after:
    print "before" + str(num_of_Eqs_before)
    print "after" + str(num_of_Eqs_after)
    ScriptAbort("\nFailed to delete EQ!!! Abort!")
# another option is to failed to find EQ according to name

# 6. Disconnect
McmsUtilClass.Disconnect()
