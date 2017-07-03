#!/usr/bin/python

# For list of profiles look at RunTest.sh

#############################################################################
# Change IP Service gatekeeper part, and verify reset wasn't recommended.
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
os.system('Scripts/Startup.sh')

c.Connect()
c.LoadXmlFile('Scripts/Gatekeeper/GetIPService.xml')
c.Send()

gk_service_list=c.xmlResponse.getElementsByTagName("EXTERNAL_GATEKEEPER_ADDRESS")
list_len=len(gk_service_list)
if(list_len != 1):
        sys.exit("Wrong ip service response!")
extGKAddress=gk_service_list[0].firstChild.data

replace="1.3.5.7"
if(extGKAddress == replace):
	replace="7.5.3.1"

c.LoadXmlFile('Scripts/Gatekeeper/UpdateIPService.xml')

# change gatekeeper service
c.ModifyXml("GATEKEEPER", "EXTERNAL_GATEKEEPER_ADDRESS", replace)

# change other service, except for gatekeeper
c.ModifyXml("IP_QOS", "QOS_ACTION", "enabled")

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

if(counter != 1):
	sys.exit("Ip service changed event, should be produced!")
	
c.Disconnect()

# -------- for next startups
os.environ["CLEAN_CFG"]="YES"

