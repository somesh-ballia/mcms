#!/mcms/python/bin/python

# #############################################################################
# Test REST API
#
# Date: 25/11/13
# By  : Shachar Bar
#
############################################################################
#
#

#*PRERUN_SCRIPTS=
#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_1
#*export SOFT_MCU_MFW="YES"
#*export MCMS_DIR=`pwd`
#*export USE_ALT_MNGMNT_SERVICE=Scripts/CsSimulationConfig/NetworkCfg_Management_ipv4_only.xml
#-- SKIP_ASSERTS

from McmsConnection import *

import subprocess
import re
#import shutil
import time
import os.path

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

ip = RunCommand("echo -n `/sbin/ifconfig | grep 'inet add' | grep -v 127.0.0.1 | tr -s ' ' | cut -d':' -f2 | cut -d' ' -f1`")
port = RunCommand("echo -n `cat /tmp/httpd.listen.conf | grep '^Listen' | awk '{print $2}' | cut -f2- -d':'`")
isSecured = RunCommand("echo -n `cat Cfg/NetworkCfg_Management.xml | grep '<IS_SECURED>true</IS_SECURED>'`")

os.environ["TARGET_IP"]=ip
os.environ["TARGET_PORT"]=port

c = McmsConnection()
c.Connect()

if isSecured!="":
	base_url  = "https://"+ip+":"+port 
else:
	base_url  = "http://"+ip+":"+port

print "Test REST GET API using " + base_url +"/plcm/mcu/api/1.0/user:"  
p = subprocess.Popen(['curl','-k','-H','If-None-Match: -1',
				  '-H','Content-Type: application/vnd.plcm.dummy+xml',
				  '-H','Authorization: Basic U1VQUE9SVDpTVVBQT1JU',
				  '-G',base_url+'/plcm/mcu/api/1.0/user'], 
				  stdout=subprocess.PIPE, 
                                  stderr=subprocess.PIPE)
out, err = p.communicate()
print out

match=re.search('^<OPER_LIST>.*administrator.*</OPER_LIST>$',out)
if match:
	print "SUCCESS"
else:
	print "ERROR"
	File=open('/mcms/ProductType','w')
	File.write(str(P_Type[0]))
	File.close()
	sys.exit(1)

print "Test REST POST API using "+base_url+"/plcm/mcu/api/1.0/user:"
p = subprocess.Popen(['curl','-k','-no-check-certificate','-H','If-None-Match: -1',
				  '-H','Content-Type: application/vnd.plcm.dummy+xml',
				  '-H','Authorization: Basic U1VQUE9SVDpTVVBQT1JU',
				  '-X','POST',
				  '--dump-header','/tmp/testRestApi.out',
				  '--data','<OPERATOR><USER_NAME>AlbertEinstein</USER_NAME><PASSWORD>Polycom</PASSWORD></OPERATOR>',
				  base_url+'/plcm/mcu/api/1.0/user'],
				  stdout=subprocess.PIPE, 
                                  stderr=subprocess.PIPE)
out, err = p.communicate()

HttpResult = RunCommand("echo -n `cat /tmp/testRestApi.out | grep '^HTTP/1.1' | awk '{print $2}'`")
print "Http return Code: " + HttpResult

#match=re.search('^.*<DESCRIPTION>Status OK</DESCRIPTION>.*</RETURN_STATUS>$',out)
if HttpResult=="200" or HttpResult=="204":
	print "SUCCESS"
else:
	print "ERROR"
	File=open('/mcms/ProductType','w')
	File.write(str(P_Type[0]))
	File.close()
	sys.exit(1)

print "Test REST DELETE API using "+ base_url+"/plcm/mcu/api/1.0/user/AlbertEinstein:"
p = subprocess.Popen(['curl','-k','-H','If-None-Match: -1',
				  '-H','Content-Type: application/vnd.plcm.dummy+xml',
				  '-H','Authorization: Basic U1VQUE9SVDpTVVBQT1JU',
				  '-no-check-certificate',
			          '--dump-header','/tmp/testRestApi.out',
				  '-X','DELETE',base_url+'/plcm/mcu/api/1.0/user/AlbertEinstein'], 
				  stdout=subprocess.PIPE, 
                                  stderr=subprocess.PIPE)
out, err = p.communicate()
HttpResult = RunCommand("echo -n `cat /tmp/testRestApi.out | grep '^HTTP/1.1' | awk '{print $2}'`")
print "Http return Code: " + HttpResult

if HttpResult=="200" or HttpResult=="204":
	print "SUCCESS"
else:
	print "ERROR"
	File=open('/mcms/ProductType','w')
	File.write(str(P_Type[0]))
	File.close()
	sys.exit(1)

