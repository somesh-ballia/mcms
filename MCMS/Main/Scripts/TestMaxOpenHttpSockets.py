#!/mcms/python/bin/python

import os, string
from array import *
from McmsConnection import *
from SysCfgUtils import *
#*export LOGIN_FILE_NAME=./Scripts/loginWithTooLongStationName.xml
    
##--------------------------------------- TEST ---------------------------------
connectionArr = []
print "Testing DMA Keep Allive = 140 sockets"	
i = 1
while i<141:
	con = McmsConnection();
	connectionArr.append(con)
	print "created Connection #" + str(i)
	con.CreateConnection();
	#sleep(1)
	i = i + 1

conlogin = McmsConnection();
conlogin.Connect();
conlogin.Disconnect();
print "Testing DMA Keep Allive = 140 sockets + an additional Connection , Success!!!"	

i = 0
while i<140:
	print "Disconnecting Connection #" + str(i+1)
	connectionArr[i].Disconnect("false");
	i = i + 1


del connectionArr[:]
connectionArr = []

print "Testing MAX_NUMBER_OF_MANAGEMENT_SESSIONS_PER_SYSTEM=10"	
sysCfgUtils = SyscfgUtilities();
sysCfgUtils.Connect();
sysCfgUtils.SendSetCfgParam("MAX_NUMBER_OF_MANAGEMENT_SESSIONS_PER_SYSTEM",10);
sysCfgUtils.Disconnect();
i = 1
#10 is max connection per system as was set earlier
while i<=10:
        con = McmsConnection();
        connectionArr.append(con)
        print "Connection #" + str(i) + " is now connected";
        con.Connect();
	#sleep(1);
        i = i + 1

additionalConn = McmsConnection();
print "Connection an extra connection beyhond the max number of sessions per users";
additionalConn.Connect(expected_statuses="Maximum number of permitted user connections has been exceeded. New connection is denied.");

i=1
while i<=10:
        print "Disconnecting Connection #" + str(i)
        connectionArr[i-1].Disconnect();
        i = i + 1

print "end Testing MAX_NUMBER_OF_MANAGEMENT_SESSIONS_PER_SYSTEM=10 , Success!!!!!"

del connectionArr[:]
connectionArr = []
print "Testing MAX_NUMBER_OF_MANAGEMENT_SESSIONS_PER_USER=80"	

sysCfgUtils = SyscfgUtilities();
sysCfgUtils.Connect();
sysCfgUtils.SendSetCfgParam("MAX_NUMBER_OF_MANAGEMENT_SESSIONS_PER_USER",80);
sysCfgUtils.SendSetCfgParam("MAX_NUMBER_OF_MANAGEMENT_SESSIONS_PER_SYSTEM",80);
sysCfgUtils.Disconnect();
#80 is max connection per user
i=1
while i<=80:
        con = McmsConnection();
        connectionArr.append(con)
        print "Connection #" + str(i) + " is now connected";
        con.Connect();
	#sleep(1);
        i = i + 1

additionalConn = McmsConnection();
print "Connection an extra connection beyhond the max number of sessions per users";
additionalConn.Connect(expected_statuses="Maximum number of permitted user connections has been exceeded. New connection is denied.");

i=1
while i<=80:
        print "Disconnecting Connection #" + str(i)
        connectionArr[i-1].Disconnect();
        i = i + 1

del connectionArr[:]
print "end Testing MAX_NUMBER_OF_MANAGEMENT_SESSIONS_PER_USER=80 , Success!!!!!"
##------------------------------------------------------------------------------
