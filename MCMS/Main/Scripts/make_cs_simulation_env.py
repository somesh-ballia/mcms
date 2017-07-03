#!/mcms/python/bin/python

import os, string
import sys
import xml.dom.minidom
import subprocess

from xml.dom.minidom import parse, parseString, Document
print "----------------- PREPARE ENV FOR CS SIMULATION ----------------------------------------------------------------------"
try:
	print "				* Read CS DIR 				"
	cs_dir=os.environ['CS_DIR']
	print "				* CS path = " + cs_dir
except KeyError:
	print "Please set the environment variable CS_DIR for CS .(<view>/vob/Carmel_IP/CS)"
	sys.exit(1)

print "				* CREATE LINK to CS DIR  "
if cs_dir != "":
	os.system("rm /tmp/cs")
		
cmd="ln -sf " + cs_dir + " /tmp/cs"
os.system(cmd)

print "				* CREATE LOG DIRECTORIES  "
os.system("mkdir -p /cs/logs/cs1/")
os.system("mkdir -p /cs/logs/cs2/")
os.system("mkdir -p /cs/logs/cs3/")
os.system("mkdir -p /cs/logs/cs4/")
os.system("mkdir -p /cs/logs/output/")

print "				* Make sure MCMS Simulation is in idle mode "
os.system("chmod 777 /cs/cfg/cfg_rmx2000/cs_private_cfg_dev.xml")
loadedXml= parse('/cs/cfg/cfg_rmx2000/cs_private_cfg_dev.xml')
module_list= loadedXml.getElementsByTagName("module")
print "module list range :" + str(len(module_list))

for index in range(len(module_list)):
    module_name= module_list[index].getElementsByTagName("description")[0].firstChild.data  
    print "\n Module name:" + module_name
    if module_name == "Mcms_Simulation" :
      	 runmode= module_list[index].getElementsByTagName("runmode")[0].firstChild.data
         if runmode != "idle" :
	    module_list[index].getElementsByTagName("runmode")[0].firstChild.data= "idle"
	    f =  open("/cs/cfg/cfg_rmx2000/cs_private_cfg_dev.xml", "wb")
	    loadedXml.writexml(f)
	    f.close()
         break

print "				* create ENV VARIABLES               "
os.system("source /cs/scripts/csipsets2")