#!/mcms/python/bin/python

from AuditUtils import *
from UsersUtils import *
from AlertUtils import *
from IpServiceUtils import *
from SysCfgUtils import *


## <AUDIT_FILE_SUMMARY>
## 	<NAME>Audit_SN0000000001_FMD09102007_FMT132253_LMD09102007_LMT132317_SZ7261_SUY_CFnone_NFV01.adt</NAME>
## 	<SEQUENCE_NUMBER>1</SEQUENCE_NUMBER>
## 	<FILE_SIZE>7261</FILE_SIZE>
## 	<FIRST_MESSAGE>2007-10-09T13:22:53</FIRST_MESSAGE>
## 	<LAST_MESSAGE>2007-10-09T13:23:17</LAST_MESSAGE>
## 	<CONTAINS_STARTUP>1</CONTAINS_STARTUP>
## 	<COMPRESSION_FORMAT>none</COMPRESSION_FORMAT>
## 	<NAME_FORMAT_VERSION>1</NAME_FORMAT_VERSION>
## 	<VISUAL_NAME>Audit00000001_09-10-2007_13-22-53.log</VISUAL_NAME>
## </AUDIT_FILE_SUMMARY>
def ValidateAuditFileSummary(auditFileSummary):
    item1 = auditFileSummary.getElementsByTagName("NAME")[0]
    item2 = auditFileSummary.getElementsByTagName("SEQUENCE_NUMBER")[0]
    item3 = auditFileSummary.getElementsByTagName("FILE_SIZE")[0]
    item4 = auditFileSummary.getElementsByTagName("FIRST_MESSAGE")[0]
    item5 = auditFileSummary.getElementsByTagName("LAST_MESSAGE")[0]
    item6 = auditFileSummary.getElementsByTagName("CONTAINS_STARTUP")[0]
    item7 = auditFileSummary.getElementsByTagName("COMPRESSION_FORMAT")[0]
    item8 = auditFileSummary.getElementsByTagName("NAME_FORMAT_VERSION")[0]
    item9 = auditFileSummary.getElementsByTagName("VISUAL_NAME")[0]

    return True
    
def ValidateFileList(xmlFileList):
    auditListSummary = xmlFileList.getElementsByTagName("AUDIT_FILE_SUMMARY_LS")[0]
    auditFileList = auditListSummary.getElementsByTagName("AUDIT_FILE_SUMMARY")

    for auditFileSummary in auditFileList:
        res = ValidateAuditFileSummary(auditFileSummary)
        if(False == res):
            return res
            
    return True
    
    

def GetAuditFileList(auditUtils, user, expectedStatus):
    auditUtils.ConnectSpecificUser(user.m_Name, user.m_Password)
    fileList = auditUtils.GetAuditFileList(expectedStatus)
    return fileList

def GetAuditEvents(auditUtils, user, expectedStatus):
    auditUtils.ConnectSpecificUser(user.m_Name, user.m_Password)
    auditUtils.GetAuditEvents(expectedStatus)


def PrintResult(message):
    print ""
    print message
    print ""

def PrintTestTitle():
    print ""
    print "-----------------------------------------------------"
    print "Test Audit Api"
    print "-----------------------------------------------------"



##--------------------------------------- TEST ---------------------------------

PrintTestTitle()

# add 3 users: operator, auditor and administrator
#-------------------------------------------------
userOperator = User("User_Operator1", "User_Operator", "operator")
userAuditor = User("User_Auditor1", "User_Auditor", "auditor")
userAdministrator = User("User_Administrator1", "User_Administrator", "administrator")

userUtils = UsersUtilities()
userUtils.Connect()

userUtils.AddNewUserNew(userOperator)
userUtils.AddNewUserNew(userAuditor)
userUtils.AddNewUserNew(userAdministrator)

userUtils.Disconnect()

# statuses
statusNoPermission = "You are not authorized to perform this operation"
statusOk = "Status OK"



# Get Audit File List
#-------------------------------------------------
auditUtils = AuditUtils()

# using operator, should be rejected
GetAuditFileList(auditUtils, userOperator, statusNoPermission)

# using auditor, should be Ok
fileList = GetAuditFileList(auditUtils, userAuditor, statusOk)
ValidateFileList(fileList)

# using administrator, should be Ok
GetAuditFileList(auditUtils, userAdministrator, statusOk)
ValidateFileList(fileList)

PrintResult("Get File List - OK")


# Get Audit Files
#-------------------------------------------------

# using operator, should be rejected
auditUtils.ConnectSpecificUser(userOperator.m_Name, userOperator.m_Password)
#auditUtils.GetAuditFile(fileName, statusNoPermission)
# TBD

# using auditor, should be Ok
auditUtils.ConnectSpecificUser(userAuditor.m_Name, userAuditor.m_Password)
#auditUtils.GetAuditFile(fileName, statusOk)
# TBD

# using administrator, should be Ok
auditUtils.ConnectSpecificUser(userAdministrator.m_Name, userAdministrator.m_Password)
#auditUtils.GetAuditFile(fileName, statusOk)
# TBD

PrintResult("Get File - Not implemented")


# Get Audit Events
#-------------------------------------------------

# using operator, should be rejected
GetAuditEvents(auditUtils, userOperator, statusNoPermission)

# using auditor, should be Ok
GetAuditEvents(auditUtils, userAuditor, statusOk)

# using administrator, should be Ok
GetAuditEvents(auditUtils, userAdministrator, statusOk)

PrintResult("Get Events - OK")


PrintResult("Test of Audit API successfully ended")

