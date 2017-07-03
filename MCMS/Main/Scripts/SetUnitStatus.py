#!/mcms/python/bin/python
 
from McmsConnection import *

import sys

c = McmsConnection()
c.Connect()
print "Send Set unit status For keep alive indication to:"


c.LoadXmlFile('Scripts/keep_allive/SetUnitStatus.xml') 
ToUnit=int(sys.argv[5])
fromUnit=int(sys.argv[3])
unitsRange= ToUnit-fromUnit+1
UnitStatus=-1
if sys.argv[6]=="fail":
   UnitStatus=3
if sys.argv[6]=="ok":  
   UnitStatus=0

if sys.argv[2] =="from":
  
  for i in range(unitsRange):
    print "Board Id: "+sys.argv[1]+ " Unit Id:" +str(fromUnit)+ " Status: " +sys.argv[6]+"("+str(UnitStatus)+")" 
    c.ModifyXml("SET_UNIT_STATUS_FOR_KEEP_ALIVE","BOARD_ID",sys.argv[1])
    c.ModifyXml("SET_UNIT_STATUS_FOR_KEEP_ALIVE","UNIT_ID",(fromUnit))
    
    c.ModifyXml("SET_UNIT_STATUS_FOR_KEEP_ALIVE","STATUS",UnitStatus)
    c.Send()
   # sleep(1)
    fromUnit=fromUnit+1
#else:
#  if sys.argv[3]=="fail":
#     UnitStatus=3
#  if sys.argv[3]=="ok":  
#     UnitStatus=0
#  print "Board Id: "+sys.argv[1]+ " Unit Id:" +sys.argv[2]+ " Status: " +sys.argv[3]+"("+str(UnitStatus)+")"  
#  c.ModifyXml("SET_UNIT_STATUS_FOR_KEEP_ALIVE","BOARD_ID",sys.argv[1])
#  c.ModifyXml("SET_UNIT_STATUS_FOR_KEEP_ALIVE","UNIT_ID",sys.argv[2])
#  c.ModifyXml("SET_UNIT_STATUS_FOR_KEEP_ALIVE","STATUS",sys.argv[3])
#  c.Send()
c.Disconnect()
