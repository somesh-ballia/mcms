#!/mcms/python/bin/python
# -*- coding: utf-8 -*-

import os

from AlertUtils import *



# Reset with bad ip service
os.system("Scripts/Destroy.sh")
os.environ["USE_ALT_IP_SERVICE"]="VersionCfg/DefaultIPServiceListWithExAscii.xml"
os.system("Scripts/Startup.sh")



# Check the existanse of an appropriate Alert
alertUtils = AlertUtils()

strAlertName = "NO_SERVICE_FOUND_IN_DB"
isAAExist = alertUtils.IsAlertExist("CSMngr", "Manager", strAlertName, 1)
if(isAAExist == False):
    print "the alert " + strAlertName + " was not found, Very bad"
    sys.exit(1)



# Check the existanse of an appropriate error file(the bad file shoukd be renamed)
strRenamedFile = "Cfg/IPServiceList.xml_error.xml"
isRenamedFileExist = os.path.exists(strRenamedFile)
if(isRenamedFileExist == False):
    print "the renamed file " + strRenamedFile + " was not found, Very bad"
    sys.exit(1)

print
print "--------------------------------------------"
print "The CSMngr has rejected the bad Ip Service."
print "Alert was found and bad File was renamed, Very Good"
print
print