print "Test REST PUT API using http://127.0.0.1:" + port+ "/plcm/mcu/api/1.0/system/logger:"
p = subprocess.Popen(['curl','-k','-H','If-None-Match: -1',
				  '-H','Content-Type: application/vnd.plcm.dummy+xml',
				  '-H','Authorization: Basic U1VQUE9SVDpTVVBQT1JU',
				  '-X','PUT',
				  '-no-check-certificate',
				  '--dump-header','/tmp/testRestApi.out',
				  '--data','<LOG_CONFIG_DATA><MAX_LOG_SIZE>2</MAX_LOG_SIZE><LOCAL_APPENDER><CHECKED>true</CHECKED><MODULE_LIST><MODULE_INFO><MODULE_NAME>Mcms</MODULE_NAME><CHECKED>true</CHECKED><LOG4CXX_LEVEL>WARN</LOG4CXX_LEVEL><PROCESS_LIST><PROCESS_INFO><ALL_MODULE_PROCESS_NAME>McmsDaemon</ALL_MODULE_PROCESS_NAME><CHECKED>true</CHECKED></PROCESS_INFO></PROCESS_LIST></MODULE_INFO></MODULE_LIST></LOCAL_APPENDER></LOG_CONFIG_DATA>',
				  base_url+'/plcm/mcu/api/1.0/system/logger'], 
				  stdout=subprocess.PIPE, 
                                  stderr=subprocess.PIPE)
out, err = p.communicate()
HpttpResult = RunCommand("echo -n `cat /tmp/testRestApi.out | grep '^HTTP/1.1' | awk '{print $2}'`")
print "Http return Code: " + HttpResult

#match=re.search('^<UPDATE><LOG_CONFIG_DATA>.*</LOG_CONFIG_DATA></UPDATE>$',out)
if HttpResult=="200" or HttpResult=="204":
	print "SUCCESS"
else:
	print "ERROR"
	File=open('/mcms/ProductType','w')
	File.write(str(P_Type[0]))
	File.close()
	sys.exit(1)


print "Test Backup operation:"
p = subprocess.Popen(['curl','-k','-H','If-None-Match: -1',
                                  '-H','Content-Type: application/vnd.plcm.dummy+xml',
                                  '-H','Authorization: Basic U1VQUE9SVDpTVVBQT1JU',
                                  '-X','POST',
                                  '-no-check-certificate',
				  '--dump-header','/tmp/testRestApi.out',
                                  base_url+'/plcm/mcu/api/1.0/system/backup/finish'],
                                  stdout=subprocess.PIPE,
                                  stderr=subprocess.PIPE)
out, err = p.communicate()
print "First Finish Previous Install if Any"
HttpResult = RunCommand("echo -n `cat /tmp/testRestApi.out | grep '^HTTP/1.1' | awk '{print $2}'`")
print "Http return Code: " + HttpResult
if HttpResult=="200" or HttpResult=="204" or HttpResult=="409":
        print "SUCCESS"
else:
        print "ERROR"
        File=open('/mcms/ProductType','w')
        File.write(str(P_Type[0]))
        File.close()
        sys.exit(1)


p = subprocess.Popen(['curl','-k','-H','If-None-Match: -1',
				  '-H','Content-Type: application/vnd.plcm.dummy+xml',
				  '-H','Authorization: Basic U1VQUE9SVDpTVVBQT1JU',
				  '-X','POST',
				  '-no-check-certificate',
				  '--dump-header','/tmp/testRestApi.out',
				  base_url+'/plcm/mcu/api/1.0/system/backup/start'],
				  stdout=subprocess.PIPE, 
                                  stderr=subprocess.PIPE)
out, err = p.communicate()
print "Start Backup Operation..."
HttpResult = RunCommand("echo -n `cat /tmp/testRestApi.out | grep '^HTTP/1.1' | awk '{print $2}'`")
print "Http return Code: " + HttpResult

if HttpResult=="200" or HttpResult=="204":
        print "SUCCESS"
else:
        print "ERROR"
        File=open('/mcms/ProductType','w')
        File.write(str(P_Type[0]))
        File.close()
        sys.exit(1)



print "Test Backup filename:"
p = subprocess.Popen(['curl','-k','-H','If-None-Match: -1',
				  '-H','Content-Type: application/vnd.plcm.dummy+xml',
			  	  '-H','Authorization: Basic U1VQUE9SVDpTVVBQT1JU',
				  '-G',
				  base_url+'/plcm/mcu/api/1.0/system/directory/Backup'],
				  stdout=subprocess.PIPE, 
				  stderr=subprocess.PIPE)
out, err = p.communicate()
print out

#os.system("ls -l /mcms/Backup")
filename = re.sub('<.[^>]*>','',out,count=0)
filename=filename.rstrip()
File="/mcms/Backup/" + filename
#shutil.copy2(File,"/tmp")
#os.system("ls -l /tmp/" + filename)
print "Backup filename: ",filename
match=re.search('^.*Backup.*$',out)
if match:
	print "SUCCESS"
	p = subprocess.Popen(['curl','-k','-H','If-None-Match: -1',
				  '-H','Content-Type: application/vnd.plcm.dummy+xml',
			  	  '-H','Authorization: Basic U1VQUE9SVDpTVVBQT1JU',
				  '-X','POST',
				  '-no-check-certificate',
				   
				  base_url+'/plcm/mcu/api/1.0/system/backup/finish'],
				  stdout=subprocess.PIPE, 
				  stderr=subprocess.PIPE)
	p.communicate()
