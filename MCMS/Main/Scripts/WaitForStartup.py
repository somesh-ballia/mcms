#!/mcms/python/bin/python

import xml.dom.minidom

from xml.dom.minidom import  parse, parseString,Document
import xml.etree.ElementTree as etree
import os
import os.path
from McmsConnection import *

print "WaitForStartup::Wait until startup ends"

def readfile(filename):
        f = open(filename,'r')
        string = ""
        while 1:
            line = f.readline()
            if not line:break
            string += line

        f.close()
        return string

# check that we got the startup_timeout as a param, if not assume default
if len(sys.argv) < 2:
	startup_timeout=30
else:
	startup_timeout=sys.argv[1]

c = McmsConnection()
port = 80
bIsSecured = False


try:
        mcuhomedir = str(os.environ['MCU_HOME_DIR'])
except:
        mcuhomedir = ""

if os.path.isfile(mcuhomedir+'/mcms/Cfg/NetworkCfg_Management.xml'):
        #print "read /mcms/Cfg/NetworkCfg_Management.xml"
        doc= parse(mcuhomedir+'/mcms/Cfg/NetworkCfg_Management.xml')
        securedElem = doc.getElementsByTagName("IS_SECURED")    
        if len(securedElem) > 0:                
                status = securedElem[0].firstChild.data
                print "System Secured Mode : " + status
                if status == "true":
			bIsSecured=True
			port=443
        else:
                print "could not find secured Element assuming non secured port"

try:	
	productType=readfile(mcuhomedir+"/mcms/ProductType")
	print productType
	if productType=="SOFT_MCU_MFW" and os.path.isfile(mcuhomedir+'/mcu_custom_config/custom.cfg'):
		xml_string=readfile(mcuhomedir+"/mcu_custom_config/custom.cfg")
		xml_etree = etree.fromstring(xml_string)
		CfgSection=xml_etree.findall(".//CFG_PAIR")
		if bIsSecured==False:			
			for pair in CfgSection:
			        if pair.find("KEY").text == "XML_API_PORT":
					port=int(pair.find("DATA").text)
		else:
	                for pair in CfgSection:
	                        if pair.find("KEY").text == "XML_API_HTTPS_PORT":
	                                port=int(pair.find("DATA").text)
except Exception:
	print "Error while trying to undertsand mfw cusom config configuration"
	
print "WaitForStartup::Startup timeout: " + str(startup_timeout)


c.Connect("POLYCOM","POLYCOM", "127.0.0.1",port)
print "Host Name:"
os.system("hostname")
state=""
for x in range(int(startup_timeout)):
    c.SendXmlFile('Scripts/GetMcuState.xml')
    state = c.GetTextUnder("MCU_STATE","DESCRIPTION")
    print "WaitForStartup::Mcu status:" + state
    if state != "Startup":
	if state == "Normal":
	   print "WaitForStartup::Card startup OK, state=Normal"
           c.Disconnect()
           sys.exit(0)
	else:
	   break
    sleep(2)

print "WaitForStartup::Card startup failed, state= " + state
c.Disconnect()
sys.exit(1)

