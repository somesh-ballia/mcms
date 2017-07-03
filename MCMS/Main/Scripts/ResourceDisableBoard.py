#!/mcms/python/bin/python
# For list of profiles look at RunTest.sh
#############################################################################
# Script which allow to disabe/enable the whole board.
# Prototype: Name_of_script BoardId Flag(YES/NO) 
# Example: Scripts/ResourceDisableBoard.py 1 YES
# By Sergey.
#############################################################################
from ResourceUtilities import *

c = ResourceUtilities()
c.Connect()

enable_flag = [' ','1', 'YES']
i=0
for arg in sys.argv:
    enable_flag[i] = str(arg)
    i = i+1

c.SendXmlFile("Scripts/ResourceDetailParty/CardDetailReq.xml")
unit_summary = c.xmlResponse.getElementsByTagName("UNIT_RESOURCE_DESCRIPTOR")
unit_len =len(unit_summary)
print "The number of configured units is  " + str(unit_len)

for i in range(unit_len):
    command_line = "Bin/McuCmd DisableUnit Resource " + str( enable_flag[1]) +" " +str(i+1)+ " " + str(enable_flag[2]) + '\n'
    os.system(command_line)

c.Disconnect()


