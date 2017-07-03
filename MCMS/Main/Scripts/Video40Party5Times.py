#!/mcms/python/bin/python

#############################################################################
# This Script do:
# Date: 15/03/06
# By  : Yuri R.
#############################################################################


from McmsConnection import *
from datetime import date

import os
import sys



scriptsName = "VideoConfWith20Participants.py"
numOfIterations = 10

i = 1
while (i <= numOfIterations):
    print ""
    print "------------------------------------------"
    print scriptsName + " : " + str(i)
    print "------------------------------------------"
    
    sleep(5)
    
    os.system("Scripts/" + scriptsName)
    i += 1

os.system("Bin/McuCmd ps mcms")

    
    
    
    
    
