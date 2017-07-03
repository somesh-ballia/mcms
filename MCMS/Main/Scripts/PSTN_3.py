#!/usr/bin/python
from UsersUtils import *
from McmsConnection import *
def Add_Span(slot,span,serv_name,expected_status="Status OK"):
	c = UsersUtilities()
	c.ConnectSpecificUser("SUPPORT","SUPPORT")
	c.LoadXmlFile('Scripts/Add_span_to_service.xml')
	c.ModifyXml("RTM_ISDN_SPAN","SERVICE_NAME",serv_name)
	c.ModifyXml("RTM_ISDN_SPAN","SLOT_NUMBER",slot)
	c.ModifyXml("RTM_ISDN_SPAN","SPAN_ID",span)
	c.Send(expected_status)

def Add_Service(serv_name,serv_type,expected_status="Status OK"):
	c = UsersUtilities()
	c.ConnectSpecificUser("SUPPORT","SUPPORT")
	c.LoadXmlFile('Scripts/Add_PSTN_service.xml')
	c.ModifyXml("COMMON_SERVICE_PARAMS","NAME",serv_name)
	c.ModifyXml("COMMON_SERVICE_PARAMS","SPAN_TYPE",serv_type)
	c.Send(expected_status)


c = McmsConnection()
c.Connect()

print "________1___Remove span_____________________________________________"
c.SendXmlFile('Scripts/Remove_span_from_service.xml')
#c.PrintLastResponse()
sleep(1)

print "________2___Add  a new T1 service___(should fail)___________________"
Add_Service("try1","t1","Inconsistent parameters ***")

print "________3___Test passed ____________________________________________"
sys.exit(0)



