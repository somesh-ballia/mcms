#!/mcms/python/bin/python

#############################################################################
# Test Script which test the ful capacity with more than one conference( the number
# of participnts in each conference equal to highest common denaminator)
# 
#############################################################################
#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_No_Valgrind





#*export MPL_SIM_FILE="VersionCfg/MPL_SIM_2.XML"
#*export SYSTEM_CARDS_MODE_FILE="VersionCfg/SystemCardsMode_mpm.txt"
#*export LICENSE_FILE="VersionCfg/Keycodes_9251aBc471_80_YES_v100.0.cfs"
 

import os
from ResourceUtilities import *




r = ResourceUtilities()
r.Connect()



os.system("Bin/McuCmd set mcms IS_DOUBLE_DSP YES")

r.SendXmlFile("Scripts/ResourceFullCapacity/ResourceMonitorCarmel.xml")
num_of_resource = r.GetTotalCarmelParties()


print "RSRC_REPORT_RMX[Total] = " + str(num_of_resource)

r.TestFreeCarmelParties(num_of_resource)

##### split to several conferences with parties

num_conf = 1
num_parties = num_of_resource

for factor in range(int(num_of_resource)):
    
   num = int ( int(num_of_resource) % int(factor+2) )
   
   if num == 0:  
         num_conf = factor+2
         num_parties = ( int(num_of_resource) / int(factor+2) )
         break


SimpleNonTerminateXConfWithYParticipaints(r,int(num_conf),int(num_parties),
                               'Scripts/ResourceFullCapacity/AddVideoCpConf.xml',
                               'Scripts/ResourceFullCapacity/AddVideoParty1.xml',
                               20)

r.TestFreeCarmelParties( 0 )

r.DeleteAllConf(3)
r.WaitAllConfEnd(100)


r.TestFreeCarmelParties(int(num_of_resource))

r.Disconnect()

########################################################################################
