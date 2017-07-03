#!/mcms/python/bin/python
from McmsTargetConnection import *
from McmsConnection import *
#from test_recovery_policy_Utilities import *
#-- SKIP_ASSERTS
#-LONG_SCRIPT_TYPE
#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_No_Valgrind

import os, string
import sys
import shutil

sleep (5)

str_to_rm_core2=str("rm -f *.core.*")
os.system(str_to_rm_core2)

sys.exit(0)







