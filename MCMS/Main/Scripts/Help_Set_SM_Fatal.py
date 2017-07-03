#!/mcms/python/bin/python
 
from McmsConnection import *

import sys

c = McmsConnection()
c.Connect()
print "The command: Scripts/Set_SM_Fatal_Failure_ind.py 0 from <unit_id> to <unit_id> <StatusDescriptionBitmask>"
print "The command Argument: <unit_id> :"
print "unit_id: 1  to SWITCH "
print "unit_id: 2  to MFA"
print "unit_id: 3  to MFA - not in use (since currently Mfa_boardId_2 is not implemented in GideonSim's default configuration)"
print "unit_id: 4  to FANS"
print "unit_id: 5  to PWR"
print "unit_id: 6  to BACKPLANE"
print "unit_id: 7  to CNTL"

print "The command Argument: <StatusDescriptionBitmask> :"
print "Status Description Bitmask: a nember from 0 (ok) to 32 (not exist),The range 1-31 is the next bit mask:"
print "// 0--------00000                               "
print "//              |> Other (+1)                      "
print "//             |>  Voltage (+2)                    "
print "//            |>   Temperature major (+4)          "
print "//           |>    Temperature critical (+8)       "
print "//          |>     Failed (gone through a reset(+16)"
c.Disconnect()


