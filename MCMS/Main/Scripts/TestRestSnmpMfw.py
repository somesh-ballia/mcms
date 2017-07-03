#!/mcms/python/bin/python

# #############################################################################
# Test SNMP REST API
#
# Date: 6/14
# By  : Yael A
#
############################################################################
#
#

#*PRERUN_SCRIPTS=
#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_1
#*export SOFT_MCU_MFW="YES"
#*export MCMS_DIR=`pwd`
#-- SKIP_ASSERTS

from McmsConnection import *

import subprocess
import re
#import shutil
import time
import os.path

def GetSnmp():
	print "Test REST GET SNMEP API using https://%s:%s/plcm/mcu/api/1.0/snmp:" % (ip, port)
	p = subprocess.Popen(['curl','-k','-H','If-None-Match: -1',
					  '-H','Content-Type: application/vnd.plcm.dummy+xml',
					  '-H','Authorization: Basic U1VQUE9SVDpTVVBQT1JU',
					  '-G','https://%s:%s/plcm/mcu/api/1.0/snmp' % (ip, port)], 
					  stdout=subprocess.PIPE, stderr=subprocess.PIPE)
	
	outGet, err = p.communicate()
	
	#print outGet
	return outGet

def PutSnmp(inreq):
	print "Test REST POST API using https://%s:%s/plcm/mcu/api/1.0/snmp " % (ip, port)
	p = subprocess.Popen(['curl','-k','-H','If-None-Match: -1',
					  '-H','Content-Type: application/vnd.plcm.dummy+xml',
					  '-H','Authorization: Basic U1VQUE9SVDpTVVBQT1JU',
					  '-X','POST',
					  '--data','%s' % (inreq),
					  'https://%s:%s/plcm/mcu/api/1.0/snmp' % (ip, port)],					 
					  stdout=subprocess.PIPE, 
	                                  stderr=subprocess.PIPE)
	out_set, err = p.communicate()
	return out_set
	
            
def CheckPattern(response, pattern, expected):
    found = True
    match=re.search(pattern,response)
    
    if match:
        found = True        
        print "Found %s" % (pattern)
    else:
        found = False
        print "Didn't  find %s" % (pattern)
    if (found != expected):            
        if (found):
            print "Error: %s was found in %s : %s" %(pattern, response, expected)            
        else:
            print "Error: %s was not found in %s %s" %(pattern, response, expected)
        File=open('/mcms/ProductType','w')
        File.write(str(P_Type[0]))
        File.close()
        sys.exit(1)
        

global ip
global port
global con
			
p = subprocess.Popen(['ps','-e','-f','|','grep','httpd','|','grep','-v','grep','|','wc','-l'],
				  stdout=subprocess.PIPE, 
                                  stderr=subprocess.PIPE)
out, err = p.communicate()

match=re.search('^2$',out)
if match:
	print "ERROR - Apache is not up"
	sys.exit(1)
else:
	print "Apache is up"

File=open('/mcms/ProductType')
P_Type=File.readlines()
File.close()

os.system('echo -n "SOFT_MCU_MFW" > /mcms/ProductType')

con = McmsConnection()

#c.Connect()
con.Connect("POLYCOM","POLYCOM", "127.0.0.1",4433, "Status OK")

ip = "127.0.0.1"
if ip == "127.0.0.1":
   try:
      ip=os.environ["TARGET_IP"]
   except KeyError:
      ip=RunCommand("echo -n `/sbin/ifconfig | grep 'inet add' | grep -v 127.0.0.1 | tr -s ' ' | cut -d':' -f2 | cut -d' ' -f1`")
      print ip

port=4433


out=GetSnmp()

CheckPattern(out, '<SNMP_DATA>.*</SNMP_DATA>', True)

