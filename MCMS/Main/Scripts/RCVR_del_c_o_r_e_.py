#!/mcms/python/bin/python
import sys
import os
import shutil
myPath="."
for entry in os.listdir(myPath):
    if ( os.path.isdir(os.path.join(myPath,entry))):
        continue
    else:
        if entry.find('.core.') != -1 and entry.find('dump') == -1 :
            print "CoreFile: " + entry + " was deleted "
            str_to_rm_core2=str("rm -f " + str(entry))
            os.system(str_to_rm_core2)
sys.exit(0)







