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

def Add_T1_Service(serv_name,serv_type,expected_status="Status OK"):
	c = UsersUtilities()
	c.ConnectSpecificUser("SUPPORT","SUPPORT")
	c.LoadXmlFile('Scripts/Add_PSTN_T1_service.xml')
	c.ModifyXml("COMMON_SERVICE_PARAMS","NAME",serv_name)
	c.ModifyXml("COMMON_SERVICE_PARAMS","SPAN_TYPE",serv_type)
	c.Send(expected_status)

c = McmsConnection()
c.Connect()

print "________1___Remove span________________________________________________"
c.SendXmlFile('Scripts/Remove_span_from_service.xml')
#c.PrintLastResponse()

print "________2___Remove Service_____________________________________________"
c.SendXmlFile('Scripts/Del_PSTN_service.xml')

sleep(1)

print "________3___Add 2 a new T1 services____________________________________"
Add_T1_Service("try1","t1")
Add_T1_Service("try2","t2")

print "________4___Add  the span that was removed to the new service__________"
Add_Span(13,1,"try1")
Add_Span(13,2,"try1")
Add_Span(13,3,"try1")
Add_Span(13,4,"try1")
Add_Span(13,5,"try1")
Add_Span(13,6,"try1")
Add_Span(13,7,"try1")
Add_Span(13,8,"try1")
Add_Span(13,9,"try1")

print "________5___Add the 10th T1 span to the new service(should fail)_______"
Add_Span(13,10,"try1","Maximum number of ISDN spans connected to this RTM ISDN card has been reached ***")

print "________6___Remove span________________________________________________"
#c.SendXmlFile('Scripts/Remove_span_from_service.xml')
#c.PrintLastResponse()

sleep(1)

print "________7___Add  the spans to board 2__________________________________"
Add_Span(14,1,"try2")
Add_Span(14,2,"try2")
Add_Span(14,3,"try2")
Add_Span(14,4,"try2")
Add_Span(14,5,"try2")
Add_Span(14,6,"try2")
Add_Span(14,7,"try2")
Add_Span(14,8,"try2")
Add_Span(14,9,"try2")

print "________8___Add the 10th T1 span to the new service(should fail)_______"
Add_Span(14,10,"try1","Maximum number of ISDN spans connected to this RTM ISDN card has been reached ***")
#c.PrintLastResponse()

print "________9___Test passed _______________________________________________"

sys.exit(0)


