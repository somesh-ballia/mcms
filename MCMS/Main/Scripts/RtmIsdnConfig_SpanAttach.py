#!/usr/bin/python

# For list of profiles look at RunTest.sh

#############################################################################
# Test Script which test RtmIsdn service configuration - attaching a span to a service
# The expected behavior:
#    - attaching a span to a service
#    - ensure the span is stored as attached for next startup
# By: Haggai
#############################################################################


from McmsConnection import *


c = McmsConnection()
c.Connect()

#-------------------------------------------------------------------------------
# -------- Attach a span
#-------------------------------------------------------------------------------

# -------- ensuring that the span is detached
c.SendXmlFile("Scripts/RtmIsdnConfig/GetRtmIsdnSpansList.xml")
spans_list = c.xmlResponse.getElementsByTagName("RTM_ISDN_SPAN")
num_of_spans = len(spans_list)
span_detached = 0
for index in range(num_of_spans):
	fieldValueBoardId = spans_list[index].getElementsByTagName("SLOT_NUMBER")[0].firstChild.data
	if fieldValueBoardId == "13":
		fieldValueSpanId = spans_list[index].getElementsByTagName("SPAN_ID")[0].firstChild.data
		if fieldValueSpanId == "10":
			fieldValueIsAttached = spans_list[index].getElementsByTagName("IS_SPAN_ATTACHED_TO_SERVICE")[0].firstChild.data
			if fieldValueIsAttached == "false":
				span_detached = span_detached+1


print ""
print "Ensuring that span 10 of board 13 is detached..."
if span_detached <> 1:
	print "Test Failed, span 10 of board 13 is not detached"
	print ""
	sys.exit(1)
else:
	print "Span 10 of board 13 is detached. Test continues..."



# -------- ettaching the span
print ""
print "Attaching span 10 of board 13 to service 'ISDN'..."
c.LoadXmlFile('Scripts/RtmIsdnConfig/AttachDetachRtmIsdnSpan.xml')
c.ModifyXml("RTM_ISDN_SPAN","SLOT_NUMBER","13")
c.ModifyXml("RTM_ISDN_SPAN","SPAN_ID","10")
c.ModifyXml("RTM_ISDN_SPAN","SERVICE_NAME","ISDN")
c.ModifyXml("RTM_ISDN_SPAN","IS_SPAN_ATTACHED_TO_SERVICE","true")
c.Send()


# -------- ensuring that the span is attached
c.SendXmlFile("Scripts/RtmIsdnConfig/GetRtmIsdnSpansList.xml")
spans_list = c.xmlResponse.getElementsByTagName("RTM_ISDN_SPAN")
num_of_spans = len(spans_list)
span_attached = 0
for index in range(num_of_spans):
	fieldValueBoardId = spans_list[index].getElementsByTagName("SLOT_NUMBER")[0].firstChild.data
	if fieldValueBoardId == "13":
		fieldValueSpanId = spans_list[index].getElementsByTagName("SPAN_ID")[0].firstChild.data
		if fieldValueSpanId == "10":
			fieldValueIsAttached = spans_list[index].getElementsByTagName("IS_SPAN_ATTACHED_TO_SERVICE")[0].firstChild.data
			if fieldValueIsAttached == "true":
				span_attached = span_attached+1

print "Ensuring that the span is now attached..."
if span_attached <> 1:
	print "Test Failed, span 10 of board 13 is not attached"
	print ""
	sys.exit(1)
else:
	print "Test passed. Span 10 of board 13 is now attached"


#-------------------------------------------------------------------------------
# -------- Restart - to check if the span is still attached
#-------------------------------------------------------------------------------

# -------- restart
print ""
print "Restart (for starting over)..."
print ""

import os
import sys

os.environ["CLEAN_CFG"]="NO"
os.system("Scripts/Startup.sh")

c = McmsConnection()
c.Connect()

# -------- for next startups
os.environ["CLEAN_CFG"]="YES"


# -------- search for span 10 of board 1
c.SendXmlFile("Scripts/RtmIsdnConfig/GetRtmIsdnSpansList.xml")
spans_list = c.xmlResponse.getElementsByTagName("RTM_ISDN_SPAN")
num_of_spans = len(spans_list)
span_attached = 0
for index in range(num_of_spans):
	fieldValueBoardId = spans_list[index].getElementsByTagName("SLOT_NUMBER")[0].firstChild.data
	if fieldValueBoardId == "13":
		fieldValueSpanId = spans_list[index].getElementsByTagName("SPAN_ID")[0].firstChild.data
		if fieldValueSpanId == "10":
			fieldValueIsAttached = spans_list[index].getElementsByTagName("IS_SPAN_ATTACHED_TO_SERVICE")[0].firstChild.data
			if fieldValueIsAttached == "true":
				span_attached = span_attached+1

c.Disconnect()

print ""
print "Ensuring that the span is still attached..."
if span_attached <> 1:
	print "Test Failed, span 10 of board 13 is not attached"
	print ""
	sys.exit(1)
else:
	print "Test passed. Span 10 of board 13 is still attached"
	print ""
	sys.exit(0)
