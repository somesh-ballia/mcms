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
	c.ModifyXml("SPAN_DEFINITION","SPAN_TYPE",serv_type)
	c.Send(expected_status)


c = McmsConnection()
c.Connect()

sleep(3)
     
print "________1___Remove Service_____________________________________________"
c.SendXmlFile('Scripts/Del_PSTN_service.xml')
#c.PrintLastResponse()
sleep(1)

print "________2___Add  a new t1 service______________________________________"
Add_Service("t1_1","t1")

print "________3___Add more T1 spans to the old service (the 10 should fail)__"
Add_Span(13,1,"t1_1")
Add_Span(13,2,"t1_1")
Add_Span(13,3,"t1_1")
Add_Span(13,4,"t1_1")
Add_Span(13,5,"t1_1")
Add_Span(13,6,"t1_1")
Add_Span(13,7,"t1_1")
Add_Span(13,8,"t1_1")
Add_Span(13,9,"t1_1")
Add_Span(13,10,"t1_1","Maximum number of ISDN spans connected to this RTM ISDN card has been reached ***")

print "________4___Test passed _______________________________________________"
sys.exit(0)


