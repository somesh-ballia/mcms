#!/mcms/python/bin/python

#############################################################################
#Script which tests the repeated reservations feature
#NOTE: this script should NOT run with valgrind (it will take too long to make 2000 reservations)
#############################################################################

# For list of profiles look at RunTest.sh
#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_No_Valgrind
#-- SKIP_ASSERTS

import os
from ResourceUtilities import *

c = ResourceUtilities()
c.Connect()

profId = c.AddProfile("profile_for_recurrence")

c.CreateAndThenDeleteRepeated(profId, 2000)

c.Disconnect()
