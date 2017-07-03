#!/mcms/python/bin/python

#############################################################################
# Test Script which Add and delete a Meetin rooms reservation 
# The Transactions which will be tested:
#   1.Add flag to system.cfg 
#   2.Check that the flag has added
#   3.Delete the flag from system.cfg
#   4.Get Meeting Room List+ delta monitoring
#   5.Get Single Meeting Room
#  
# Date: 06/03/06
# By  : Shlomit B.
#####################################################################################
import os
import sys
import shutil
 
from McmsConnection import *
 
c = McmsConnection()
c.Connect()

c = McmsConnection()
c.Connect()
############save the default values#################to return at the end of the script 
########### 1.Add flag to system.cfg ################################################

print "Send Set in section GATE_KEEPER: CM_KEEP_ALIVE_RECEIVE_PERIOD=3 (seconds) in system.cfg"
c.LoadXmlFile('Scripts/SetSystemCfg.xml') 

c.ModifyXml("SET_CFG","NAME","GATE_KEEPER")
c.ModifyXml("SET_CFG","KEY","CM_KEEP_ALIVE_RECEIVE_PERIOD")
c.ModifyXml("SET_CFG","DATA",3)
c.Send()

########### 2.Check that the flag has added ##########################################

print "get the flag from system.cfg ..."
c.SendXmlFile("Scripts/GetSystemCfg.xml")

cfg_elm_list = c.xmlResponse.getElementsByTagName("RESPONSE_TRANS_CFG")

for index in range(len(cfg_elm_list)):
     cfg_desc =c.xmlResponse.getElementsByTagName("CFG_PAIR")[0].getElementsByTagName("KEY")[0].firstChild.data
     if cfg_desc == "CM_KEEP_ALIVE_RECEIVE_PERIOD":
        cfg_data = c.xmlResponse.getElementsByTagName("CFG_PAIR")[0].getElementsByTagName("DATA")[0].firstChild.data 
        if cfg_data == "3":
           print cfg_desc+"= " + cfg_data + " found system.cfg" 

########### 1.Add flag to system.cfg ################################################

os.system("export CLEAN_CFG=NO")
os.system("Scripts/Startup.sh")
os.system("Scripts/SocketDisconnectConnect.py")
   
         
c.Disconnect()

