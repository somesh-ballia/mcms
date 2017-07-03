#!/mcms/python/bin/python
#-LONG_SCRIPT_TYPE
#*export ACTIVE_DIRECTORY_FILE=Scripts/SysConfig/ActiveDirectoryConfiguration.xml
#*export SYSTEM_CFG_USER_FILE=Scripts/SysConfig/SystemCfgApacheKeepAlive120.xml
#*PROCESSES_FOR_VALGRIND =    LdapModule


from McmsConnection import *
import os

print "Test login using default user "

#defConnection = McmsConnection()
#defConnection.Connect();
print "Waitng 5 sec to  LdapModule to connect to active directory...."
sleep(5)
print "Test Login using night test user to the active directory"
c = McmsConnection()
c.Connect("nighttest","Polycom123")