else:
	print "ERROR"
	File=open('/mcms/ProductType','w')
	File.write(str(P_Type[0]))
	File.close()
	sys.exit(1)

os.remove("/tmp/testRestApi.out")
print "Rest Api Tests Finish Successfully!!!"

#print "Test Restore filename:"
#p = subprocess.Popen(['curl','-k','-H','If-None-Match: -1',
#				  '-H','Content-Type: application/vnd.plcm.dummy+xml',
#			  	  '-H','Authorization: Basic U1VQUE9SVDpTVVBQT1JU',
#				  '-X','POST',
#				  'http://127.0.0.1:8080/plcm/mcu/api/1.0/system/restore/start'],
#				  stdout=subprocess.PIPE, 
#				  stderr=subprocess.PIPE)
#out, err = p.communicate()
#match=re.search('^.*Status OK.*$',out)
#if match:
#	print "SUCCESS"
#else:
#	print "ERROR"
#	sys.exit(1)
#
#Restore_File="file=@/tmp/" + filename
#print Restore_File
#curl -s -k -H "$PARM" -H "$AUTH" -H "$UPLOAD_TYPE" -X POST -F file=@${Restore_File} https://$MYIP/plcm/mcu/api/1.0/system/restore/`basename ${Restore_File}`
#p = subprocess.Popen(['curl','-k','-H','If-None-Match: -1',
#			  	  '-H','Authorization: Basic U1VQUE9SVDpTVVBQT1JU',
#				  '-H','Content-Type: Content-Type: multipart/form-data',
#				  '-X','POST','-F',Restore_File,
#				  'http://127.0.0.1:8080/plcm/mcu/api/1.0/system/restore/' + filename],
#				  stdout=subprocess.PIPE, 
#				  stderr=subprocess.PIPE)
#out, err = p.communicate()
#curl -s -k -H "$PARM" -H "$TYPE" -H "$AUTH" -G https://$MYIP/plcm/mcu/api/1.0/system/info/state
#p = subprocess.Popen(['curl','-k','-H','If-None-Match: -1',
#				  '-H','Content-Type: application/vnd.plcm.dummy+xml',
#			  	  '-H','Authorization: Basic U1VQUE9SVDpTVVBQT1JU',
#				  '-G',
#				  'http://127.0.0.1:8080/plcm/mcu/api/1.0/system/info/state'],
#				  stdout=subprocess.PIPE, 
#				  stderr=subprocess.PIPE)
#out, err = p.communicate()
#match=re.search('^.*in_progress.*$',out)
#if match:
#	print "Restore is in progress"
#else:
#	print "ERROR"
#	sys.exit(1)
#
#print "Waiting for Restore to finish"
#
#FINISH=0
#while FINISH == 0:
#	p = subprocess.Popen(['curl','-k','-H','If-None-Match: -1',
#					  '-H','Content-Type: application/vnd.plcm.dummy+xml',
#				  	  '-H','Authorization: Basic U1VQUE9SVDpTVVBQT1JU',
#					  '-G',
#					  'http://127.0.0.1:8080/plcm/mcu/api/1.0/system/info/state'],
#					  stdout=subprocess.PIPE, 
#					  stderr=subprocess.PIPE)
#	out, err = p.communicate()
#	print out
#	q=subprocess.Popen(['ls','-l','/mcms/Restore'],stdout=subprocess.PIPE,stderr=subprocess.PIPE)
#	qout,qerr=q.communicate()
#	print qout
#	match=re.search('^.*in_progress.*$',out)
#	if match:
#		print "."
#		time.sleep(5)
#	else:
#		FINISH=1
#
#curl -s -k -H "$PARM" -H "$AUTH" -X POST https://$MYIP/plcm/mcu/api/1.0/system/restore/finish/`basename ${Restore_File}`
#p = subprocess.Popen(['curl','-k','-H','If-None-Match: -1',
#				  '-H','Content-Type: application/vnd.plcm.dummy+xml',
#			  	  '-H','Authorization: Basic U1VQUE9SVDpTVVBQT1JU',
#				  '-X','POST',
#				  'http://127.0.0.1:8080/plcm/mcu/api/1.0/system/restore/finish/' + filename],
#				  stdout=subprocess.PIPE, 
#				  stderr=subprocess.PIPE)
#out, err = p.communicate()
#print out
#match=re.search('^.*Status OK.*$',out)
#if match:
#	print "SUCCESS"
#else:
#	print "ERROR"
#	sys.exit(1)
#

File=open('/mcms/ProductType','w')
File.write(str(P_Type[0]))
File.close()

c.Disconnect()
