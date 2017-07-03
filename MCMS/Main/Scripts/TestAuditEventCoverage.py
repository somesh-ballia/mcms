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
    print "Test Event Coverage"
    print "-----------------------------------------------------"





##------------------------------ TEST --------------------------------------

PrintTestTitle()
    
# add 3 users: operator, auditor and administrator
#-------------------------------------------------

userAuditor = User("User_Administrator_2", "User_Administrator", "administrator")

userUtils = UsersUtilities()
userUtils.Connect()

userUtils.AddNewUserNew(userAuditor)

userUtils.Disconnect()

# statuses
statusOk = "Status OK"


# Check MCMS event arrival
#-------------------------------------------------

auditUtils = AuditUtils()

# Login
auditUtils.ConnectSpecificUser(userAuditor.m_Name, userAuditor.m_Password)
if(False == auditUtils.DoesEventExistInMemory("Login", statusOk)):
    print "event was not found in memory (1)"
    exit(1)

auditBufferFileName = auditUtils.GetAuditBufferFileName()

if(False == auditUtils.DoesEventExistInFile(auditBufferFileName, "Login", statusOk)):
    print "event was not found in file"
    exit(1)

PrintResult("Login - OK")


# Logout
auditUtils.Disconnect()
auditUtils.ConnectSpecificUser(userAuditor.m_Name, userAuditor.m_Password)
if(False == auditUtils.DoesEventExistInMemory("Logout", statusOk)):
    print "event was not found in memory (2)"
    exit(1)

if(False == auditUtils.DoesEventExistInFile(auditBufferFileName, "Logout", statusOk)):
    print "event was not found in file"
    exit(1)

PrintResult("Logout - OK")


# Disconnect
# TBD

PrintResult("DISCONNECTION - Not Implemented")



# Alerts
alertUtils = AlertUtils()
alertUtils.AddAlert(1)
if(False == auditUtils.DoesEventExistInMemory("Notification", statusOk, "Used for testing the Active Alarms mechanism")):
    print "event was not found in memory (3)"
    exit(1)

if(False == auditUtils.DoesEventExistInFile(auditBufferFileName, "Notification", statusOk, "Used for testing the Active Alarms mechanism")):
    print "event was not found in file"
    exit(1)

PrintResult("ALERT - OK")


# IP change via USB
if(False == auditUtils.DoesEventExistInMemory("IP Management Service modified", statusOk)):
    print "event was not found in memory (4)"
    exit(1)

if(False == auditUtils.DoesEventExistInFile(auditBufferFileName, "IP Management Service modified", statusOk)):
    print "event was not found in file"
    exit(1)

PrintResult("IP change via USB - OK")


# SET transaction
sysCfgUtils = SyscfgUtilities ()
sysCfgUtils.ConnectSpecificUser(userAuditor.m_Name, userAuditor.m_Password)
sysCfgUtils.UpdateSyscfgFlag("MCMS_PARAMETERS_DEBUG","KEEP_ALIVE_RECEIVE_PERIOD",1)

sleep(1)

if(False == auditUtils.DoesEventExistInMemory("Set configuration flags", statusOk)):
    print "event was not found in memory (5)"
    exit(1)

if(False == auditUtils.DoesEventExistInFile(auditBufferFileName, "Set configuration flags", statusOk)):
    print "event was not found in file"
    exit(1)

sysCfgUtils.Disconnect()

PrintResult("SET transaction - OK")


# PUT transaction
# TBD

PrintResult("PUT - Not Implemented")


# GET transaction (unsuccessful)
# TBD

PrintResult("GET - Not Implemented")

# rm/mk dir
# TBD

PrintResult("RM DIR- Not Implemented")

# rm/mk file
# TBD

PrintResult("RM FILE - Not Implemented")

auditUtils.Disconnect()
