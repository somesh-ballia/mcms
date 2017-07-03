#!/usr/bin/python

# For list of profiles look at RunTest.sh

#############################################################################
# Test Script which test RtmIsdn service configuration - detaching a span to a service
# The expected behavior:
#    - detaching a span to a service
#    - ensure the span is stored as detached for next startup
# By: Haggai
#############################################################################


from McmsConnection import *


c = McmsConnection()
c.Connect()

#-------------------------------------------------------------------------------
# -------- Detach a span
#-------------------------------------------------------------------------------

# -------- ensuring that the span is attached
c.SendXmlFile("Scripts/RtmIsdnConfig/GetRtmIsdnSpansList.xml")
spans_list = c.xmlResponse.getElementsByTagName("RTM_ISDN_SPAN")
num_of_spans = len(spans_list)
span_attached = 0
for index in range(num_of_spans):
	fieldValueBoardId = spans_list[index].getElementsByTagName("SLOT_NUMBER")[0].firstChild.data
	if fieldValueBoardId == "13":
		fieldValueSpanId = spans_list[index].getElementsByTagName("SPAN_ID")[0].firstChild.data
		if fieldValueSpanId == "1":
			fieldValueIsAttached = spans_list[index].getElementsByTagName("IS_SPAN_ATTACHED_TO_SERVICE")[0].firstChild.data
			if fieldValueIsAttached == "true":
				span_attached = span_attached+1


print ""
print "Ensuring that span 1 of board 13 is attached..."
if span_attached <> 1:
	print "Test Failed, span 1 of board 13 is not attached"
	print ""
	sys.exit(1)
else:
	print "Span 1 of board 13 is attached. Test continues..."



# -------- detaching the span
print ""
print "Detaching span 1 of board 13..."
c.LoadXmlFile('Scripts/RtmIsdnConfig/AttachDetachRtmIsdnSpan.xml')
c.ModifyXml("RTM_ISDN_SPAN","SLOT_NUMBER","13")
c.ModifyXml("RTM_ISDN_SPAN","SPAN_ID","1")
c.ModifyXml("RTM_ISDN_SPAN","SERVICE_NAME","")
c.ModifyXml("RTM_ISDN_SPAN","IS_SPAN_ATTACHED_TO_SERVICE","false")
c.Send()


# -------- ensuring that the span is detached
c.SendXmlFile("Scripts/RtmIsdnConfig/GetRtmIsdnSpansList.xml")
spans_list = c.xmlResponse.getElementsByTagName("RTM_ISDN_SPAN")
num_of_spans = len(spans_list)
span_detached = 0
for index in range(num_of_spans):
	fieldValueBoardId = spans_list[index].getElementsByTagName("SLOT_NUMBER")[0].firstChild.data
	if fieldValueBoardId == "13":
		fieldValueSpanId = spans_list[index].getElementsByTagName("SPAN_ID")[0].firstChild.data
		if fieldValueSpanId == "1":
			fieldValueIsAttached = spans_list[index].getElementsByTagName("IS_SPAN_ATTACHED_TO_SERVICE")[0].firstChild.data
			if fieldValueIsAttached == "false":
				span_detached = span_detached+1

print "Ensuring that the span is now detached..."
if span_detached <> 1:
	print "Test Failed, span 1 of board 13 is not detached"
	print ""
	sys.exit(1)
else:
	print "Test passed. Span 1 of board 13 is now detached"


#-------------------------------------------------------------------------------
# -------- Restart - to check if the span is still detached
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

# -------- search for span 1 of board 13
c.SendXmlFile("Scripts/RtmIsdnConfig/GetRtmIsdnSpansList.xml")
spans_list = c.xmlResponse.getElementsByTagName("RTM_ISDN_SPAN")
num_of_spans = len(spans_list)
span_detached = 0
for index in range(num_of_spans):
	fieldValueBoardId = spans_list[index].getElementsByTagName("SLOT_NUMBER")[0].firstChild.data
	if fieldValueBoardId == "13":
		fieldValueSpanId = spans_list[index].getElementsByTagName("SPAN_ID")[0].firstChild.data
		if fieldValueSpanId == "1":
			fieldValueIsAttached = spans_list[index].getElementsByTagName("IS_SPAN_ATTACHED_TO_SERVICE")[0].firstChild.data
			if fieldValueIsAttached == "false":
				span_detached = span_detached+1

c.Disconnect()

print ""
print "Ensuring that the span is still detached..."
if span_detached <> 1:
	print "Test Failed, span 1 of board 13 is not detached"
	print ""
	sys.exit(1)
else:
	print "Test passed. Span 1 of board 13 is still detached"
	print ""
	sys.exit(0)
