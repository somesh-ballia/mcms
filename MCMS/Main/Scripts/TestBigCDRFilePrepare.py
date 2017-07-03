#!/mcms/python/bin/python

#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_No_Valgrind

import os
import sys

os.system("rm -f CdrFiles/*")
os.system("cp Scripts/c1.cdr CdrFiles/")
os.system("cp Scripts/cdrindex.log CdrFiles/")
os.system("chmod a+w CdrFiles/c1.cdr")

sys.exit(0)

