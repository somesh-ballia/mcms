#!/mcms/python/bin/python
# For list of profiles look at RunTest.sh
#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_6

from McmsConnection import *
 
c = McmsConnection()
c.Connect()

print "Send Set in section GATE_KEEPER: CM_KEEP_ALIVE_RECEIVE_PERIOD=3 (seconds) in system.cfg"
c.LoadXmlFile('Scripts/SetSystemCfg.xml') 

c.ModifyXml("SET_CFG","NAME","GATE_KEEPER")
c.ModifyXml("SET_CFG","KEY","CM_KEEP_ALIVE_RECEIVE_PERIOD")
c.ModifyXml("SET_CFG","DATA",3)
c.Send()
sleep(20)
#d = McmsConnection()
#d.Connect()
#d.LoadXmlFile('Scripts/SetSystemCfg.xml') 

#d.ModifyXml("SET_CFG","NAME","GATE_KEEPER")
#d.ModifyXml("SET_CFG","KEY","CM_KEEP_ALIVE_SEND_PERIOD")
#d.ModifyXml("SET_CFG","DATA",2)
#d.Send()

c.Disconnect()
#d.Disconnect()
