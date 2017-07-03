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

print "________2___Add  a new E1 service___________________________________"
Add_Service("try1","e1")

print "________3___Add  the span that was removed to the new service_______"
Add_Span(13,1,"try1")
Add_Span(13,5,"try1")
Add_Span(13,6,"try1")
Add_Span(13,7,"try1")

print "________4___Add the 8 E1 span to the new service(should fail)_______"
Add_Span(13,8,"try1","Maximum number of ISDN spans connected to this RTM ISDN card has been reached ***")

print "________5___Remove span_____________________________________________"
c.SendXmlFile('Scripts/Remove_span_from_service.xml')
#c.PrintLastResponse()
sleep(1)

print "________6___Add  the spans to board 2_______________________________"
Add_Span(14,5,"try1")
Add_Span(14,6,"try1")
Add_Span(14,7,"try1")

print "________7___Add the 8 E1 span to the new service(should fail)_______"
Add_Span(14,8,"try1","Maximum number of ISDN spans connected to this RTM ISDN card has been reached ***")

#c.PrintLastResponse()

print "________8___Test passed ____________________________________________"
sys.exit(0)


