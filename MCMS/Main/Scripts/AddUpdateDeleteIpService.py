#!/mcms/python/bin/python

# For list of profiles look at RunTest.sh
#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_11

#*export CLEAN_CFG=YES
#*export USE_DEFAULT_IP_SERVICE=NO
#*export GIDEONSIM=YES
#*export ENDPOINTSSIM=YES

#-- EXPECTED_ASSERT(2)=HTTP_DIGEST_LIST
#-- EXPECTED_ASSERT(1)= Failed to shadow private data, audit event will be discarded

import os

from IpServiceUtils import *
from UsersUtils import *


ipServiceUtils = IpServiceUtils()



# ---------------------------------------------------------------------------------
#        Check No Service  
# ---------------------------------------------------------------------------------

userIndex = 1
ipServiceUtils.CheckServiceNotExist(userIndex)
userIndex = userIndex + 1




# ---------------------------------------------------------------------------------
#        Add Service 
# ---------------------------------------------------------------------------------

ipServiceUtils.PerformAddService(userIndex)
userIndex = userIndex + 1

ipServiceUtils.CheckServiceExist(userIndex)
userIndex = userIndex + 1


# ---------------------------------------------------------------------------------
#        Reset
# ---------------------------------------------------------------------------------

os.system("Scripts/Destroy.sh")
os.environ["CLEAN_CFG"]="NO"
os.system("Scripts/Startup.sh")




# ---------------------------------------------------------------------------------
#        Check Service Exists  
# ---------------------------------------------------------------------------------

ipServiceUtils.CheckServiceExist(userIndex)
userIndex = userIndex + 1


# ---------------------------------------------------------------------------------
#        Update Service 
# ---------------------------------------------------------------------------------

ipServiceUtils.PerformUpdateService(userIndex)
userIndex = userIndex + 1

isDynShouldBeUpdated = 0
ipServiceUtils.CheckServiceUpdated(userIndex, isDynShouldBeUpdated)
userIndex = userIndex + 1


# ---------------------------------------------------------------------------------
#        Reset
# ---------------------------------------------------------------------------------

os.system("Scripts/Destroy.sh")
os.system("Scripts/Startup.sh")



# ---------------------------------------------------------------------------------
#        Check Service Exists and Updated
# ---------------------------------------------------------------------------------

ipServiceUtils.CheckServiceExist(userIndex)
userIndex = userIndex + 1

isDynShouldBeUpdated = 1
ipServiceUtils.CheckServiceUpdated(userIndex, isDynShouldBeUpdated)
userIndex = userIndex + 1



# ---------------------------------------------------------------------------------
#        Delete Service 
# ---------------------------------------------------------------------------------

ipServiceUtils.PerformDeleteService(userIndex)
userIndex = userIndex + 1

ipServiceUtils.CheckServiceDeleted(userIndex)
userIndex = userIndex + 1

sys.exit(0)