CheckPattern(out, '<SNMP_VERSION>snmpv3</SNMP_VERSION>', True)
CheckPattern(out, '<ACCEPT_ALL_REQUESTS>true</ACCEPT_ALL_REQUESTS>', True)
CheckPattern(out, '<COMMUNITY_NAME>public</COMMUNITY_NAME>', True)
CheckPattern(out, '<SECURITY_USER_NAME>SameTime9</SECURITY_USER_NAME>', True)
CheckPattern(out, '<SECURITY_LEVEL>priv</SECURITY_LEVEL>', True)
CheckPattern(out, '<AUTHENTICATION_PROTOCOL>SHA</AUTHENTICATION_PROTOCOL>', True)
CheckPattern(out, '<PRIVACY_PROTOCOL>AES</PRIVACY_PROTOCOL>', True)

respout=PutSnmp(out)

#CheckPattern(respout, '<RETURN_STATUS>', False)
CheckPattern(respout, '<RETURN_STATUS>', True)


print "Trying to set snmpv2"
newSet=out.replace('<SNMP_VERSION>snmpv3</SNMP_VERSION>', '<SNMP_VERSION>snmpv2</SNMP_VERSION>')
respout=PutSnmp(newSet)

CheckPattern(respout, 'Only SNMP V3 is allowed', True)

print "Configuring trap"

newSet=out.replace('<TRAP_DESTINATION_LIST/>', '<TRAP_DESTINATION_LIST><TRAP_DESTINATION><IP>10.1.1.1</IP><TRAP_ADDRESS>10.1.1.1</TRAP_ADDRESS><COMMUNITY_NAME /><SECURITY_USER_NAME>yael</SECURITY_USER_NAME><AUTHENTICATION_PROTOCOL>SHA</AUTHENTICATION_PROTOCOL><AUTHENTICATION_PASSWORD>12345678</AUTHENTICATION_PASSWORD><PRIVACY_PROTOCOL>AES</PRIVACY_PROTOCOL><PRIVACY_PASSWORD>12345678</PRIVACY_PASSWORD><SECURITY_LEVEL>priv</SECURITY_LEVEL><ENGINE_ID /><TRAP_VERSION>snmpv3</TRAP_VERSION><TRAP_INFORM_ENABLED>false</TRAP_INFORM_ENABLED></TRAP_DESTINATION></TRAP_DESTINATION_LIST>')
respout=PutSnmp(newSet)

CheckPattern(respout, '<RETURN_STATUS>', True)


out=GetSnmp()
CheckPattern(out, '<TRAP_VERSION>snmpv3</TRAP_VERSION>', True)

print "Trying to set snmpv2 for trap"
newSet=out.replace('<TRAP_VERSION>snmpv3</TRAP_VERSION>', '<TRAP_VERSION>snmpv2</TRAP_VERSION>')
respout=PutSnmp(newSet)

CheckPattern(respout, 'Traps version must be V3', True)

print "Checking that snmp is up"
out=GetSnmp()
CheckPattern(out, '<SNMP_ENABLED>true</SNMP_ENABLED>', True)


snmp_p=RunCommand("ps -ef | grep snmpd | grep snmpd.conf | grep \"\-F\" | grep -v \"grep snmpd\"")
CheckPattern(snmp_p, 'snmpd', True)

print "Disabling snmp"
newSet=out.replace('<SNMP_ENABLED>true</SNMP_ENABLED>', '<SNMP_ENABLED>false</SNMP_ENABLED>')
respout=PutSnmp(newSet)

out=GetSnmp()
CheckPattern(out, '<SNMP_ENABLED>false</SNMP_ENABLED>', True)

snmp_p=RunCommand("ps -ef | grep snmpd | grep snmpd.conf | grep \"\-F\" | grep -v \"grep snmpd\"")
CheckPattern(snmp_p, 'snmpd', False)

print "Enabling snmp"

newSet=out.replace('<SNMP_ENABLED>false</SNMP_ENABLED>', '<SNMP_ENABLED>true</SNMP_ENABLED>')

respout=PutSnmp(newSet)
out=GetSnmp()
CheckPattern(out, '<SNMP_ENABLED>true</SNMP_ENABLED>', True)


File=open('/mcms/ProductType','w')
File.write(str(P_Type[0]))
File.close()
con.Disconnect()

print "Finish TestRestSnmpMfw"
                                                    
