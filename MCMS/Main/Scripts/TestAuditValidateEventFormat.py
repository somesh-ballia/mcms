#!/mcms/python/bin/python

from AuditUtils import *
from UsersUtils import *
from AlertUtils import *
from IpServiceUtils import *
from SysCfgUtils import *




def PrintResult(message):
    print ""
    print message
    print ""

def PrintTestTitle():
    print ""
    print "-----------------------------------------------------"
    print "Test Audit Validate Format Of Events"
    print "-----------------------------------------------------"


## validate 
## 
## <AUDIT_EVENT>
## 	<SEQ_NUMBER>1</SEQ_NUMBER>
## 	<DATE_TIME>2007-10-09T08:10:45</DATE_TIME>
## 	<USER_ID></USER_ID>
## 	<REPORTER>Mcms</REPORTER>
## 	<WORK_STATION></WORK_STATION>
## 	<IP_ADDRESS></IP_ADDRESS>
## 	<TYPE>Internal</TYPE>
## 	<ACTION>FAULT</ACTION>
## 	<STATUS>Ok</STATUS>
## 	<DESCRIPTION>0 : MCU_RESTART</DESCRIPTION>
## 	<DESCRIPTION_EX>System is starting</DESCRIPTION_EX>
## 	<FREE_DATA>RMX Version: 0.0.0.0, MCMS Version: 2.0.0.344</FREE_DATA>
## </AUDIT_EVENT>

def ValidateAuditEvent(eventElem):
    actionElem = eventElem.getElementsByTagName("ACTION")[0]
    seqNum = eventElem.getElementsByTagName("SEQ_NUMBER")[0]
    dataTime = eventElem.getElementsByTagName("DATE_TIME")[0]
    reporter = eventElem.getElementsByTagName("REPORTER")[0]
    workStation = eventElem.getElementsByTagName("WORK_STATION")[0]
    ipAddress = eventElem.getElementsByTagName("IP_ADDRESS")[0]
    type = eventElem.getElementsByTagName("TYPE")[0]
    action = eventElem.getElementsByTagName("ACTION")[0]
    status = eventElem.getElementsByTagName("STATUS")[0]
    description = eventElem.getElementsByTagName("DESCRIPTION")[0]
    descriptionEx = eventElem.getElementsByTagName("DESCRIPTION_EX")[0]
    freeData = eventElem.getElementsByTagName("FREE_DATA")[0]
    
    
def ValidateEventList(eventList):
    for auditElem in eventList:
        ValidateAuditEvent(auditElem)
   
##--------------------------------------- TEST ---------------------------------

PrintTestTitle()


# validate audit file
auditUtils = AuditUtils()
auditFileName = auditUtils.GetAuditBufferFileName()

xmlTree = parse(auditFileName)
auditListSummary = xmlTree.getElementsByTagName("AUDIT_EVENT_LIST")[0]
auditList = auditListSummary.getElementsByTagName("AUDIT_EVENT")

ValidateEventList(auditList)

PrintResult("Validate Audit File - OK")


# validate audit event list from API
auditUtils.Connect()
xmlTree = auditUtils.GetAuditEvents("Status OK")
auditListSummary = xmlTree.getElementsByTagName("AUDIT_EVENT_LIST")[0]
auditList = auditListSummary.getElementsByTagName("AUDIT_EVENT")

ValidateEventList(auditList)

PrintResult("Validate Audit FileList From API - OK")

