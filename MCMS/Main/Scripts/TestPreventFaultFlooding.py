#!/mcms/python/bin/python

#-- SKIP_ASSERTS



import os
import sys
from time import *


the_best_string = "cucu_lulu___"

numberOfIterations = 30
numberOfAsserts = 3

# ---------------------------------------------------------------------------------
#  Perform Flooding 
# ---------------------------------------------------------------------------------
for i in range(numberOfIterations):
    description = the_best_string + str(i)
    commandLine = "Bin/McuCmd @add_asserts cdr " + str(numberOfAsserts) + " "  + description 
    os.system(commandLine)



# ---------------------------------------------------------------------------------
#  Check protection from fault flooding, each fault type should appear once only 
# ---------------------------------------------------------------------------------
outputFileName = "tmpTerminalCommandOutput.yura"
for x in range(numberOfIterations):
    description = the_best_string + str(i)

    commandLine = "cat Faults/Faults.xml | grep " + description + " | wc -l > " + outputFileName
    os.system(commandLine)

    commandOutput = open(outputFileName)
    line = commandOutput.readline()
    numberOfAsserts = int(line)
    
    commandOutput.close()
    commandLine = "rm " + outputFileName
    os.system(commandLine)

    if(1 < numberOfAsserts):
        sys.exit(1)

sys.exit(0)
