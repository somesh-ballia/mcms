#!/mcms/python/bin/python

# #############################################################################
# Change configuration of Flexera Server Simualation
#
# Date: 23/08/09
# By  : Kobi G  
#
############################################################################
#
#
import os
import sys
from McmsConnection import *
expected_status="In progress;Status OK"
print
print "Start FlexeraLicense test"
print
os.system("cp Scripts/LicensingModuleSimulation/FullLicenseWith20RPCS.xml Cfg/LicensingDebugFeatureList.xml")
os.system("Scripts/McuCmd.sh @refresh LicenseServer")
print "End of FlexeraLicense test"
print

