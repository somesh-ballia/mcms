#!/mcms/python/bin/python
# -*- coding: utf-8 -*-

#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_19

from AuditUtils import *
from UsersUtils import *
from AlertUtils import *
from IpServiceUtils import *
from SysCfgUtils import *


ret = os.system("Scripts/TestAuditApi.py")
if(0 != ret):
    sys.exit("Scripts/TestAuditApi.py FAILED, ret = " + str(ret))
    
ret = os.system("Scripts/TestAuditEventCoverage.py")
if(0 != ret):
    sys.exit("Scripts/TestAuditEventCoverage.py FAILED, ret = " + str(ret))

ret = os.system("Scripts/TestAuditValidateEventFormat.py")
if(0 != ret):
    sys.exit("Scripts/TestAuditValidateEventFormat.py FAILED, ret = " + str(ret))


print "Full Audit Test Accomplished Successfully"
print ""

##------------------------------------------------------------------------------
