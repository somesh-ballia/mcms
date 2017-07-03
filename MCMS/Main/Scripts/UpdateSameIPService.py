#!/usr/bin/python

# For list of profiles look at RunTest.sh

#############################################################################
# Send IP Service without any change and verify reset wasn't recommended.
# By: Hillel
#############################################################################


from McmsConnection import *
from xml.dom import minidom 
import os

c = McmsConnection()
c.Connect()

# --- Set system with IP Service, and restart the system
os.environ["CLEAN_CFG"]="NO"
c.LoadXmlFile('Scripts/Gatekeeper/UpdateIPService.xml')
c.Send()
sleep(2)
os.system('Scripts/Startup.sh')


c.Connect()
c.LoadXmlFile('Scripts/Gatekeeper/UpdateIPService.xml')
c.Send()

# -------- check Alerts
sleep(1)
print ""
print "Monitor System Alerts list..."

c.SendXmlFile("Scripts/GetActiveAlarms.xml")
alerts_list = c.xmlResponse.getElementsByTagName("LOG_FAULT_ELEMENT")
num_of_alerts = len(alerts_list)

counter=0
for index in range(num_of_alerts):
        alert_desc = alerts_list[index].getElementsByTagName("DESCRIPTION")[0].firstChild.data
	if(alert_desc == "IP_SERVICE_CHANGED"):
		counter=counter+1

if(counter != 0):
	sys.exit("Ip service changed event, should NOT be produced!")
	
c.Disconnect()

# -------- for next startups
os.environ["CLEAN_CFG"]="YES"

