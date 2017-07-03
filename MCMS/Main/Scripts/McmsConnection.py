#!/mcms/python/bin/python
# -*- coding: utf-8 -*-

import sys
import os
import httplib, urllib
import xml.dom.minidom
from time import *
import socket
import string
import subprocess
#from datetime import date
from datetime import *

from xml.dom.minidom import parse, parseString, Document

reload(sys) # Python2.5 初始化后会删除 sys.setdefaultencoding 这个方法，我们需要重新载入
sys.setdefaultencoding('utf-8')

#The following redirects error stream to the standard stream,
#to allow capture of interpreter errors in script logs
sys.stderr = sys.stdout

availableLayoutTypes = "1x1", "1x2", "2x1","2x2","1and5","3x3","1x2Ver","1x2Hor","1and2Hor","1and2Ver","1and3Hor","1and3Ver","1and4Ver","1and4Hor","1and8Central","1and8Upper","1and2HorUpper", "1and3HorUpper","1and4HorUpper","1and8Lower", "1and7","4x4","2and8","1and12"

#-----------------------
# Global Functions
#-----------------------
def ScriptAbort(errMsg):
    print "Error: " + errMsg
    sys.exit(errMsg)

def RunCommand(command):
    ps     = subprocess.Popen(command , shell=True, stdout=subprocess.PIPE)
    output = ps.stdout.read()
    ps.stdout.close()
    ps.wait()
    return output
#-----------------------


class McmsHttpConnection(httplib.HTTPConnection):
    def SetTimeOut(self,timeout):
        self.sock.settimeout(timeout)

    #def CreateConnection(self,ip="127.0.0.1",port=8080):
    def CreateConnection(self,ip="127.0.0.1",port=80):
        self.connect()

class McmsHttpsConnection(httplib.HTTPSConnection):
    def SetTimeOut(self,timeout):
        self.sock.settimeout(timeout)

    def CreateConnection(self,ip="127.0.0.1",port=443):
        self.connect()


class McmsConnection:
    """This is a utility class.

    It enables all sorts of actions in front of mcms.
    """
    last_status = None
    xmlResponse = None
    connection = None
    loadedXml = None
    mcutoken = "-1"
    usertoken = "-1"
    ip = None
    port = None
    user = "POLYCOM"
    password = "POLYCOM"
    #pass = "POLYCOM"
    headers = {"Content-Type": "text/xml; charset=UTF-8",
               "Server": "PolycomHTTPServer",
               "Connection": "Keep-Alive",
               "Cache-control": "private"}
    headers_put = {"Content-Type": "text/xml; charset=UTF-8","Server": "PolycomHTTPServer","Connection": "Keep-Alive", 
                   "Cache-control": "private","Authorization": "Basic U1VQUE9SVDpTVVBQT1JU"}
#------------------------------------------------------

    #def CreateConnection(self,user="POLYCOM",password="POLYCOM", ip="127.0.0.1",port=8080):
    def CreateConnection(self,user="POLYCOM",password="POLYCOM", ip="127.0.0.1",port=80):
	if ip == "127.0.0.1":
           try:
              ip=os.environ["TARGET_IP"]
           except KeyError:
              ip=RunCommand("echo -n `/sbin/ifconfig | grep 'inet add' | grep -v 127.0.0.1 | tr -s ' ' | cut -d':' -f2 | cut -d' ' -f1`")
	      print ip

        #if port == 8080:
        if port == 80:
           try:
              port=os.environ["TARGET_PORT"]
           except KeyError:
              #port="8080"
              port="80"

        self.ip = ip
        self.port = string.atoi(port)
        self.user = user
        self.password = password

        if self.port == 8080 or self.port == 80:
            print "Connection is not secured"
            self.connection = McmsHttpConnection(ip,self.port)
        else:
            print "Connection is secured"
            self.connection = McmsHttpsConnection(ip,self.port)
        self.connection.CreateConnection(ip, self.port)
#------------------------------------------------------

    #def Connect(self,user="POLYCOM",password="POLYCOM", ip="127.0.0.1",port=8080,expected_statuses="Status OK"):
    def Connect(self,user="POLYCOM",password="POLYCOM", ip="127.0.0.1",port=80,expected_statuses="Status OK"):

	if ip == "127.0.0.1":
           try:
              ip=os.environ["TARGET_IP"]
           except KeyError:
              ip=RunCommand("echo -n `/sbin/ifconfig | grep 'inet add' | grep -v 127.0.0.1 | tr -s ' ' | cut -d':' -f2 | cut -d' ' -f1`")
	      print ip
        #if port == 8080:
        #if port == 80:
        try:
             str_port=os.environ["TARGET_PORT"]
             port = string.atoi(str_port)
        except KeyError:
               port=port
              #port="8080"
              #port=80

        self.ip = ip
        self.port = port
        self.user = user
        self.password = password

	print "Here "+self.user+" "+self.password+" "+str(port)

	try:
	    loginFileName=os.environ["LOGIN_FILE_NAME"]
	except KeyError:
            loginFileName="Scripts/login.xml"

        logindom = parse(loginFileName)

        if self.port == 8080 or self.port == 80:
            print "Connection is not secured"
            self.connection = McmsHttpConnection(ip,self.port)
        else:
            print "Connection is secured"
            self.connection = McmsHttpsConnection(ip,self.port)
        logindom.getElementsByTagName("IP")[0].firstChild.data = ip
        logindom.getElementsByTagName("LISTEN_PORT")[0].firstChild.data = self.port
        logindom.getElementsByTagName("USER_NAME")[0].firstChild.data = self.user
        logindom.getElementsByTagName("PASSWORD")[0].firstChild.data = self.password

        seconds = 100
        for retry in range(seconds + 1):
            try:
                self.SendXml(logindom,expected_statuses,100)
                break
            except:
                if retry == seconds:
                    print "Test stop, cannot login!"
                    raise
                else:
                    sleep(1)
                    print "Send xml abort, continue try!"
        self.connection.SetTimeOut(60) # set timeout for each transaction
        self.mcutoken = self.GetTextUnder("LOGIN","MCU_TOKEN")
        self.usertoken = self.GetTextUnder("LOGIN","MCU_USER_TOKEN")
        self.product_type = self.GetTextUnder("LOGIN","PRODUCT_TYPE")

#------------------------------------------------------
    #def ProxyLogin(self,ip="127.0.0.1",port=8080):
    def ProxyLogin(self,ip="127.0.0.1",port=80):

	print "===ProxyLogin==="

        try:
            ip=os.environ["TARGET_IP"]
        except KeyError:
            ip=RunCommand("echo -n `/sbin/ifconfig | grep 'inet add' | grep -v 127.0.0.1 | tr -s ' ' | cut -d':' -f2 | cut -d' ' -f1`")
            print ip

        try:
            port=os.environ["TARGET_PORT"]
        except KeyError:
            #port="8080"
            port="80"

        self.ip = ip
        self.port = string.atoi(port)
        self.user = "POLYCOM"
        self.password = "POLYCOM"

        logindom = parse('Scripts/login.xml')
	logindom.getElementsByTagName("IP")[0].firstChild.data = ip

        if self.port == 8080 or self.port == 80:
            print "Connection is not secured"
            self.connection = McmsHttpConnection(ip,self.port)
        else:
            print "Connection is secured"
            self.connection = McmsHttpsConnection(ip,self.port)

	pragma = "MESSAGE_ID=10;TRANSACTION=TRANS_MCU;ACTION=LOGIN;STATION_NAME=f6-jshuva-dt;USER_NAME=SUPPORT;PASSWORD=SUPPORT"

	print "sent login request to shm"
	self.SendXmlProxy(logindom, "Status OK", "SHM/",pragma)

	print "create conference on another RMX"
	self.SimpleNewConfTestWithoutTerminate("ConfSHM", # conference name
                  'Scripts/AddVideoCpConfTemplate.xml', # conf template
                  1, # timeout
                  "SHM/")

	print "send logout request to shm"
	logoutdom = parse('Scripts/Logout.xml')
	pragma = "MESSAGE_ID=10;TRANSACTION=TRANS_MCU;ACTION=LOGOUT"

	self.SendXmlProxy(logoutdom, "Status OK", "SHM/",pragma)

#------------------------------------------------------
    def SendXmlAction(self,xmlActionString,expected_statuses="STATUS_OK",encoding="",python_encoding=""):

        if python_encoding == "":
            if encoding != "":
                python_encoding = encoding
            else:
                python_encoding = "UTF-8"

        encoding_string = ""
        if encoding != "":
            encoding_string = " encoding=\"" + encoding + "\""

        transString = ""
        transString += "<?xml version=\"1.0\""+encoding_string+"?>\n"
        transString += "<TRANS_MCU>\n<TRANS_COMMON_PARAMS>\n<MCU_TOKEN>" + self.mcutoken
        transString += "</MCU_TOKEN>\n<MCU_USER_TOKEN>" + self.usertoken + "</MCU_USER_TOKEN>\n"
        transString += "<ASYNC><YOUR_TOKEN1>0</YOUR_TOKEN1><YOUR_TOKEN2>0</YOUR_TOKEN2></ASYNC>\n"
        transString += "<MESSAGE_ID>0</MESSAGE_ID>\n</TRANS_COMMON_PARAMS>\n"

        #totalString = ""
        totalString = transString.encode(python_encoding)
        totalString +=  xmlActionString
        totalString += "\n</TRANS_MCU>".encode(python_encoding)

        charSet = "UTF-8"
        if(encoding != ""):
            charSet = encoding
        strContentType = "text/xml; charset=" + charSet
        localhttpHeaders = {"Content-Type": strContentType,
                            "Server": "PolycomHTTPServer",
                            "Connection": "Keep-Alive",
                            "Cache-control": "private"}

        seconds = 10
        for retry in range(seconds+1):
            try:
                #if self.port == 8080:
                if self.port == 80:
                    self.connection.request("POST", "http://", totalString, localhttpHeaders)
                else:
                    self.connection.request("POST", "https://", totalString, localhttpHeaders)
                break
            except socket.error ,(errno, strerror) :
                #if last retry failed
                if retry == seconds:
                    raise
                else:
                    sleep(2)
                    print "SendXml received error: " + strerror +" ,retrying to connect. "
                    #if self.port == 8080:
                    if self.port == 80:
                        self.connection = McmsHttpConnection(self.ip, self.port)
                    else:
                        self.connection = McmsHttpsConnection(self.ip, self.port)
                    continue
            except:
                raise



        response = self.connection.getresponse()

        if response.status != 200:
            ScriptAbort("post failed " + response.reason)

        data = response.read()
        self.xmlResponse = parseString(data.encode("utf-8"))
        self.last_status = self.xmlResponse.getElementsByTagName(
            "RETURN_STATUS")[0].getElementsByTagName("DESCRIPTION")[0].firstChild.data

        if expected_statuses != "":
            if  -1 == expected_statuses.find(self.last_status):
                print totalString
                print
                print self.xmlResponse.toprettyxml(encoding="utf-8")
                print
                print "*** SendXml - Expected: " + expected_statuses + ", got: "+self.last_status + " ***"
                ScriptAbort("SendXml - abort!")
                self.Disconnect()
            return self.last_status


 #------------------------------------------------------------------------------
    #def ConnectSpecificUser(self,user_name,user_password,expected_status="Status OK",ip="127.0.0.1",port=8080):
    def ConnectSpecificUser(self,user_name,user_password,expected_status="Status OK",ip="127.0.0.1",port=80):

        try:
            ip=os.environ["TARGET_IP"]
        except KeyError:
            ip=RunCommand("echo -n `/sbin/ifconfig | grep 'inet add' | grep -v 127.0.0.1 | tr -s ' ' | cut -d':' -f2 | cut -d' ' -f1`")
            print ip

        try:
            port=os.environ["TARGET_PORT"]
        except KeyError:
            #port="8080"
            port="80"

        self.ip = ip
        self.port = string.atoi(port)
        self.user = user_name
        self.password = user_password

       # logindom = parse('Scripts/login.xml')
        self.LoadXmlFile('Scripts/login.xml')

        #if self.port == 8080:
        if self.port == 80:
            self.connection = McmsHttpConnection(ip,self.port)
        else:
            self.connection = McmsHttpsConnection(ip,self.port)
        #logindom.getElementsByTagName("IP")[0].firstChild.data = ip
       # self.getElementsByTagName("IP")[0].firstChild.data = ip
        self.ModifyXml("LOGIN","IP",ip)
        self.ModifyXml("LOGIN","USER_NAME",user_name)
        self.ModifyXml("LOGIN","PASSWORD" ,user_password)
        #self.SendXml(logindom)
        #self.Send()
        self.Send(expected_status)

        #self.connection.SetTimeOut(60) # set timeout for each transaction
        self.mcutoken = self.GetTextUnder("LOGIN","MCU_TOKEN")
        self.usertoken = self.GetTextUnder("LOGIN","MCU_USER_TOKEN")

#------------------------------------------------------------------------------
    #def ConnectSpecificUserFederalFirstLogin(self,user_name,user_password,new_password,expected_status1="User must change password",expected_status2="Status OK",ip="127.0.0.1",port=8080):
    def ConnectSpecificUserFederalFirstLogin(self,user_name,user_password,new_password,expected_status1="User must change password",expected_status2="Status OK",ip="127.0.0.1",port=80):

        try:
            ip=os.environ["TARGET_IP"]
        except KeyError:
            ip=RunCommand("echo -n `/sbin/ifconfig | grep 'inet add' | grep -v 127.0.0.1 | tr -s ' ' | cut -d':' -f2 | cut -d' ' -f1`")
            print ip

        try:
            port=os.environ["TARGET_PORT"]
        except KeyError:
            #port="8080"
            port="80"

        self.ip = ip
        self.port = string.atoi(port)
        self.user = user_name
        self.password = user_password

       # logindom = parse('Scripts/login.xml')
        self.LoadXmlFile('Scripts/login.xml')

        #if self.port == 8080:
        if self.port == 80:
            self.connection = McmsHttpConnection(ip,self.port)
        else:
            self.connection = McmsHttpsConnection(ip,self.port)
        #logindom.getElementsByTagName("IP")[0].firstChild.data = ip
       # self.getElementsByTagName("IP")[0].firstChild.data = ip
        self.ModifyXml("LOGIN","IP",ip)
        self.ModifyXml("LOGIN","USER_NAME",user_name)
        self.ModifyXml("LOGIN","PASSWORD" ,user_password)
        #self.SendXml(logindom)
        #self.Send()
        status = self.Send(expected_status1)
        if("User must change password" == status):
            #self.Sleep(60)
            print "First login, Changing password"

            self.LoadXmlFile('Scripts/FederalLogin.xml')
            self.ModifyXml("LOGIN","IP",ip)
            self.ModifyXml("LOGIN","USER_NAME",user_name)
            self.ModifyXml("LOGIN","PASSWORD" ,user_password)
            self.ModifyXml("LOGIN","NEW_PASSWORD" ,new_password)
            self.Send(expected_status2)
        else:
            print "Not First Login."

        #self.connection.SetTimeOut(60) # set timeout for each transaction
        self.mcutoken = self.GetTextUnder("LOGIN","MCU_TOKEN")
        self.usertoken = self.GetTextUnder("LOGIN","MCU_USER_TOKEN")

#------------------------------------------------------------------------------
    def Disconnect(self,send_logout="true"):
		if send_logout == "true":
			self.SendXmlFile('Scripts/Logout.xml')

		self.connection.close()

#------------------------------------------------------------------------------
    def LoadXmlFile(self,xmlFile):

        self.loadedXml = parse(xmlFile)

#------------------------------------------------------------------------------
    def Send(self,expected_status="Status OK",seconds=10):
        return self.SendXml(self.loadedXml,expected_status,seconds)
#------------------------------------------------------------------------------
    def ModifyXml(self,tag1,tag2,newtext):
        #self.loadedXml.getElementsByTagName(tag1)[0].getElementsByTagName(tag2)[0].firstChild.data = newtext
        elem1 = self.loadedXml.getElementsByTagName(tag1)
        if len(elem1) > 0:
        	if elem1[0].firstChild:
        		elem2 = elem1[0].getElementsByTagName(tag2)
        		if len(elem2) > 0:
        			if elem2[0].firstChild:
        				elem2[0].firstChild.data = newtext
        			else:
        				print "** Warning ModifyXml xml Node " + tag2 + " is empty value could not modify it *********"
        		else:
        			print "** Warning ModifyXml could not find xml node " + tag2 + " ***"
        else:
            print "** Warning ModifyXml could not find xml node " + tag1 + " ***"

#------------------------------------------------------------------------------
    def ModifyXml3(self,tag1,tag2,tag3,newtext):
    	elem = self.loadedXml.getElementsByTagName(tag1)[0].getElementsByTagName(tag2)[0].getElementsByTagName(tag3)[0]
    	if elem.firstChild:
        	elem.firstChild.data = newtext


#------------------------------------------------------------------------------
    def ModifyXmlList(self,tag1,tag2,i2,newtext):
        self.loadedXml.getElementsByTagName(tag1)[0].getElementsByTagName(tag2)[i2].firstChild.data = newtext
#------------------------------------------------------------------------------
    def AddXML(self, parentElementName, childTagName, childText = "",index=0):
        childElement = self.loadedXml.createElement(childTagName)
        if childText <> "":
            childTextElement = self.loadedXml.createTextNode(childText)
            childElement.appendChild(childTextElement)
        parentElement = self.loadedXml.getElementsByTagName(parentElementName)[index]
        parentElement.appendChild(childElement)
#------------------------------------------------------------------------------
    def SendXml(self,xmlRequest,expected_statuses="Status OK",seconds=10):

#added new support for expected_statuses, which can be more than one status splitted by ; delimiter
#This is required when the operation might be an immidate and Resposne Status Ok , or returns Status Inprogress , and afterwards retrieving the status with other transaction

	if  expected_statuses == "" :
		expected_statuses="Status OK"
	expected_stat_list = expected_statuses.split(';');
        self.loadedXml = xmlRequest
        self.ModifyXml("TRANS_COMMON_PARAMS","MCU_TOKEN",self.mcutoken)
        self.ModifyXml("TRANS_COMMON_PARAMS","MCU_USER_TOKEN",self.usertoken)

        for retry in range(seconds+1):
            try:

                send_utf8_string = self.loadedXml.toxml(encoding="utf-8")

                #if self.port == 8080:
                if self.port == 80:
                    self.connection.request("POST", "http://", send_utf8_string, self.headers)
                else:
                    self.connection.request("POST", "https://", send_utf8_string, self.headers)
                break
            except socket.error ,(errno, strerror) :
                #if last retry failed
                if retry == seconds:
                    raise
                else:
                    sleep(2)
                    print "SendXml received error: " + strerror +" ,retrying to connect. "
                    #if self.port == 8080:
                    if self.port == 80:
                        self.connection = McmsHttpConnection(self.ip, self.port)
                    else:
                        self.connection = McmsHttpsConnection(self.ip, self.port)
                    continue
            except:
                if retry == seconds:
                    raise
                else:
                    sleep(2)
                    print "SendXml received unknown exception: " + strerror +" ,retrying to connect. "
                    #if self.port == 8080:
                    if self.port == 80:
                        self.connection = McmsHttpConnection(self.ip, self.port)
                    else:
                        self.connection = McmsHttpsConnection(self.ip, self.port)
                    continue



        response = self.connection.getresponse()

        if response.status != 200:
            ScriptAbort("post failed " + response.reason)

        data = response.read()
        self.xmlResponse = parseString(data)
        self.last_status = self.xmlResponse.getElementsByTagName(
            "RETURN_STATUS")[0].getElementsByTagName("DESCRIPTION")[0].firstChild.data

	bstatusFound=False
        if expected_statuses != "":
		for expected in expected_stat_list:
	            if  -1 != expected.find(self.last_status):
			bstatusFound=True
			break

	if bstatusFound == False:
	    print self.loadedXml.toprettyxml(encoding="utf-8")
            print
            print self.xmlResponse.toprettyxml(encoding="utf-8")
            print
            print "*** SendXml - Expected: " + expected_statuses + ", got: "+self.last_status + " ***"
            ScriptAbort("SendXml - abort!")
            self.Disconnect()

        return self.last_status

#------------------------------------------------------------------------------
    def SendXmlProxy(self,xmlRequest,expected_statuses="Status OK",urlpath="",pragma=""):
        for retry in range(seconds+1):
            try:

                send_utf8_string = self.loadedXml.toxml(encoding="utf-8")

                #if self.port == 8080:
                if self.port == 80:
                    self.connection.request("POST", "http://", send_utf8_string, self.headers)
                else:
                    self.connection.request("POST", "https://", send_utf8_string, self.headers)
                break
            except socket.error ,(errno, strerror) :
                #if last retry failed
                if retry == seconds:
                    raise
                else:
                    sleep(2)
                    print "SendXml received error: " + strerror +" ,retrying to connect. "
                    #if self.port == 8080:
                    if self.port == 80:
                        self.connection = McmsHttpConnection(self.ip, self.port)
                    else:
                        self.connection = McmsHttpsConnection(self.ip, self.port)
                    continue
            except:
                raise



        response = self.connection.getresponse()

        if response.status != 200:
            ScriptAbort("post failed " + response.reason)

        data = response.read()
        self.xmlResponse = parseString(data)
        self.last_status = self.xmlResponse.getElementsByTagName(
            "RETURN_STATUS")[0].getElementsByTagName("DESCRIPTION")[0].firstChild.data

        if expected_statuses != "":
            if  -1 == expected_statuses.find(self.last_status):
                print self.loadedXml.toprettyxml(encoding="utf-8")
                print
                print self.xmlResponse.toprettyxml(encoding="utf-8")
                print
                print "*** SendXml - Expected: " + expected_statuses + ", got: "+self.last_status + " ***"
                ScriptAbort("SendXml - abort!")
                self.Disconnect()
        return self.last_status
#------------------------------------------------------------------------------
    def SendSpecialXmlRequest(self, action, destIpAddress, destPort):

        self.connection = McmsHttpConnection(destIpAddress, destPort)
        seconds = 10

        for retry in range(seconds+1):
            try:
                self.connection.request(action, "http://", "", self.headers)

                response = self.connection.getresponse()
                return response

            except socket.error ,(errno, strerror) :
                if retry == seconds:
                    raise
                else:
                    sleep(2)
                    print "SendXml received error: " + strerror +" ,retrying to connect. "
                    self.connection = McmsHttpConnection(self.ip, self.port)
                    continue
            except:
                raise

        return None

#------------------------------------------------------------------------------
    def SendExpandedXml(self, httpHeaders, xmlRequest, expected_statuses = "STATUS_OK"):

        self.loadedXml = xmlRequest
        self.ModifyXml("TRANS_COMMON_PARAMS","MCU_TOKEN",self.mcutoken)
        self.ModifyXml("TRANS_COMMON_PARAMS","MCU_USER_TOKEN",self.usertoken)

        seconds = 10
        for retry in range(seconds+1):
            try:

                send_utf8_string = self.loadedXml.toxml(encoding="utf-8")

                #if self.port == 8080:
                if self.port == 80:
                    self.connection.request("POST", "http://", send_utf8_string, httpHeaders)
                else:
                    self.connection.request("POST", "https://", send_utf8_string, httpHeaders)
                break
            except socket.error ,(errno, strerror) :
                #if last retry failed
                if retry == seconds:
                    raise
                else:
                    sleep(2)
                    print "SendXml received error: " + strerror +" ,retrying to connect. "
                    #if self.port == 8080:
                    if self.port == 80:
                        self.connection = McmsHttpConnection(self.ip, self.port)
                    else:
                        self.connection = McmsHttpsConnection(self.ip, self.port)
                    continue
            except:
                raise



        response = self.connection.getresponse()

        if response.status != 200:
            ScriptAbort("post failed " + response.reason)


        data = response.read()
        self.xmlResponse = parseString(data)
        self.last_status = self.xmlResponse.getElementsByTagName(
            "RETURN_STATUS")[0].getElementsByTagName("DESCRIPTION")[0].firstChild.data

        if expected_statuses != "":
            if  -1 == expected_statuses.find(self.last_status):
                print self.loadedXml.toprettyxml(encoding="utf-8")
                print
                print self.xmlResponse.toprettyxml(encoding="utf-8")
                print
                print "*** SendXml - Expected: " + expected_statuses + ", got: "+self.last_status + " ***"
                ScriptAbort("SendXml - abort!")
                self.Disconnect()
        return self.last_status


#------------------------------------------------------------------------------
    def PostString(self,string,expected_statuses="Status OK"):

        seconds = 10

        for retry in range(seconds+1):
            try:
                #if self.port == 8080:
                if self.port == 80:
                    self.connection.request("POST", "http://", string, self.headers)
                else:
                    self.connection.request("POST", "https://", string, self.headers)
                break
            except socket.error ,(errno, strerror) :
                #if last retry failed
                if retry == seconds:
                    raise
                else:
                    sleep(2)
                    print "SendXml received error: " + strerror +" ,retrying to connect. "
                    #if self.port == 8080:
                    if self.port == 80:
                        self.connection = McmsHttpConnection(self.ip, self.port)
                    else:
                        self.connection = McmsHttpsConnection(self.ip, self.port)
                    continue
            except:
                raise



        response = self.connection.getresponse()

        if response.status != 200:
            sys.exit("post failed " + response.reason)

        data = response.read()
        self.xmlResponse = parseString(data.encode("utf-8"))
        self.last_status = self.xmlResponse.getElementsByTagName(
            "RETURN_STATUS")[0].getElementsByTagName("DESCRIPTION")[0].firstChild.data

        if expected_statuses != "":
            if  -1 == expected_statuses.find(self.last_status):
                print self.loadedXml.toprettyxml(encoding="utf-8")
                print
                print self.xmlResponse.toprettyxml(encoding="utf-8")
                print
                print "*** SendXml - Expected: " + expected_statuses + ", got: "+self.last_status + " ***"
                sys.exit("SendXml - abort!")
                self.Disconnect()
        return self.last_status


#------------------------------------------------------------------------------
    def Upload(self,string,location,expected_statuses="Status OK"):

        
 		seconds = 3
 		for retry in range(seconds+1):
 			try:
 				#if self.port == 8080:
 				if self.port == 80:
 					self.connection.request("PUT", "http://"+self.ip+location, string, self.headers_put)
 				else:
 					self.connection.request("PUT", "https://"+self.ip+location, string, self.headers_put)
				break
			except socket.error ,(errno, strerror) :
				if retry == seconds:
					raise
				else:
					sleep(1)
					print "Upload received error: " + strerror +" ,retrying to connect. "
					#if self.port == 8080:
					if self.port == 80:
						self.connection = McmsHttpConnection(self.ip, self.port)
					else:
						self.connection = McmsHttpsConnection(self.ip, self.port)
					continue
			except:
				raise

		response = self.connection.getresponse()
		data = response.read()

		if response.status != 200:
			ScriptAbort("PUT failed " + response.reason)

		return response.status

#------------------------------------------------------------------------------
    def SendXmlFile(self,xmlFile,expected_statuses="Status OK"):

        return self.SendXml(parse(xmlFile),expected_statuses)
#------------------------------------------------------------------------------

    def GetTextUnder(self,tag1,tag2,tag1Index=0,tag2Index=0):
        elmlist1 = self.xmlResponse.getElementsByTagName(tag1)
        if len(elmlist1) <= tag1Index:
            return ""
        elmlist2 = elmlist1[tag1Index].getElementsByTagName(tag2)
        if len(elmlist2) <= tag2Index:
            return ""

        if elmlist2[tag2Index].firstChild is not None:
            return elmlist2[tag2Index].firstChild.data
        else:
            print "GetTextUnder - firstChild of element " + elmlist2[tag2Index].tagName + " is none!"
            return ""
        connection
#------------------------------------------------------------------------------

    def GetTextUnderList(self,tag1,tag2,tag3,tag1Index=0,tag2Index=0,tag3Index=0):
        elmlist1 = self.xmlResponse.getElementsByTagName(tag1)
        if len(elmlist1) <= tag1Index:
            return ""
        elmlist2 = elmlist1[tag1Index].getElementsByTagName(tag2)
        if len(elmlist2) <= tag2Index:
            return ""
        elmlist3 = elmlist2[tag2Index].getElementsByTagName(tag3)
        if len(elmlist3) <= tag3Index:
            return ""
        return elmlist3[tag3Index].firstChild.data
        connection
#------------------------------------------------------------------------------
    def TestElement(self,tag1,tag2,required):
        found = self.GetTextUnder(tag1,tag2)
        if found != required:
            self.Disconnect()
            ScriptAbort(self.xmlResponse.toprettyxml(encoding="utf-8") + "expected: " + required
                     + " got: " + found + " under " + tag1 + " " + tag2)

#------------------------------------------------------------------------------
    def IsAmos(self):
        if self.product_type == "Rmx_4000":
            return True

        return False

#------------------------------------------------------------------------------
    def DisconnectParty(self,confId,partyId):
        """Disconnect a certain party from a certain conference.

        confId - the conference id in which the party is defined
        partyId - the id of the party to be disconnected
        """
        # disconnect participant
        print "Disconnecting Party"
        self.LoadXmlFile('Scripts/DisconnectIPParty.xml')
        self.ModifyXml("SET_CONNECT", "ID",confId)
        self.ModifyXml("SET_CONNECT", "PARTY_ID", partyId)
        self.Send()

#------------------------------------------------------------------------------
    def DisconnectPartyExpStatus(self,confId,partyId,expected_status="Status OK"):
        """Disconnect a certain party from a certain conference.

        confId - the conference id in which the party is defined
        partyId - the id of the party to be disconnected
        """
        # disconnect participant
        print "Disconnecting Party"
        self.LoadXmlFile('Scripts/DisconnectIPParty.xml')
        self.ModifyXml("SET_CONNECT", "ID",confId)
        self.ModifyXml("SET_CONNECT", "PARTY_ID", partyId)
        self.Send(expected_status)

#------------------------------------------------------------------------------
    def DeleteParty(self,confId,partyId,expected_status="Status OK"):
        """Delete a certain party from a certain conference.

        confId - the conference id in which the party is defined
        partyId - the id of the party to be deleted
        """
        self.LoadXmlFile('Scripts/DeleteVoipParty.xml')
        self.ModifyXml("DELETE_PARTY","ID",confId)
        self.ModifyXml("DELETE_PARTY","PARTY_ID",partyId)
        self.Send(expected_status)

#------------------------------------------------------------------------------
    def SetPartyCapset(self, partyName, capSetName):
        """Set party defined capset

        partyName - party name that its caps are changed
        capSetName - the capset name to be set for the simulation party
        """
        print "Set party defined Capset " + partyName + " " + capSetName
        self.LoadXmlFile("Scripts/SimSetPartyCapset.xml")
        self.ModifyXml("SET_PARTY_CAPSET","PARTY_NAME",partyName)
        self.ModifyXml("SET_PARTY_CAPSET","CAPSET_NAME",capSetName)
        self.Send()
        sleep(1)
#------------------------------------------------------------------------------
    def ReconnectParty(self, confId, state, partyId):
        """reconnect or disconnect dial out EP.

        confid - destination conference.
        connectCondition - true or false - reconnect or disconnect party
        partyId - the id of the party to be reconnect or disconnect
        """
        print "Reconnect confId " + str(confId) + " for partyId " + str(partyId) + " set connect state " + state
        self.LoadXmlFile("Scripts/ReconnectParty/TransSetConnect.xml")
        self.ModifyXml("SET_CONNECT","ID",confId)
        self.ModifyXml("SET_CONNECT","CONNECT",state)
        self.ModifyXml("SET_CONNECT","PARTY_ID",partyId)
        self.Send()
        sleep(1)

#------------------------------------------------------------------------------
    def DeleteConf(self,confId,exp_sts = "In progress"):
        """Delete a given conference.
        """
        print "Delete conference ID: "+confId
        self.LoadXmlFile('Scripts/DeleteVoipConf.xml')
        self.ModifyXml("TERMINATE_CONF","ID",confId)
        self.Send(exp_sts)

#------------------------------------------------------------------------------
    def CreateConf(self,confName, fileName='Scripts/AddCpConf.xml'):
        """Create a new conference.

        confName - conference name
        fileName - the xml file which will be used to define the conference.
        """
        print "Adding Conf " + confName + " ..."
        self.LoadXmlFile(fileName)
        self.ModifyXml("RESERVATION","NAME",confName)
        self.Send()

#------------------------------------------------------------------------------
    def CreateConfWithDisplayName(self, confName, fileName='Scripts/AddCpConf.xml'):
        """Create a new conference.

        confName - conference name
        fileName - the xml file which will be used to define the conference.
        """
        print "Adding Conf " + confName + " ..."
        self.LoadXmlFile(fileName)
        self.ModifyXml("RESERVATION","NAME",confName)
        self.ModifyXml("RESERVATION","DISPLAY_NAME",confName)
        self.Send()

#------------------------------------------------------------------------------
    def CreateConfAndSetCallRate(self,confName, fileName, CallRate):  
        """Create a new conference.
        
        confName - conference name
        fileName - the xml file which will be used to define the conference.
        """
        print "Adding Conf " + confName + " ..."
        self.LoadXmlFile(fileName)
        self.ModifyXml("RESERVATION","NAME",confName)
        self.ModifyXml("RESERVATION","DISPLAY_NAME",confName)
        CallRateInK = CallRate/10
        self.ModifyXml("RESERVATION","TRANSFER_RATE",CallRateInK)
        self.Send()

#------------------------------------------------------------------------------
    def CreateConfWithConfId(self,confName, confId, fileName='Scripts/AddCpConf.xml'):  
        """Create a new conference.
        
        confName - conference name
        fileName - the xml file which will be used to define the conference.
        """
        print "Adding Conf " + confName + " ..."
        self.LoadXmlFile(fileName)
        self.ModifyXml("RESERVATION","NAME",confName)
        self.ModifyXml("RESERVATION","DISPLAY_NAME",confName)
        self.ModifyXml("RESERVATION","ID",confId)
        self.Send()


#------------------------------------------------------------------------------
    def CreateConfNameAndDisplayName(self,confName,displayName, fileName='Scripts/AddCpConf.xml'):
        """Create a new conference with different name and display name.

        confName - conference name
	display name - the conference display name which can be unicode
        fileName - the xml file which will be used to define the conference.
        """
        print "Adding Conf name " + confName + " ..."
        print "Adding Conf display name " + displayName + " ..."
        self.LoadXmlFile(fileName)
        self.ModifyXml("RESERVATION","NAME",confName)
        self.ModifyXml("RESERVATION","DISPLAY_NAME",displayName.encode('utf-8'))
        self.Send()


#------------------------------------------------------------------------------
    def CreateConfFromProfile(self, confName, profileID, fileName='Scripts/AddCpConf.xml'):
        """Create a new conference, with data taken from a profile.

        confName - conference name
        profileID - a predefined profile used to create a new conference
        fileName - the xml file which will be used to define the conference.
        """
        print "Adding Conf " + confName + " ..."
        self.LoadXmlFile(fileName)
        self.ModifyXml("RESERVATION","DISPLAY_NAME",confName)
        self.ModifyXml("RESERVATION","NAME",confName)
        self.ModifyXml("RESERVATION","AD_HOC_PROFILE_ID",profileID)
        self.Send()
#------------------------------------------------------------------------------
    def CreateConfFromTemplate(self,confName,tmplId,profId,fileName):
        """Create a new conference, with data taken from template.

        confName - conference name
        templateId - a predefined template used to create a new conference
        fileName - the xml file which will be used to define the conference.
        """
        print "Adding Conf " + confName + " ..."
        self.LoadXmlFile(fileName)
        self.ModifyXml("RESERVATION","DISPLAY_NAME",confName)
        self.ModifyXml("RESERVATION","NAME",confName)
        self.ModifyXml("RESERVATION","ID",tmplId)
        self.ModifyXml("RESERVATION","AD_HOC_PROFILE_ID",profId)

        self.Send()

#------------------------------------------------------------------------------
    def AddVideoParty(self,confid, partyname, partyip, sip=False, videoAlgo="auto",videoBitRate="automatic",aliasName="NotGiven"):
        """Add a new party.

        confid - destination conference.
        partyname - party name.
        sip - added party is a SIP party (True) or H323 (False)
        """
        if(sip):
            print "Adding SIP Party..." + partyname
            self.LoadXmlFile('Scripts/SipAddVideoParty1.xml')
        else:
            print "Adding H323 Party..." + partyname
            self.LoadXmlFile('Scripts/AddVideoParty1.xml')
        #partyip =  "1.2.3.4"
        self.ModifyXml("PARTY","NAME",partyname)
        self.ModifyXml("PARTY","IP",partyip)
        self.ModifyXml("ADD_PARTY","ID",confid)
        self.ModifyXml("PARTY","VIDEO_PROTOCOL", videoAlgo)
        self.ModifyXml("PARTY","VIDEO_BIT_RATE", videoBitRate)
        if ( sip == False):
            if aliasName == "NotGiven":
                self.ModifyXml("ALIAS","NAME",partyname)
            else:
                self.ModifyXml("ALIAS","NAME",aliasName)
        self.Send()

#------------------------------------------------------------------------------
    def AddCascadeVideoParty(self,confid, partyname, partyip, destAlias, videoAlgo="auto",videoBitRate="automatic"):
        """Add a new party.
        
        confid - destination conference.
        partyname - party name.
        """
        print "Adding H323 Party..." + partyname
        self.LoadXmlFile('Scripts/AddVideoParty4.xml')

        self.ModifyXml("PARTY","NAME",partyname)
        self.ModifyXml("PARTY","IP",partyip)
        self.ModifyXml("ADD_PARTY","ID",confid)
        self.ModifyXml("PARTY","VIDEO_PROTOCOL", videoAlgo)
        self.ModifyXml("PARTY","VIDEO_BIT_RATE", videoBitRate)
        self.ModifyXml("ALIAS","NAME", destAlias)
        self.Send()   
        
#------------------------------------------------------------------------------
    def AddParty(self,confid, partyname, partyIp, partyFile,expected_status="Status OK"):
        """Add a new party.

        confid - destination conference.
        partyname - party name.
        partyIp - ip address for new party
        partyFile - xml file which will be used to define the party
        """
        print "Adding Party..." + partyname
        self.LoadXmlFile(partyFile)
        self.ModifyXml("PARTY","NAME",partyname)
        self.ModifyXml("PARTY","IP",partyIp)
        self.ModifyXml("ADD_PARTY","ID",confid)
        self.ModifyXml("ALIAS", "NAME",partyname)
        self.Send(expected_status)
#--------------------------------------------------------------------------------
    def AddPartyH323AudioOnly(self,confid, partyname, partyIp, partyFile,expected_status="Status OK"):
        """Add a new party.
        confid - destination conference.
        partyname - party name.
        partyIp - ip address for new party
        partyFile - xml file which will be used to define the party
        """
        print "Adding Party..." + partyname
        self.LoadXmlFile(partyFile)
        self.ModifyXml("PARTY","NAME",partyname)
        self.ModifyXml("PARTY","IP",partyIp)
        self.ModifyXml("ADD_PARTY","ID",confid)
        self.Send(expected_status)
#------------------------------------------------------------------------------
    def AddSIPParty(self,confid, partyname, partyIp, partySipAdd, partyFile):
        """Add a new SIP party.

        confid - destination conference.
        partyname - party name.
        partyIp - ip address for new party
        partyFile - xml file which will be used to define the party
        """
        print "Adding SIP Party..." + partyname
        self.LoadXmlFile(partyFile)
        self.ModifyXml("PARTY","NAME",partyname)
        self.ModifyXml3("PARTY", "ALIAS", "NAME",partyname)
        self.ModifyXml("PARTY","IP",partyIp)
        self.ModifyXml("ADD_PARTY","ID",confid)
        self.ModifyXml("PARTY","SIP_ADDRESS",partySipAdd)
        self.Send()

#-----------------------------------------------------------------------------
#------------------------------------------------------------------------------
    def GetPartyName(self,confid,partyid):
        """Monitor party in conference, and return it's name.

        confid - target conference id
        partyid- name of the party to monitor
        """
        self.LoadXmlFile('Scripts/TransConf2.xml')
        self.ModifyXml("GET","ID",confid)
        self.Send()
        ongoing_party_list = self.xmlResponse.getElementsByTagName("ONGOING_PARTY")

        if (len(ongoing_party_list) == 0):
            ScriptAbort("the party was not connected...")
        else:
            for x in range(len(ongoing_party_list)):
                if ((ongoing_party_list[x].getElementsByTagName("ID")[0].firstChild.data) == str(partyid)):
                    return ongoing_party_list[x].getElementsByTagName("NAME")[0].firstChild.data
        return "party is not found:"
#------------------------------------------------------------------------------
    def GetPartyAlias(self,confid,partyid):
        """Monitor party in conference, and return it's alias.

        confid - target conference id
        partyid- name of the party to monitor
        """
        self.LoadXmlFile('Scripts/TransConf2.xml')
        self.ModifyXml("GET","ID",confid)
        self.Send()
        ongoing_party_list = self.xmlResponse.getElementsByTagName("ONGOING_PARTY")

        if (len(ongoing_party_list) == 0):
            ScriptAbort("the party was not connected...")
        else:
            AliasStruct=ongoing_party_list[int(partyid)].getElementsByTagName("ALIAS")
            return AliasStruct[0].getElementsByTagName("NAME")[0].firstChild.data
#------------------------------------------------------------------------------
    def GetPartyInterface(self,confid,partyid):
        """Monitor party in conference, and return it's interface: h323 or sip.

        confid - target conference id
        partyid- name of the party to monitor
        """
        self.LoadXmlFile('Scripts/TransConf2.xml')
        self.ModifyXml("GET","ID",confid)
        self.Send()
        ongoing_party_list = self.xmlResponse.getElementsByTagName("ONGOING_PARTY")

        if (len(ongoing_party_list) == 0):
            ScriptAbort("the party was not connected...")
        else:
            return (ongoing_party_list[int(partyid)].getElementsByTagName("INTERFACE")[0].firstChild.data)
#------------------------------------------------------------------------------
    def GetPartyIdByAliasName(self,confid, partyAliasName):
        """Monitor party in conference, and return it's id.

        confid - target conference id
        partyname - name of the party to monitor
        """
        self.LoadXmlFile('Scripts/TransConf2.xml')
        self.ModifyXml("GET","ID",confid)
        self.Send()
        ongoing_party_list = self.xmlResponse.getElementsByTagName("ONGOING_PARTY")

        if (len(ongoing_party_list) == 0):
            ScriptAbort("the party was not connected...")
        else:
            for index in range(len(ongoing_party_list)):
                if self.GetTextUnder("PARTY","CONNECTION",index) == "dial_in":
                    AliasStruct=ongoing_party_list[index].getElementsByTagName("ALIAS")
                    if partyAliasName == AliasStruct[0].getElementsByTagName("NAME")[0].firstChild.data:
                        return ongoing_party_list[index].getElementsByTagName("ID")[0].firstChild.data
 #------------------------------------------------------------------------------
    def GetPartyId(self,confid, partyname):
        """Monitor party in conference, and return it's id.

        confid - target conference id
        partyname - name of the party to monitor
        """
        self.LoadXmlFile('Scripts/TransConf2.xml')
        self.ModifyXml("GET","ID",confid)
        self.Send()
        ongoing_party_list = self.xmlResponse.getElementsByTagName("ONGOING_PARTY")
        print "the num of ongoing parties are: " + str(len(ongoing_party_list))

        if (len(ongoing_party_list) == 0):
            ScriptAbort("the party was not connected...")
        else:
            for index in range(len(ongoing_party_list)):
                if(partyname == ongoing_party_list[index].getElementsByTagName("NAME")[0].firstChild.data):
                    partyid = ongoing_party_list[index].getElementsByTagName("ID")[0].firstChild.data
                    return partyid

 #------------------------------------------------------------------------------
    def GetPartyIDs(self,confid):
        """Monitor parties in conference, and return their IDs.

        confid - target conference id
        """
        self.LoadXmlFile('Scripts/TransConf2.xml')
        self.ModifyXml("GET","ID",confid)
        self.Send()
        ongoing_party_list = self.xmlResponse.getElementsByTagName("ONGOING_PARTY")
        print "the num of ongoing parties are: " + str(len(ongoing_party_list))

        if (len(ongoing_party_list) == 0):
            ScriptAbort("the party was not connected...")
        else:
            listIDs = list()
            for index in range(len(ongoing_party_list)):
                partyid = ongoing_party_list[index].getElementsByTagName("ID")[0].firstChild.data
                listIDs.append(partyid)
            print "Party IDs: " + str(listIDs)
            return listIDs
#------------------------------------------------------------------------------
    def GetPartyIDsAndNames(self,confid):
        """Monitor parties in conference, and return their IDs and Names.

        confid - target conference id
        """
        self.LoadXmlFile('Scripts/TransConf2.xml')
        self.ModifyXml("GET","ID",confid)
        self.Send()
        ongoing_party_list = self.xmlResponse.getElementsByTagName("ONGOING_PARTY")
        print "the num of ongoing parties are: " + str(len(ongoing_party_list))

        if (len(ongoing_party_list) == 0):
            ScriptAbort("the party was not connected...")
        else:
            listIDs = list()
            listNames = list()
            for index in range(len(ongoing_party_list)):
                partyid = ongoing_party_list[index].getElementsByTagName("ID")[0].firstChild.data
                partyName = ongoing_party_list[index].getElementsByTagName("NAME")[0].firstChild.data

                listIDs.append(partyid)
                listNames.append(partyName)

            print "Party IDs: " + str(listIDs)
            return listIDs,listNames
 #------------------------------------------------------------------------------
    def GetCurrPartyID(self, connection,confid,listIndex,num_retries):
        print "Getting the new Undefined party id ...",
        connection.LoadXmlFile('Scripts/SpeakerChange/TransConf2.xml')
        connection.ModifyXml("GET","ID",confid)
        for retry in range(num_retries+1):
            connection.Send()
            ongoing_parties = connection.xmlResponse.getElementsByTagName("ONGOING_PARTY")
            num_ongoing_parties = len(ongoing_parties)
            if (listIndex+1) == num_ongoing_parties: # new party were added
                partyTmpId= ongoing_parties[listIndex].getElementsByTagName("ID")[0].firstChild.data
                if partyTmpId != "":
                    print
                    return string.atoi(partyTmpId)
                else:
                    connection.Disconnect()
                    ScriptAbort("Error:Can not find partry id of party index: "+str(listIndex))
            if (retry == num_retries):
                connection.Disconnect()
                ScriptAbort("party index "+str(listIndex) + " Did not Connected!!!")
            sys.stdout.write(".")
            sys.stdout.flush()
            sleep(1)

        print
        return -1

 #------------------------------------------------------------------------------
    def ChangeConfLayoutType(self, confid, newConfLayout):
        print "Conference ID: "+ confid + " Changing Layout Type To: " + newConfLayout
        self.LoadXmlFile('Scripts/ConfVideoLayout/ChangConfLayout.xml')
        self.ModifyXml("SET_VIDEO_LAYOUT","ID",confid)
        self.ModifyXml("FORCE","LAYOUT",newConfLayout)
        self.Send()

#------------------------------------------------------------------------------
    def ChangeConfLayoutTypeExpStatus(self, confid, newConfLayout,expected_status="Status OK"):
        print "Conference ID: "+ confid + " Changing Layout Type To: " + newConfLayout
        self.LoadXmlFile('Scripts/ConfVideoLayout/ChangConfLayout.xml')
        self.ModifyXml("SET_VIDEO_LAYOUT","ID",confid)
        self.ModifyXml("FORCE","LAYOUT",newConfLayout)
        self.Send(expected_status)

#------------------------------------------------------------------------------
    def WaitConfCreated(self,confName,retires = 20):
        """Monitor conferences list until 'confName' is found.
        Returns conference ID.

        confName - lookup conf name
        """
        print "Wait untill Conf \'" + confName + "\' is created...",
        bFound = False
        for retry in range(retires+1):
            status = self.SendXmlFile('Scripts/TransConfList.xml',"Status OK")
            ongoing_conf_list = self.xmlResponse.getElementsByTagName("CONF_SUMMARY")
            for index in range(len(ongoing_conf_list)):
                if(confName == ongoing_conf_list[index].getElementsByTagName("NAME")[0].firstChild.data):
                    confid = ongoing_conf_list[index].getElementsByTagName("ID")[0].firstChild.data
                    if confid != "":
                        print
                        bFound = True
                        break
            if(bFound):
                break
            if (retry == retires):
                print self.xmlResponse.toprettyxml(encoding="utf-8")
                self.Disconnect()
                ScriptAbort("Can not monitor conf:" + status)
            sys.stdout.write(".")
            sys.stdout.flush()
            sleep(1)
        print "Created Conf, ID:" + str(confid)
        return confid

#------------------------------------------------------------------------------
    def WaitConfCreatedByNamePrefix(self,confNamePrefix,retires = 20):
        """Monitor conferences list until 'confName' is found.
        Returns conference ID.

        confNamePrefix - lookup conf name that start with confNamePrefix
        """
        print "Wait untill Conf \'" + confNamePrefix + "...\' is created...",
        bFound = False
        for retry in range(retires+1):
            status = self.SendXmlFile('Scripts/TransConfList.xml',"Status OK")
            ongoing_conf_list = self.xmlResponse.getElementsByTagName("CONF_SUMMARY")
            for index in range(len(ongoing_conf_list)):
                if(ongoing_conf_list[index].getElementsByTagName("NAME")[0].firstChild.data.startswith(confNamePrefix)):
                    confid = ongoing_conf_list[index].getElementsByTagName("ID")[0].firstChild.data
                    if confid != "":
                        print
                        bFound = True
                        break
            if(bFound):
                break
            if (retry == retires):
                print self.xmlResponse.toprettyxml(encoding="utf-8")
                self.Disconnect()
                ScriptAbort("Can not monitor conf:" + status)
            sys.stdout.write(".")
            sys.stdout.flush()
            sleep(1)
        print "Created Conf, ID:" + str(confid)
        return confid

#------------------------------------------------------------------------------
    def WaitEqConfCreated(self,confName,retires = 20):
        print "Wait untill Eq \'" + confName + "\' is created...",
        ongoingConf = ''
        bFound = False
        for retry in range(retires+1):
            status = self.SendXmlFile('Scripts/TransConfList.xml',"Status OK")
            ongoing_conf_list = self.xmlResponse.getElementsByTagName("CONF_SUMMARY")
            for index in range(len(ongoing_conf_list)):
                ongoingConf = ongoing_conf_list[index].getElementsByTagName("NAME")[0].firstChild.data
                confid = ongoing_conf_list[index].getElementsByTagName("ID")[0].firstChild.data
                onGoingTmpName = confName+ '(' + str(confid)+ ')'
                if ( onGoingTmpName == ongoingConf):
                    #if (confName == ongoingConf[:len(confName)]):
                    print
                    bFound = True
                    break
            if(bFound):
                break
            if (retry == retires):
                print self.xmlResponse.toprettyxml(encoding="utf-8")
                self.Disconnect()
                ScriptAbort("Can not monitor conf:" + status)
            sys.stdout.write(".")
            sys.stdout.flush()
            sleep(1)

        print "Created EQ ongoing Conf: " + ongoingConf + " , with ID:" + str(confid)
        return confid,ongoingConf
#------------------------------------------------------------------------------
    def WaitUntillEQorMRAwakes(self,eqName, num_of_parties,num_retries,isEQ=False):
        """Check that EQ or MR is in on-going list, then verify it has all parties defined and connected.

        eqName - Eq name
        num_of_parties - number of parties to be added to EQ.
        """

        eqOnGoingId = ""
        print "Wait untill EQ Awakes...",
        if isEQ:
            eqOnGoingId,eqName  = self.WaitEqConfCreated(eqName,num_retries)
        else:
            eqOnGoingId = self.WaitConfCreated(eqName,num_retries)
        print "Eq " + eqName + " with id-" +eqOnGoingId+ " is Awake..."

        self.WaitAllPartiesWereAdded(eqOnGoingId,num_of_parties,num_retries*num_of_parties)
        self.WaitAllOngoingConnected(eqOnGoingId)
        return eqOnGoingId

#------------------------------------------------------------------------------
    def WaitMRCreated(self,mrName,retires = 20):
        """Monitor meeting rooms list until 'mrName' is found.
        Maybe used also to find EQ.
        Returns MR ID, and MR NID.

        mrName - lookup MR name
        """
        print "Wait untill Conf \'" + mrName + "\' is created...",
        bFound = False
        for retry in range(retires+1):
            status = self.SendXmlFile('Scripts/GetMRList.xml',"Status OK")
            mr_list = self.xmlResponse.getElementsByTagName("MEETING_ROOM_SUMMARY")
            for index in range(len(mr_list)):
                if(mrName == mr_list[index].getElementsByTagName("NAME")[0].firstChild.data):
                    mrId = mr_list[index].getElementsByTagName("ID")[0].firstChild.data
                    mrNumericId = mr_list[index].getElementsByTagName("NUMERIC_ID")[0].firstChild.data
                    if mrId != "":
                        print
                        bFound = True
                        break
            if(bFound):
                break
            if (retry == retires):
                print self.xmlResponse.toprettyxml(encoding="utf-8")
                self.Disconnect()
                ScriptAbort("Can not monitor MR:" + status)
            sys.stdout.write(".")
            sys.stdout.flush()
        print "Created MR, ID:" + str(mrId) + ", NID:" + str(mrNumericId)
        return mrId, mrNumericId
#------------------------------------------------------------------------------
    def WaitResCreated(self,resName,retires = 20):
        """Monitor Reservation  list until 'resName' is found.
        Returns Res ID, and Res NID.

        ResName - lookup Res name
        """
        print "Wait untill Conf \'" + resName + "\' is reserved...",
        bFound = False
        for retry in range(retires+1):
            status = self.SendXmlFile('Scripts/AddRemoveReservation/TransResList.xml',"Status OK")
            res_list = self.xmlResponse.getElementsByTagName("RES_SUMMARY")
            for index in range(len(res_list)):
                print "\n" + "Res Name:" + res_list[index].getElementsByTagName("NAME")[0].firstChild.data
                if(resName == res_list[index].getElementsByTagName("NAME")[0].firstChild.data):
                    resId = res_list[index].getElementsByTagName("ID")[0].firstChild.data
                    resNumericId = res_list[index].getElementsByTagName("NUMERIC_ID")[0].firstChild.data
                    if resId != "":
                        print
                        bFound = True
                        break
            if(bFound):
                break
            if (retry == retires):
                print self.xmlResponse.toprettyxml(encoding="utf-8")
                self.Disconnect()
                ScriptAbort("Can not monitor Res:" + status)
            sys.stdout.write(".")
            sys.stdout.flush()
        print "Created Res, ID:" + str(resId) + ", NID:" + str(resNumericId)
        return resId, resNumericId

#------------------------------------------------------------------------------
    def WaitConfTemplateCreated(self,ConfTemplateName,retires = 50):
        """Monitor Conf Template  list until 'ConfTemplateName' is found.
        Returns ConfTemplate ID.

        ConfTemplateName - lookup Conf Template name
        """
        print "Wait untill Conf Template \'" + ConfTemplateName + "\' is created...",
        bFound = False
        for retry in range(retires+1):
            status = self.SendXmlFile('Scripts/AddRemoveConfTemplate/TransConfTemplateList.xml',"Status OK")
            conf_template_list = self.xmlResponse.getElementsByTagName("CONFERENCE_TEMPLATE_SUMMARY")
            print " Number of Elements in List " + str(len(conf_template_list))
            for index in range(len(conf_template_list)):
                print "\n" + "Conf Template Name:" + conf_template_list[index].getElementsByTagName("DISPLAY_NAME")[0].firstChild.data
                if(ConfTemplateName == conf_template_list[index].getElementsByTagName("DISPLAY_NAME")[0].firstChild.data):
                    confTemplateId = conf_template_list[index].getElementsByTagName("ID")[0].firstChild.data
                    if confTemplateId != "":
                        print
                        bFound = True
                        break
            if(bFound):
                break
            if (retry == retires):
                print self.xmlResponse.toprettyxml(encoding="utf-8")
                self.Disconnect()
                ScriptAbort("Can not monitor Conf Template:" + status)
            sys.stdout.write(".")
            sys.stdout.flush()
        print "Created Conf Template, ID:" + str(confTemplateId) + ", Name:" + str(ConfTemplateName)
        return  confTemplateId

#------------------------------------------------------------------------------
    def WaitConfEnd(self,confId,retires = 20):
        """Monitor conference untill it will b e deleted
        """

        print "Waiting until conference id: " + str(confId) + " will be deleted ",
        for retry in range(retires+1):
            stillOnAir=False
            sys.stdout.write(".")
            sys.stdout.flush()
            self.SendXmlFile('Scripts/TransConfList.xml',"Status OK")
            conf_list = self.xmlResponse.getElementsByTagName("CONF_SUMMARY")
            for conf in conf_list:
                conf_id = conf.getElementsByTagName("ID")[0].firstChild.data
                if confId == conf_id:
                    stillOnAir=True
                    break
            if  stillOnAir == False:
                print
                break
            if retry == retires:
                self.Disconnect()
                ScriptAbort("Failed delete conference!!!")
            sleep(1)

#------------------------------------------------------------------------------
    def WaitAllConfEnd(self,retires = 20):
        """Monitor conferences list until empty
        """
        print "Waiting until all conferences end",
        for retry in range(retires+1):
            sys.stdout.write(".")
            sys.stdout.flush()
            self.SendXmlFile('Scripts/TransConfList.xml',"Status OK")
            if self.GetTextUnder("CONF_SUMMARY","ID") == "":
                print
                break
            if retry == retires:
                self.Disconnect()
                ScriptAbort("Failed delete conference!!!")
            sleep(1)
#------------------------------------------------------------------------------
    def WaitAllOngoingDialOutConnected(self,confid,num_retries=30,delayBetweenParticipants=1):
        """Monitor conference until all ongoing parties are connected.
        """
        print "Wait until all ongoing dial out parties connect",
        self.LoadXmlFile('Scripts/TransConf2.xml')
        self.ModifyXml("GET","ID",confid)
        self.Send()
        last_num_connected = 0
        num_dial_out_parties = 0
        for retry in range(num_retries+1):
            ongoing_parties = self.xmlResponse.getElementsByTagName("ONGOING_PARTY")
            num_ongoing_parties = len(ongoing_parties)
            num_dial_out_connected = 0
            for index in range (num_ongoing_parties):
            	party_type = self.GetTextUnder("PARTY","CONNECTION",index)
            	if party_type != "dial_out":
            		num_dial_out_parties = num_dial_out_parties+1

            for index in range (num_ongoing_parties):
            	party_type = self.GetTextUnder("PARTY","CONNECTION",index)
            	if (party_type != "dial_out"):
            		continue
            	status = self.GetTextUnder("ONGOING_PARTY_STATUS","DESCRIPTION",index)
                print "index is: " + str(index) + " type: " + party_type + " status is: " + status
                if status != "connected":
                    if (retry == num_retries):
                        print self.xmlResponse.toprettyxml(encoding="utf-8")
                        self.Disconnect()
                        ScriptAbort("dial out party not connected :" + status)

                else:
                    num_dial_out_connected=num_dial_out_connected+1;
            sys.stdout.write(".")
            if last_num_connected != num_dial_out_connected:
                con_string =  "["+str(num_dial_out_connected)+"/"+str(num_dial_out_parties)+"]"
                sys.stdout.write(con_string)
            sys.stdout.flush()
            last_num_connected = num_dial_out_connected
            if num_dial_out_connected == num_dial_out_parties:
                print
                break
            sleep(delayBetweenParticipants)
            self.Send()
#------------------------------------------------------------------------------
    def WaitAllOngoingConnected(self,confid,num_retries=30,delayBetweenParticipants=1):
        """Monitor conference until all ongoing parties are connected.
        """
        print "Wait until all ongoing parties connect",
        self.LoadXmlFile('Scripts/TransConf2.xml')
        self.ModifyXml("GET","ID",confid)
        self.Send()
        last_num_connected = 0
        for retry in range(num_retries+1):
            ongoing_parties = self.xmlResponse.getElementsByTagName("ONGOING_PARTY_STATUS")
            num_ongoing_parties = len(ongoing_parties)
            num_connected = 0
            ongoing_parties = self.xmlResponse.getElementsByTagName("ONGOING_PARTY_STATUS")
            for party in ongoing_parties:
                status = party.getElementsByTagName("DESCRIPTION")[0].firstChild.data
                if status != "connected":
                    if (retry == num_retries):
                        print self.xmlResponse.toprettyxml(encoding="utf-8")
                        self.Disconnect()
                        ScriptAbort("party not connected :" + status)

                else:
                    num_connected=num_connected+1;
            sys.stdout.write(".")
            if last_num_connected != num_connected:
                con_string =  "["+str(num_connected)+"/"+str(len(ongoing_parties))+"]"
                sys.stdout.write(con_string)
            sys.stdout.flush()
            last_num_connected = num_connected
            if num_connected == len(ongoing_parties):
                print
                break
            sleep(delayBetweenParticipants)
            self.Send()
#------------------------------------------------------------------------------
    def ReconnectDisconnectedParties(self, confid):
        print "Reconnecting disconnected parties for conference: " + confid
        self.LoadXmlFile('Scripts/TransConf2.xml')
        self.ModifyXml("GET","ID",confid)
        self.Send()

        ongoing_parties = self.xmlResponse.getElementsByTagName("ONGOING_PARTY_STATUS")
        ongoing_party_list = self.xmlResponse.getElementsByTagName("ONGOING_PARTY")
        i = 0;
        for party in ongoing_parties:
            status = party.getElementsByTagName("DESCRIPTION")[0].firstChild.data
            if (status == "disconnected") :
	            partyId = ongoing_party_list[i].getElementsByTagName("ID")[0].firstChild.data
	            print "Reconnecting party " + str(partyId) + " that had status " + party.getElementsByTagName("DESCRIPTION")[0].firstChild.data
	            self.ReconnectParty(confid ,"true",partyId)
	            self.Send()
	            sleep(1)
            i=i+1
#------------------------------------------------------------------------------
    def ConnectDisconnectAllParties(self, confid,connectVal):
        if connectVal == "true":
            msgStr = "Connecting"
        else:
            msgStr = "Disconnecting"

        self.LoadXmlFile('Scripts/TransConf2.xml')
        self.ModifyXml("GET","ID",confid)
        self.Send()

        ongoing_parties = self.xmlResponse.getElementsByTagName("PARTY")
        num_ongoing_parties = len(ongoing_parties)
        print msgStr + " " + str(num_ongoing_parties) + " parties in conf " + str(confid)

        for x in range(num_ongoing_parties):
          partyId = ongoing_parties[x].getElementsByTagName("ID")[0].firstChild.data
          print msgStr + " party " + str(partyId)
          self.LoadXmlFile('Scripts/ReconnectParty/TransSetConnect.xml')
          self.ModifyXml("SET_CONNECT","ID",confid)
          self.ModifyXml("SET_CONNECT","CONNECT",connectVal)
          self.ModifyXml("SET_CONNECT","PARTY_ID",partyId)
          self.Send()
          if x % 10 == 0:
              sleep(1)

#------------------------------------------------------------------------------
    def SimpleCreateConfAndParties(self,num_of_parties=10):
        return self.SimpleXmlConfPartyTest('Scripts/AddCpConf.xml', 'Scripts/AddVideoParty.xml', num_of_parties,60,"FALSE")
#------------------------------------------------------------------------------
    def SimpleXmlConfPartyTest(self, confFile, partyFile, num_of_parties, num_retries, deleteConf="TRUE",secondPartyFile="NONE",delayBetweenParticipants=0,connectPartiesOneByOne="FALSE"):
        print "Adding Conf..."
        status = self.SendXmlFile(confFile)

        print "Wait untill Conf create...",
        for retry in range(num_retries+1):
            status = self.SendXmlFile('Scripts/TransConfList.xml',"Status OK")
            confid = self.GetTextUnder("CONF_SUMMARY","ID")
            if confid != "":
                print
                break
            if (retry == num_retries):
                print self.xmlResponse.toprettyxml(encoding="utf-8")
                self.Disconnect()
                ScriptAbort("Can not monitor conf:" + status)
            sys.stdout.write(".")
            sys.stdout.flush()

        print "Create conf with id " + str(confid)
        self.LoadXmlFile(partyFile)
        for x in range(num_of_parties):
            partyname = "Party"+str(x+1)
            partyip =  "1.2.3." + str(x+1)
            print "Adding Party ("+partyname+")"
            self.ModifyXml("PARTY","NAME",partyname)
            self.ModifyXml("PARTY","IP",partyip)
            self.ModifyXml("ADD_PARTY","ID",confid)
            self.ModifyXml("ALIAS","NAME",partyname)
            self.Send()
            sleep(delayBetweenParticipants)
            if (connectPartiesOneByOne == "TRUE"):
            	#self.WaitAllOngoingConnected(confid,num_retries)
            	partyid = self.GetPartyId(confid,partyname)
            	self.WaitPartyConnected2(confid,partyid)
            	self.LoadXmlFile(partyFile)
                if x == 0:
                    print "Dima from Scripts after first party connected"
                    os.system("callgrind_control -d")

        all_num_parties = num_of_parties
        if secondPartyFile != "NONE":
            all_num_parties = 2*num_of_parties
            self.LoadXmlFile(secondPartyFile)
            for x in range(num_of_parties):
                partyname = "Party"+str(num_of_parties+x+1)
                partyip =  "5.6.7." + str(num_of_parties+x+1)
                print "Adding Party ("+partyname+")"
                self.ModifyXml("PARTY","NAME",partyname)
                self.ModifyXml("PARTY","IP",partyip)
                self.ModifyXml("ADD_PARTY","ID",confid)
                self.ModifyXml("ALIAS","NAME",partyname)
                self.Send()

        self.WaitAllPartiesWereAdded(confid,all_num_parties,num_retries*all_num_parties)
        self.WaitAllOngoingConnected(confid,num_retries*all_num_parties)
        self.WaitAllOngoingNotInIVR(confid)

        if deleteConf=="TRUE":
            if(delayBetweenParticipants > 0):
                self.LoadXmlFile('Scripts/TransConf2.xml')
                self.ModifyXml("GET","ID",confid)
                self.Send()
                participants_id_list = []
                for x in range(all_num_parties):
                    some_party_id = self.GetTextUnder("PARTY","ID",x)
                    if some_party_id != "":
                        participants_id_list.append(some_party_id)
                for x in participants_id_list:
                    self.DeleteParty(confid,x)
                    print "Delete party id - "+ x
                    sleep(delayBetweenParticipants)

            #print "Delete Conference..."
            sleep (2)
            self.DeleteConf(confid)

            #print "Wait until no conferences..."
            self.WaitAllConfEnd()

        return confid
#------------------------------------------------------------------------------
    def SimpleXmlConfPartyTest_1080_60(self,confFile,partyFile,num_of_parties,num_retries,deleteConf="TRUE",secondPartyFile="NONE",delayBetweenParticipants=0,connectPartiesOneByOne="FALSE"):
        print "Adding Conf..."
        status = self.SendXmlFile(confFile)

        print "Wait untill Conf create...",
        for retry in range(num_retries+1):
            status = self.SendXmlFile('Scripts/TransConfList.xml',"Status OK")
            confid = self.GetTextUnder("CONF_SUMMARY","ID")
            if confid != "":
                print
                break
            if (retry == num_retries):
                print self.xmlResponse.toprettyxml(encoding="utf-8")
                self.Disconnect()
                ScriptAbort("Can not monitor conf:" + status)
            sys.stdout.write(".")
            sys.stdout.flush()

        print "Create conf with id " + str(confid)
        self.LoadXmlFile(partyFile)
        for x in range(num_of_parties):
            #partyname = "Party"+str(x+1)
	    partyname = str(x+1) + "_##FORCE_1080_AT_60_SETUP"
            partyip =  "1.2.3." + str(x+13)
            print "Adding Party ("+partyname+")"
            self.ModifyXml("PARTY","NAME",partyname)
            self.ModifyXml("PARTY","IP",partyip)
            self.ModifyXml("ADD_PARTY","ID",confid)
            self.Send()
            sleep(delayBetweenParticipants)
            if (connectPartiesOneByOne == "TRUE"):
            	#self.WaitAllOngoingConnected(confid,num_retries)
            	partyid = self.GetPartyId(confid,partyname)
            	self.WaitPartyConnected2(confid,partyid)
            	self.LoadXmlFile(partyFile)

        all_num_parties = num_of_parties
        if secondPartyFile != "NONE":
            all_num_parties = 2*num_of_parties
            self.LoadXmlFile(secondPartyFile)
            for x in range(num_of_parties):
                partyname = "Party"+str(num_of_parties+x+1)
                partyip =  "5.6.7." + str(num_of_parties+x+1)
                print "Adding Party ("+partyname+")"
                self.ModifyXml("PARTY","NAME",partyname)
                self.ModifyXml("PARTY","IP",partyip)
                self.ModifyXml("ADD_PARTY","ID",confid)
                self.Send()

        self.WaitAllPartiesWereAdded(confid,all_num_parties,num_retries*all_num_parties)
        self.WaitAllOngoingConnected(confid,num_retries*all_num_parties)
        self.WaitAllOngoingNotInIVR(confid)

        if deleteConf=="TRUE":
            if(delayBetweenParticipants > 0):
                self.LoadXmlFile('Scripts/TransConf2.xml')
                self.ModifyXml("GET","ID",confid)
                self.Send()
                participants_id_list = []
                for x in range(all_num_parties):
                    some_party_id = self.GetTextUnder("PARTY","ID",x)
                    if some_party_id != "":
                        participants_id_list.append(some_party_id)
                for x in participants_id_list:
                    self.DeleteParty(confid,x)
                    print "Delete party id - "+ x
                    sleep(delayBetweenParticipants)

            #print "Delete Conference..."
            self.DeleteConf(confid)

            #print "Wait until no conferences..."
            self.WaitAllConfEnd()

        return confid

#------------------------------------------------------------------------------
    def WaitAllPartiesWereAdded(self,confid,num_of_parties,num_retries,delayBetweenXmlReq = 1):
        """Monitor conference until it has 'num_of_parties' defined parties
        """
        self.LoadXmlFile('Scripts/TransConf2.xml')
        self.ModifyXml("GET","ID",confid)
        print "Wait Untill All parties are added..",
        self.Send()
        last_ongoing = 0
        for retry in range(num_retries+1):
            ongoing_parties = self.xmlResponse.getElementsByTagName("ONGOING_PARTY_STATUS")
            num_ongoing_parties = len(ongoing_parties)
            if last_ongoing != num_ongoing_parties:
                con_string =  "["+str(num_ongoing_parties)+"/"+str(num_of_parties)+"]"
                sys.stdout.write(con_string)
            last_ongoing = num_ongoing_parties
            if num_ongoing_parties == num_of_parties:
                print
                break

            sys.stdout.write(".")
            sys.stdout.flush()
            if (retry == num_retries):
                print self.xmlResponse.toprettyxml(encoding="utf-8")
                self.Disconnect()
                ScriptAbort(str(num_of_parties - num_ongoing_parties)+" parties are not connected")
            self.Send()
            sleep(delayBetweenXmlReq)


#------------------------------------------------------------------------------
    def MonitorConf(self,confId):
        """Details monitoring for a conference.
        """
        self.LoadXmlFile('Scripts/GetConferenceMonitorTemplate.xml')
        self.ModifyXml("GET","ID",confId)
        self.Send()

#------------------------------------------------------------------------------
    def MonitorParty(self,confId,partyId):
        """Details monitoring for a party.
        """
        self.LoadXmlFile('Scripts/MonitoringParty.xml' )
        self.ModifyXml("GET","CONF_ID",confId)
        self.ModifyXml("GET","PARTY_ID",partyId)
        self.Send()

#------------------------------------------------------------------------------
    def DeleteAllConf(self,delayBetweenConfDel=0):
        """Delete all on-going conferences.
        """
        print "Delete all conferences..."
        self.SendXmlFile('Scripts/TransConfList.xml',"Status OK")
        conf_list = self.xmlResponse.getElementsByTagName("CONF_SUMMARY")
        for conf in conf_list:
            conf_id = conf.getElementsByTagName("ID")[0].firstChild.data
            self.DeleteConf(conf_id)
            sleep(delayBetweenConfDel)

#------------------------------------------------------------------------------
    def SimpleXConfTest(self,num_of_conf,confFile,timeout):

        for conf_num in range(num_of_conf):
            confname = "Conf"+str(conf_num+1)
            self.LoadXmlFile(confFile)
            self.ModifyXml("RESERVATION","NAME",confname)
            print "Adding Conf: " + confname + "  ..."
            self.Send()

        if (self.IsProcessUnderValgrind("Resource")):
            sleeptime = 5
        else:
            sleeptime = 1

        sleep(sleeptime)

        #print "delete all conferences"
        self.DeleteAllConf()

        #print "waiting until all conferences end"
        self.WaitAllConfEnd(timeout)

#------------------------------------------------------------------------------
    def SimpleNewConfTestWithoutTerminate(self,confname,confFile,timeout, additionUrl=""):

        self.LoadXmlFile(confFile)
        self.ModifyXml("RESERVATION","NAME",confname)
        print "Adding Conf: " + confname + "  ..."

        self.SendXml(self.loadedXml, "Status OK", additionUrl)

#------------------------------------------------------------------------------
    def TestFreeCarmelParties(self,num_of_resource):

        self.SendXmlFile("Scripts/ResourceMonitorCarmel.xml")
        my_string = self.GetTextUnder("RSRC_REPORT_RMX","FREE")

        if my_string !=  str(num_of_resource):
            print self.xmlResponse.toprettyxml(encoding="utf-8")
            ScriptAbort("error" + my_string )

#------------------------------------------------------------------------------
    def PrintLastResponse(self):
        print self.xmlResponse.toprettyxml(encoding="utf-8")

#------------------------------------------------------------------------------
    def WaitAllOngoingDisConnected(self,confid,num_retries=30):
        """Monitor conference until all on-going partys are 'disconnected'.
        """
        print "Wait until all ongoing parties are disconnected",
        self.LoadXmlFile('Scripts/TransConf2.xml')
        self.ModifyXml("GET","ID",confid)
        self.Send()
        for retry in range(num_retries+1):
            all_status = ""
            wanted_all_status = ""
            ongoing_parties = self.xmlResponse.getElementsByTagName("ONGOING_PARTY_STATUS")

            for party in ongoing_parties:
                status = party.getElementsByTagName("DESCRIPTION")[0].firstChild.data
                all_status += status + " "
                wanted_all_status += "disconnected "
                if status != "disconnected":
                    if (retry == num_retries):
                        print self.xmlResponse.toprettyxml(encoding="utf-8")
                        self.Disconnect()
                        ScriptAbort("party is connected :" + status)
            sys.stdout.write(".")
            sys.stdout.flush()
            if all_status == wanted_all_status:
                print
                break
            self.Send()
            sleep(1)

#------------------------------------------------------------------------------
    def WaitPartyDisConnected(self,confid,partyid,num_retries=30):
        """Monitor conference until the party is 'disconnected'.
        """
        print "Wait until party is disconnected",
        self.LoadXmlFile('Scripts/TransConf2.xml')
        self.ModifyXml("GET","ID",confid)
        self.Send()
        wanted_status = "disconnected"

        status = ""
        for retry in range(num_retries+1):
            ongoing_parties = self.xmlResponse.getElementsByTagName("ONGOING_PARTY_STATUS")
            ongoing_party_list = self.xmlResponse.getElementsByTagName("ONGOING_PARTY")
            i = 0;
            for party in ongoing_parties:
                if(partyid == ongoing_party_list[i].getElementsByTagName("ID")[0].firstChild.data):
                    status = party.getElementsByTagName("DESCRIPTION")[0].firstChild.data
                    if status != "disconnected":
                        if (retry == num_retries):
                            print self.xmlResponse.toprettyxml(encoding="utf-8")
                            self.Disconnect()
                            ScriptAbort("party is connected :" + status)
                    break
                i=i+1

            sys.stdout.write(".")
            sys.stdout.flush()
            if status == "disconnected":
                print
                break
            self.Send()
            sleep(1)

 #------------------------------------------------------------------------------
    def WaitPartyConnected(self,confid,partyid,num_retries=30):
        """Monitor conference until the party is 'connected'.
        """
        print "Wait until party is connected",
        self.LoadXmlFile('Scripts/TransConf2.xml')
        self.ModifyXml("GET","ID",confid)
        self.Send()
        for retry in range(num_retries+1):
            all_status = ""
            wanted_status = "connected"
            ongoing_parties = self.xmlResponse.getElementsByTagName("ONGOING_PARTY_STATUS")
            ongoing_party_list = self.xmlResponse.getElementsByTagName("ONGOING_PARTY")
            i = 0;
            for party in ongoing_parties:
                status = party.getElementsByTagName("DESCRIPTION")[0].firstChild.data
                if(partyid == ongoing_party_list[i].getElementsByTagName("ID")[0].firstChild.data):
                    if status != "connected":
                        if (retry == num_retries):
                            print self.xmlResponse.toprettyxml(encoding="utf-8")
                            self.Disconnect()
                            ScriptAbort("party is connected :" + status)
                    break
                i=i+1
            sys.stdout.write(".")
            sys.stdout.flush()
            if status == wanted_status:
                print
                break
            self.Send()
            sleep(1)
#------------------------------------------------------------------------------
    def WaitPartyConnected2(self,confid,partyid,num_retries=30):
        """Monitor conference until the party is 'connected'.
        """
        print "Wait until party is connected",
        self.LoadXmlFile('Scripts/TransConf2.xml')
        self.ModifyXml("GET","ID",confid)
        self.Send()
        for retry in range(num_retries+1):
            ongoing_parties = self.xmlResponse.getElementsByTagName("ONGOING_PARTY_STATUS")
            ongoing_party_list = self.xmlResponse.getElementsByTagName("ONGOING_PARTY")
	    i = 0;
            for party in ongoing_parties:
                if(partyid == ongoing_party_list[i].getElementsByTagName("ID")[0].firstChild.data):
                    num_retries1 = 10
                    for retry1 in range(num_retries1):
                    	status = party.getElementsByTagName("DESCRIPTION")[0].firstChild.data
                    	if status != "connected":
                            if (retry1 == num_retries1):
                                print self.xmlResponse.toprettyxml(encoding="utf-8")
                            	self.Disconnect()
                            	ScriptAbort("party is not connected : status = " + status)
                            sys.stdout.write("-")
            		    sys.stdout.flush()
            		    self.Send() ##??
                    	else:
                    	   return
		i=i+1

	    sys.stdout.write(".")
            sys.stdout.flush()
            self.Send()

#------------------------------------------------------------------------------
    def GetPartyStatus(self,confid,partyid):
        """Monitor conference and gets the status of the party.
        """
        self.LoadXmlFile('Scripts/TransConf2.xml')
        self.ModifyXml("GET","ID",confid)
        self.Send()
        ongoing_parties = self.xmlResponse.getElementsByTagName("ONGOING_PARTY_STATUS")
        ongoing_party_list = self.xmlResponse.getElementsByTagName("ONGOING_PARTY")
        i = 0;
        for party in ongoing_parties:
            if(partyid == ongoing_party_list[i].getElementsByTagName("ID")[0].firstChild.data):
               return party.getElementsByTagName("DESCRIPTION")[0].firstChild.data
            i=i+1
#------------------------------------------------------------------------------
    def WaitAllOngoingChangedLayoutType(self,confid, newConfLayout, num_retries=30, party_name_who_shouldnt_see_conf_layout = ""):
        print "Wait until all ongoing parties see new layout type:" + newConfLayout,
        self.LoadXmlFile('Scripts/TransConf2.xml')
        self.ModifyXml("GET","ID",confid)
        for retry in range(num_retries+1):
            self.Send()
            all_layouts = ""
            wanted_all_layouts = ""
            ongoing_parties = self.xmlResponse.getElementsByTagName("ONGOING_PARTY")
            party_index = 0
            for party in ongoing_parties:
                if (party_name_who_shouldnt_see_conf_layout != ""):
                	party_name = ongoing_parties[party_index].getElementsByTagName("NAME")[0].firstChild.data
                	if (party_name == party_name_who_shouldnt_see_conf_layout):
                		party_index = party_index + 1
                		continue
                force = self.getElementsByTagNameFirstLevelOnly(ongoing_parties[party_index],"FORCE")[0]
                layouts = force.getElementsByTagName("LAYOUT")
                force_num = 0
                if(len(layouts) == 2):
                    force_num = 1
                layout = layouts[force_num].firstChild.data
                all_layouts += (layout + " ")
                wanted_all_layouts += (newConfLayout +" ")
                if layout != newConfLayout:
                    if (retry == num_retries):
                        print self.xmlResponse.toprettyxml(encoding="utf-8")
                        self.Disconnect()
                        ScriptAbort("Party is not in conf new layout: "+ newConfLayout +", But in layout : " + layout)
                party_index = party_index + 1
            if all_layouts == wanted_all_layouts:
                break
            sys.stdout.write(".")
            sys.stdout.flush()
            sleep(1)

#------------------------------------------------------------------------------
    def ChangeDialOutAudioSpeaker(self, confid, partyid):
    	if((partyid+1)<10):
        	partyname = "DIAL_OUT#1000" + str(partyid+1)
        else:
        	partyname = "DIAL_OUT#100" + str(partyid+1)
        self.LoadXmlFile('Scripts/SpeakerChange/SimAudioSpeaker.xml')
        self.ModifyXml("AUDIO_SPEAKER","PARTY_NAME", partyname)
        self.Send()
        print "New Audio Speaker PartyId = " + str(partyid) + ", sim gui name = " + partyname

#------------------------------------------------------------------------------
    def ChangeDialOutVideoSpeaker(self, confid, partyid):
    	if((partyid+1)<10):
        	partyname = "DIAL_OUT#1000" + str(partyid+1)
        else:
        	partyname = "DIAL_OUT#100" + str(partyid+1)
        self.LoadXmlFile('Scripts/SpeakerChange/SimActiveSpeaker.xml')
        self.ModifyXml("ACTIVE_SPEAKER","PARTY_NAME", partyname)
        self.Send()
        print "New Video Speaker PartyId = " + str(partyid) + ", sim gui name = " + partyname

#------------------------------------------------------------------------------
    def WaitPartySeesPartyInCell(self, confid, partyid, partyToForce, cellToForce, num_retries=30):
        print "Wait until party: " + str(partyid) + " sees party: " + str(partyToForce) + " in cell: " + str(cellToForce)
        self.LoadXmlFile('Scripts/TransConf2.xml')
        self.ModifyXml("GET","ID",confid)
        for retry in range(num_retries+1):
            self.Send()
            ongoing_parties = self.xmlResponse.getElementsByTagName("ONGOING_PARTY")
            if len(ongoing_parties[partyid].getElementsByTagName("CELL")) == 0 and partyToForce == -1:
                break;
            seenImageInCell = ongoing_parties[partyid].getElementsByTagName("CELL")[cellToForce].getElementsByTagName("SOURCE_ID")[0].firstChild.data
            print "party: " + str(partyid) + " sees party: " + seenImageInCell + " in cell: " + str(cellToForce) + " [ need to see: " + str(partyToForce) + " ]"
            if (seenImageInCell == str(partyToForce)):
                break
            if (retry == num_retries):
                print self.xmlResponse.toprettyxml(encoding="utf-8")
                self.Disconnect()
                ScriptAbort("Party does not see Party Level force: "+ str(partyToForce) +", But Party : " + seenImageInCell)
            sys.stdout.write(".")
            sys.stdout.flush()
            sleep(1)
#------------------------------------------------------------------------------
    def GetForceElementFromConference(self,confid, num_retries=2):

        self.LoadXmlFile('Scripts/TransConf2.xml')
        self.ModifyXml("GET","ID",confid)

        for retry in range(num_retries+1):
            self.Send()
            return self.xmlResponse.getElementsByTagName("FORCE")[0]

        ScriptAbort("Force element wasnt found")

#------------------------------------------------------------------------------
    def WaitAllPartiesSeesPartyInCell(self, confid, partyToForce, cellToForce, num_retries=30):
        print "Wait until All parties sees party: " + str(partyToForce) + " in cell: " + str(cellToForce)
        self.LoadXmlFile('Scripts/TransConf2.xml')
        self.ModifyXml("GET","ID",confid)
        self.Send()
        ongoing_parties = self.xmlResponse.getElementsByTagName("ONGOING_PARTY")
        index = 0
        for party in ongoing_parties:
           	partyid = party.getElementsByTagName("ID")[0].firstChild.data
           	name =  party.getElementsByTagName("NAME")[0].firstChild.data
           	if (self.GetTextUnder("ONGOING_PARTY_STATUS","DESCRIPTION",index) == "connected"):
           		self.WaitPartySeesPartyInCell(confid, int(partyid), int(partyToForce), int(cellToForce), num_retries)
           	else:
           		print name + "is not connected"
           	index = index + 1
#------------------------------------------------------------------------------
    def WaitCellIsMarkedAsAutoScan(self,confid,newConfLayout,autoScanCell,num_retries=30):
 	print "Wait until cell number: " +autoScanCell +" is marked as auto scan in layout " + newConfLayout
        ForcedCell = int(autoScanCell) - 1
        self.LoadXmlFile('Scripts/TransConf2.xml')
        self.ModifyXml("GET","ID",confid)
        for retry in range(num_retries+1):
            self.Send()
            force_list = self.xmlResponse.getElementsByTagName("FORCE")
	    for force in force_list:
		if(force.getElementsByTagName("LAYOUT")[0].firstChild.data == newConfLayout):
        	   if(force.getElementsByTagName("CELL")[ForcedCell].getElementsByTagName("FORCE_STATE")[0].firstChild.data == "auto_scan"):
        		print "found auto scan cell"
        		return
            if (retry == num_retries):
                print self.xmlResponse.toprettyxml(encoding="utf-8")
                self.Disconnect()
                ScriptAbort("Did not found auto scan cell in conf layout")
            sys.stdout.write(".")
            sys.stdout.flush()


#------------------------------------------------------------------------------
    def WaitPartySeesPersonalLayout(connection,confid,partyid,LayoutType,num_retries=30):
        print "Wait until party sees Personal layout type:" + LayoutType
        connection.LoadXmlFile('Scripts/TransConf2.xml')
        connection.ModifyXml("GET","ID",confid)
        for retry in range(num_retries+1):
            connection.Send()
            ongoing_parties = connection.xmlResponse.getElementsByTagName("ONGOING_PARTY")
            currentlayoutType = ongoing_parties[partyid-1].getElementsByTagName("LAYOUT_TYPE")[0].firstChild.data
            currentlayout = ongoing_parties[partyid-1].getElementsByTagName("PERSONAL_LAYOUT")[0].firstChild.data
            if (currentlayoutType == "personal"):
                if(currentlayout == LayoutType):
                    break
            if (retry == num_retries):
                print connection.xmlResponse.toprettyxml(encoding="utf-8")
                connection.Disconnect()
                ScriptAbort("Party is not in Personal Layout: "+ LayoutType +", But in layout : " + currentlayout)
            sys.stdout.write(".")
            sys.stdout.flush()
            sleep(1)

#------------------------------------------------------------------------------
    def WaitPartySeesConfLayout(connection,confid,partyid,LayoutType,num_retries=30):
        print "Wait until party sees Conference layout type:" + LayoutType
        connection.LoadXmlFile('Scripts/TransConf2.xml')
        connection.ModifyXml("GET","ID",confid)
        connection.Send()
        ongoing_parties = connection.xmlResponse.getElementsByTagName("ONGOING_PARTY")

        for partyIndex in range(len(ongoing_parties)):
            if (ongoing_parties[partyIndex].getElementsByTagName("ID")[0].firstChild.data == partyid):
                break

        for retry in range(num_retries+1):
            currentlayoutType = ongoing_parties[partyIndex].getElementsByTagName("LAYOUT_TYPE")[0].firstChild.data
            force = connection.getElementsByTagNameFirstLevelOnly(ongoing_parties[partyIndex],"FORCE")[0]
            layouts = force.getElementsByTagName("LAYOUT")
            force_num = 0
            if(len(layouts) == 2):
                force_num = 1
            currentlayout = layouts[force_num].firstChild.data
            if (currentlayoutType == "conference"):
                if(currentlayout == LayoutType):
                    break
            if (retry == num_retries):
                print connection.xmlResponse.toprettyxml(encoding="utf-8")
                connection.Disconnect()
                ScriptAbort("Party is not in ConfLayout Layout: "+ LayoutType +", But in layout : " + currentlayout)
            sys.stdout.write(".")
            sys.stdout.flush()
            sleep(1)

#------------------------------------------------------------------------------
    def CheckValidityOfLectureMode(self,confid,lecturerPartyId,conflayoutType,num_retries=30):
        self.WaitActiveLecturer(confid, lecturerPartyId+1, num_retries)
        self.WaitAllSeeLecturer(confid, lecturerPartyId+1, num_retries)
        self.WaitLecturerSeesConfLayout(confid,lecturerPartyId,conflayoutType, num_retries)

#------------------------------------------------------------------------------
    def WaitActiveLecturer(self,confid,lecturerPartyId,num_retries=30):
        print "Wait until PartyId: " + str(lecturerPartyId) + " is identified as the lecturer"
        self.LoadXmlFile('Scripts/SpeakerChange/TransConf2.xml')
        self.ModifyXml('GET','ID',confid)
        for retry in range(num_retries+1):
            self.Send()
            currentLecturerPartyId = self.xmlResponse.getElementsByTagName("LECTURE_ID")[0].firstChild.data
            if(currentLecturerPartyId == str(lecturerPartyId)):
                break
            if (retry == num_retries):
                print
                self.Disconnect()
                ScriptAbort("Setting Party Id: "+ str(lecturerPartyId)+" as Lecturer Failed!!!")
            sys.stdout.write(".")
            sys.stdout.flush()
            sleep(1)

#------------------------------------------------------------------------------
    def getElementsByTagNameFirstLevelOnly(self, xmlElement, name):
	NodeL = []
        return self._get_elements_by_tagName_First_Level_helper(xmlElement, name, NodeL)
#------------------------------------------------------------------------------
    def _get_elements_by_tagName_First_Level_helper(self, parent, name, rc):

        for node in parent.childNodes:
	    if (node.nodeName == name):
                rc.append(node)
            #self._get_elements_by_tagName_First_Level_helper(node, name, rc)

    	return rc

#------------------------------------------------------------------------------
    def WaitAllSeeLecturer(self,confid,lecturerPartyId,num_retries=30):
        print "Wait until All Parties see Lecturer - PartyId: " + str(lecturerPartyId) + " in Layout 1x1"
        sleep(2)
        self.LoadXmlFile('Scripts/TransConf2.xml')
        self.ModifyXml("GET","ID",confid)
        for retry in range(num_retries+1):
            self.Send()
            all_layouts = ""
            wanted_all_layouts = ""
            ongoing_parties = self.xmlResponse.getElementsByTagName("ONGOING_PARTY")
            i = 0
            for party in ongoing_parties:
                force = self.getElementsByTagNameFirstLevelOnly(ongoing_parties[i],"FORCE")[0]
                layout = force.getElementsByTagName("LAYOUT")[0].firstChild.data
                if (len(force.getElementsByTagName("CELL")) <= 0):
                    imagePartyIdInCellZero ="INVALID"
                else:
                    imagePartyIdInCellZero = force.getElementsByTagName("CELL")[0].getElementsByTagName("SOURCE_ID")[0].firstChild.data
                thisPartyId = party.getElementsByTagName("ID")[0].firstChild.data
                layout_and_image = layout + " " +imagePartyIdInCellZero
                if (thisPartyId != str(lecturerPartyId)):
                    all_layouts += (layout_and_image + " ")
                    wanted_all_layouts += ("1x1 "+ str(lecturerPartyId) + " ")
                    if layout_and_image != ("1x1 "+ str(lecturerPartyId)):
                       if (retry == num_retries):
                           print self.xmlResponse.toprettyxml(encoding="utf-8")
                           self.Disconnect()
                           ScriptAbort("PartyId: " + str(thisPartyId) +  " does not see lecturer: "+ str(lecturerPartyId) +", But Party: " + str(imagePartyIdInCellZero) + " In Layout: " + str(layout))
                i = i + 1
            if all_layouts == wanted_all_layouts:
                break
            sys.stdout.write(".")
            sys.stdout.flush()
            sleep(1)

#------------------------------------------------------------------------------
    def WaitLecturerSeesConfLayout(self,confid,lecturerPartyId,confLayoutType,num_retries=30):
        print "Wait until lecturer sees layout type:" + confLayoutType
        self.LoadXmlFile('Scripts/TransConf2.xml')
        self.ModifyXml("GET","ID",confid)
        for retry in range(num_retries+1):
            self.Send()
            ongoing_parties = self.xmlResponse.getElementsByTagName("ONGOING_PARTY")
            force = self.getElementsByTagNameFirstLevelOnly(ongoing_parties[lecturerPartyId],"FORCE")[0]
            layout = force.getElementsByTagName("LAYOUT")[0].firstChild.data
            if layout == confLayoutType:
                break
            if (retry == num_retries):
                print self.xmlResponse.toprettyxml(encoding="utf-8")
                self.Disconnect()
                ScriptAbort("Lecturer is not in conf new layout: "+ confLayoutType +", But in layout : " + layout)
            sys.stdout.write(".")
            sys.stdout.flush()
            sleep(1)

#------------------------------------------------------------------------------
    def SimulationAddSipParty(self, partyName, confName, capSetName = "FULL CAPSET"):
        """Add SIP Party in Simulation.
        The Party will be a dial-in to confName.

        partyName - dial-in party name.
        confName - destination conf.
        """
        print "Adding Sim SIP Party " + partyName
        self.LoadXmlFile("Scripts/SimAddSipParty.xml")
        self.ModifyXml("SIP_PARTY_ADD","PARTY_NAME",partyName)
        self.ModifyXml("SIP_PARTY_ADD","CONF_NAME",confName)
        self.ModifyXml("SIP_PARTY_ADD","CAPSET_NAME",capSetName)
        self.Send()
        sleep(1)
#------------------------------------------------------------------------------
    def SimulationAddSipTelepresenceParty(self, partyName, confName, userAgent, capSetName = "FULL CAPSET"):
        """Add SIP Party in Simulation.
        The Party will be a dial-in to confName.

        partyName - dial-in party name.
        confName - destination conf.
        """
        print "Adding Sim SIP Party " + partyName
        self.LoadXmlFile("Scripts/SimAddSipParty.xml")
        self.ModifyXml("SIP_PARTY_ADD","PARTY_NAME",partyName)
        self.ModifyXml("SIP_PARTY_ADD","CONF_NAME",confName)
        self.ModifyXml("SIP_PARTY_ADD","CAPSET_NAME",capSetName)
        self.ModifyXml("SIP_PARTY_ADD","USER_AGENT",userAgent)
        self.Send()
        sleep(1)

#------------------------------------------------------------------------------
#------------------------------------------------------------------------------
    def SimulationConnectSipParty(self, partyName):
        """Connect SIP Party defined in Simulation.
        Party must be first defined in simulation!

        partyName - dial-in party name to be connected.
        """
        print "Connecting Sim SIP Party " + partyName
        self.LoadXmlFile("Scripts/SimConnectSipParty.xml")
        self.ModifyXml("SIP_PARTY_CONNECT","PARTY_NAME",partyName)
        self.Send()

#------------------------------------------------------------------------------
    def SimulationSipPartyDTMF(self, partyName, dtmfString):
        """Send DTMF string through connected party.
        Party must be first connected.

        partyName - dial-in party name.
        dtmfString - dtmf code to be sent.
        """
        self.LoadXmlFile("Scripts/SimDtmfSipParty.xml")
        self.ModifyXml("SIP_PARTY_DTMF","PARTY_NAME",partyName)
        self.ModifyXml("SIP_PARTY_DTMF","DTMF_STRING",dtmfString + "#")
        self.Send()

#------------------------------------------------------------------------------
    def SimulationDisconnectSipParty(self, partyName):
        """Disconnect SIP Party defined in Simulation.

        partyName - dial-in party name to be disconnected.
        """
        print "Disconnecting Sim SIP Party " + partyName
        self.LoadXmlFile("Scripts/SimDisconnectSipParty.xml")
        self.ModifyXml("SIP_PARTY_DISCONNECT","PARTY_NAME", partyName)
        self.Send()

#------------------------------------------------------------------------------
    def SimulationDeleteSipParty(self, partyName):
        """Delete SIP party from simulation.

        partyName - dial-in party name.
        """
        print "Deleting SIM SIP party..."
        self.LoadXmlFile("Scripts/SimDelSipParty.xml")
        self.ModifyXml("SIP_PARTY_DEL","PARTY_NAME",partyName)
        self.Send()

#------------------------------------------------------------------------------
    def SimulationAddH323Party(self, partyName, confName, capSetName="FULL CAPSET", ipVer=0,manuName="EndpointsSim",sourcePartyAliasName="HDX"):
        """Add H323 Party in Simulation.
        The Party will be a dial-in to confName.

        partyName - dial-in party name.
        confName - destination conf.
        """
        print "Adding Sim H323 Party " + partyName
        self.LoadXmlFile("Scripts/SimAdd323Party.xml")
        self.ModifyXml("H323_PARTY_ADD","PARTY_NAME",partyName)
        self.ModifyXml("H323_PARTY_ADD","CONF_NAME",confName)
        self.ModifyXml("H323_PARTY_ADD","CAPSET_NAME",capSetName)
        self.ModifyXml("H323_PARTY_ADD","IP_VER",ipVer)
        self.ModifyXml("H323_PARTY_ADD","MANUFUCTURER_NAME",manuName)
        self.ModifyXml("H323_PARTY_ADD","SOURCE_PARTY_ALIAS",sourcePartyAliasName)
        self.Send()


 #------------------------------------------------------------------------------
    def SimulationConnectH323Party(self, partyName):
        """Connect H323 Party defined in Simulation.
        Party must be first defined in simulation!

        partyName - dial-in party name to be connected.
        """
        print "Connecting Sim H323 Party " + partyName
        self.LoadXmlFile("Scripts/SimConnect323Party.xml")
        self.ModifyXml("H323_PARTY_CONNECT","PARTY_NAME",partyName)
        self.Send()


##------------------------------------------------------------------------------
    def SimulationConnectH323PartyExpStatus(self, partyName, expected_status="Status OK"):
        """Connect H323 Party defined in Simulation.
        Party must be first defined in simulation!

        partyName - dial-in party name to be connected.
        """
        print "Connecting Sim H323 Party " + partyName
        self.LoadXmlFile("Scripts/SimConnect323Party.xml")
        self.ModifyXml("H323_PARTY_CONNECT","PARTY_NAME",partyName)
        self.Send(expected_status)

##------------------------------------------------------------------------------
    def SimulationH323PartyDTMF(self, partyName, dtmfString):
        """Send DTMF string through connected party.
        Party must be first connected.

        partyName - dial-in party name.
        dtmfString - dtmf code to be sent.
        """
        self.LoadXmlFile("Scripts/SimDtmf323Party.xml")
        self.ModifyXml("H323_PARTY_DTMF","PARTY_NAME",partyName)
        self.ModifyXml("H323_PARTY_DTMF","DTMF_STRING",dtmfString + "#")
        self.Send()

#------------------------------------------------------------------------------
    def SimulationH323PartyDTMFWithoutDelimiter(self, partyName, dtmfString):
        """Send DTMF string through connected party.
        Party must be first connected.

        partyName - dial-in party name.
        dtmfString - dtmf code to be sent.
        """
        self.LoadXmlFile("Scripts/SimDtmf323Party.xml")
        self.ModifyXml("H323_PARTY_DTMF","PARTY_NAME",partyName)
        self.ModifyXml("H323_PARTY_DTMF","DTMF_STRING",dtmfString)
        self.Send()

#------------------------------------------------------------------------------
    def WaitUntillPartyDeleted(self,confid,num_retries):
        print "Monitor Party list until empty in Conf: "+confid
        self.LoadXmlFile('Scripts/TransConf2.xml')
        self.ModifyXml("GET","ID",confid)
        self.Send()
        for retry in range(num_retries+1):
            ongoing_parties = self.xmlResponse.getElementsByTagName("ONGOING_PARTY_STATUS")
            if len(ongoing_parties) == 0:
                break

            if (retry == num_retries):
                print "Timeout for Monitor Party list until empty"
                print self.xmlResponse.toprettyxml(encoding="utf-8")
                self.Disconnect()
                ScriptAbort("There are still" + str(len(ongoing_parties)) + " connected parties")

            sys.stdout.write(".")
            sys.stdout.flush()

            self.Send()
            sleep(1)

        print
        print "No Parties in Conference: "+confid

#------------------------------------------------------------------------------
    def WaitAllOngoingNotInIVR(self,confid,num_retries=30):
        """Monitor conference until all ongoing parties are not in IVR.
        """
        print "Wait until all ongoing parties are no longer in IVR",
        self.LoadXmlFile('Scripts/TransConf2.xml')
        self.ModifyXml("GET","ID",confid)
        self.Send()
        last_num_connected = 0
        for retry in range(num_retries+1):
            ongoing_parties = self.xmlResponse.getElementsByTagName("ONGOING_PARTY")
            num_ongoing_parties = len(ongoing_parties)
            num_connected = 0
            for party in ongoing_parties:
                status = party.getElementsByTagName("ATTENDING_STATE")[0].firstChild.data
                if status != "inconf":
                    if (retry == num_retries):
                        print self.xmlResponse.toprettyxml(encoding="utf-8")
                        self.Disconnect()
                        ScriptAbort("party not inconf :" + status)

                else:
                    num_connected=num_connected+1;
            sys.stdout.write(".")
            if last_num_connected != num_connected:
                con_string =  "["+str(num_connected)+"/"+str(len(ongoing_parties))+"]"
                sys.stdout.write(con_string)
            sys.stdout.flush()
            last_num_connected = num_connected
            if num_connected == len(ongoing_parties):
                print
                break
            sleep(1)
            self.Send()

#-----------------------------------------------------------------------------
    def GetConfNumericId(self,targetConfID):
        self.LoadXmlFile('Scripts/MoveUndef/TransConf2.xml')
        self.ModifyXml("GET","ID",targetConfID)
        self.Send()
        numericId = self.GetTextUnder("RESERVATION","NUMERIC_ID")
        return numericId
#-----------------------------------------------------------------------------
    def SimulationDisconnectPSTNParty(self, partyName):
        print "Disconnect participant from EPsim"
        self.LoadXmlFile("Scripts/SimDisconnectEndpoint.xml")
        self.ModifyXml("PARTY_DISCONNECT","PARTY_NAME",partyName)
        self.Send()
#-----------------------------------------------------------------------------
    def AddPSTN_DialoutParty(self,confId,partyName,phone):
        print "Adding PSTN dilaout party: "+partyName+ ", from EMA with phone: " + phone
        self.LoadXmlFile("Scripts/PSTN_Party.xml")
        self.ModifyXml("PARTY","NAME",partyName)
        self.ModifyXml("ADD_PARTY","ID",confId)
        self.ModifyXml("PHONE_LIST","PHONE1",phone)
        self.Send()

#-----------------------------------------------------------------------------
    def DeletePSTNPartyFromSimulation(self,partyName):
        print "Deleting and disconnecting party: "+partyName
        self.LoadXmlFile("Scripts/SimDeleteEndpoint.xml")
        self.ModifyXml("PARTY_DELETE","PARTY_NAME",partyName)
        self.Send()

#-----------------------------------------------------------------------------
    def CreatePSTN_EQ(self,eqName,eqPhone,eqProfileId):
        print "Creating PSTN EQ: " + eqName + ",with phone: " + eqPhone
        self.LoadXmlFile("Scripts/PSTN_EQ.xml")
        self.ModifyXml("RESERVATION","NAME",eqName)
        self.ModifyXml("RESERVATION","AD_HOC_PROFILE_ID",str(eqProfileId))
        self.ModifyXml("SERVICE","PHONE1",str(eqPhone))
        self.Send()
#-----------------------------------------------------------------------------
    def SimulationConnectPSTNParty(self, partyName):
        print "Connect participant from EPsim"
        self.LoadXmlFile("Scripts/SimConnectEndpoint.xml")
        self.ModifyXml("PARTY_CONNECT","PARTY_NAME",partyName)
        self.Send()
#-----------------------------------------------------------------------------
    def SimulationAddPSTNParty(self, partyName,phone):
        print "Add participant:" + partyName+", phone="+ phone
        self.LoadXmlFile("Scripts/SimAddPstnEndpoint.xml")
        self.ModifyXml("PSTN_PARTY_ADD","PARTY_NAME",partyName)
        self.ModifyXml("PSTN_PARTY_ADD","PHONE_NUMBER",phone)
        self.Send()

#-----------------------------------------------------------------------------
    def SimulationDisconnectH323Party(self, partyName):
        """Disconnect H323 Party defined in Simulation.

        partyName - dial-in party name to be disconnected.
        """
        print "disconnect first participant from EPsim"
        self.LoadXmlFile("Scripts/SimDisconnect323Party.xml")
        self.ModifyXml("H323_PARTY_DISCONNECT","PARTY_NAME",partyName)
        self.Send()

#------------------------------------------------------------------------------
    def SimulationDeleteH323Party(self, partyName):
        """Delete H323 party from simulation.

        partyName - dial-in party name.
        """
        print "Deleting SIM H323 party "+partyName+"..."
        self.LoadXmlFile("Scripts/SimDel323Party.xml")
        self.ModifyXml("H323_PARTY_DEL","PARTY_NAME",partyName)
        self.Send()


#------------------------------------------------------------------------------
    def AddProfile(self, profileName, fileName="Scripts/CreateNewProfile.xml",SameLayout=False):
        """Create new Profile.

        profileName - name of profile to be created.
        fileName - XML file
        """
        print "Adding new Profile..."
        self.LoadXmlFile(fileName)
        self.ModifyXml("RESERVATION","NAME", profileName)
        if SameLayout:
            self.ModifyXml("RESERVATION","SAME_LAYOUT", "true")
        self.Send()
        ProfId = self.GetTextUnder("RESERVATION","ID")
        print "Profile, named: " + profileName + " ,ID = " + ProfId + ", is added"
        if (self.IsProcessUnderValgrind("Resource")):
            sleep(1)
        return ProfId

#------------------------------------------------------------------------------
    def DelProfile(self, profId, fileName="Scripts/RemoveNewProfile.xml"):
        """Remove Profile.

        profId - ID of profile to be deleted.
        fileName - XML file
        """
        #print "Deleting profile: " + profId
        self.LoadXmlFile(fileName)
        self.ModifyXml("TERMINATE_PROFILE","ID",profId)
        self.Send("Status OK")

    #------------------------------------------------------------------------------
    def DelProfileByName(self,ProfileNameToDel):
        """ Remove profile by it's name"""

        self.SendXmlFile('Scripts/GetProfileList.xml')
        profile_list = self.xmlResponse.getElementsByTagName("PROFILE_SUMMARY")
        for index in range(1, len(profile_list)):
            tmpProfileName = profile_list[index].getElementsByTagName("NAME")[0].firstChild.data
            profId = profile_list[index].getElementsByTagName("ID")[0].firstChild.data
            if (tmpProfileName == ProfileNameToDel ):
                print "Deleting Profile: " + tmpProfileName
                self.DelProfile(profId)
#------------------------------------------------------------------------------
    def IsProfileNameExists(self,profile_name):
        """ Checks if profile name already exists
        profile_name - profile name
        returns profile id if exists, otherwise -1"""

        self.SendXmlFile('Scripts/GetProfileList.xml')
        profile_list =  self.xmlResponse.getElementsByTagName("PROFILE_SUMMARY")
        for index in range(1, len(profile_list)):
            profName = profile_list[index].getElementsByTagName("NAME")[0].firstChild.data
            profId = profile_list[index].getElementsByTagName("ID")[0].firstChild.data
            if profile_name == profName:
                return profId
        return -1
#------------------------------------------------------------------------------

    def ClearAllDefaultRsrv(self):
        print "Removing all default Reservations"
        self.DeleteAllMR()
        self.DeleteAllProfiles()


#------------------------------------------------------------------------------

    def DeleteAllProfiles(self):
        print "Deleting All Profiles"
        self.SendXmlFile('Scripts/GetProfileList.xml')
        profile_list = self.xmlResponse.getElementsByTagName("PROFILE_SUMMARY")
        for index in range(1, len(profile_list)):
            profName = profile_list[index].getElementsByTagName("NAME")[0].firstChild.data
            profId = profile_list[index].getElementsByTagName("ID")[0].firstChild.data
            print "Deleting Profile: " + profName
            self.DelProfile(profId)

#------------------------------------------------------------------------------

    def DeleteAllMR(self):
        print "Deleting all Meeting rooms"
        status = self.SendXmlFile('Scripts/GetMRList.xml',"Status OK")
        mr_list = self.xmlResponse.getElementsByTagName("MEETING_ROOM_SUMMARY")
        for index in range(len(mr_list)):
            mrName = mr_list[index].getElementsByTagName("NAME")[0].firstChild.data
            mrId = mr_list[index].getElementsByTagName("ID")[0].firstChild.data
            print "Deleting Mr: " + mrName
            self.DelReservation(mrId,"Scripts/AwakeMrByUndef/RemoveMr.xml")

#------------------------------------------------------------------------------
    def CreateAdHocEQ(self, eqName, ProfId, fileName,isAdHoc='true'):
        """Create new EQ reservation.

        eqName - name of EQ to be created.
        ProfId - Id of profile used by EQ.
        fileName - XML file
        """
        print "Adding a new Entry Queue " + eqName
        self.LoadXmlFile(fileName)
        self.ModifyXml("RESERVATION","NAME",eqName)
        self.ModifyXml("RESERVATION","AD_HOC",isAdHoc)
        self.ModifyXml("RESERVATION","AD_HOC_PROFILE_ID",ProfId)
        self.Send()

#------------------------------------------------------------------------------
    def CreateMR(self, mrName, ProfId, fileName="Scripts/CreateMR.xml"):
        """Create new MR reservation.

        mrName - name of MR to be created.
        ProfId - Id of profile used in reservation.
        fileName - XML file
        """
        print "Adding a new MR " + mrName
        self.LoadXmlFile(fileName)
        self.ModifyXml("RESERVATION","NAME",mrName)
        self.ModifyXml("RESERVATION","DISPLAY_NAME",mrName)
        self.ModifyXml("RESERVATION","AD_HOC_PROFILE_ID",str(ProfId))
        self.Send()
#------------------------------------------------------------------------------
    def CreateMRWithNumericId(self, mrName, ProfId, strNID, expected_status="Status OK", fileName="Scripts/CreateMR.xml"):
        """Create new MR reservation.
        mrName - name of MR to be created.
        ProfId - Id of profile used in reservation.
        fileName - XML file
        """
        print "Adding a new MR - " + mrName
        self.LoadXmlFile(fileName)
        self.ModifyXml("RESERVATION","NAME",mrName)
        self.ModifyXml("RESERVATION","AD_HOC_PROFILE_ID",str(ProfId))
        self.AddXML("RESERVATION","NUMERIC_ID",strNID)
        self.Send(expected_status)

        #if we got until here, it means we got the expected status, print it unless it's OK
        if expected_status!= "Status OK":
           print "Indeed received required status of - " + expected_status
           return ""
        else:
           #the status is OK, and this is what we got, we will return the id of the meeting room
           MRId = self.GetTextUnder("RESERVATION","ID")
           print "MR, named: " + mrName + " ,ID = " + MRId + ", is added"
           return MRId
#------------------------------------------------------------------------------
    def DelReservation(self, id, fileName):
        """Delete reservation.
        May be used to delete MRs or EQs.

        id - Id of reservation to be deleted.
        fileName - XML file
        """
        print "Remove reservation, Id:" + str(id)
        self.LoadXmlFile(fileName)
        self.ModifyXml("TERMINATE_MEETING_ROOM","ID",id)
        self.Send("Status OK")
#------------------------------------------------------------------------------
    def DelConfReservation(self, id, fileName = 'Scripts/AddRemoveReservation/DeleteRes.xml'):
        """Delete reservation.
        May be used to delete Reservations.

        id - Id of reservation to be deleted.
        fileName - XML file
        """
        print "Remove Conf Reservation, Id:" + str(id)
        self.LoadXmlFile(fileName)
        self.ModifyXml("TERMINATE_RES","ID",id)
        self.Send("Status OK")
#------------------------------------------------------------------------------
    def DelConfTemplate(self, id, fileName):
        """Delete Conf Template.
        May be used to delete Conf Templates.

        id - Id of Conf template to be deleted.
        fileName - XML file
        """
        print "Remove Conf Template, Id:" + str(id)
        self.LoadXmlFile(fileName)
        self.ModifyXml("TERMINATE_CONFERENCE_TEMPLATE","ID",id)
        self.Send("Status OK")

#------------------------------------------------------------------------------
    def CreateRes(self, resName, ProfId, startTime, strNID="", min_audio = 0, min_video = 0, expected_status="Status OK" , bIsWarning = 0 ,fileName="Scripts/AddRemoveReservation/StartRes.xml", phone1="None"):
        """Create new reservation.
        resName  - name of Res to be created.
        ProfId - Id of profile used in reservation.
        fileName - XML file
        will return the reservation id
        """
        print "Adding a new Reservation named - " + resName
        self.LoadXmlFile(fileName)
        iso_time = startTime.strftime("%Y-%m-%dT%H:%M:%S")
        print "Setting start time: " + iso_time

        self.ModifyXml("RESERVATION","NAME",resName)
        self.ModifyXml("RESERVATION","DISPLAY_NAME",resName)
        self.ModifyXml("RESERVATION","AD_HOC_PROFILE_ID",str(ProfId))
        self.ModifyXml("RESERVATION","START_TIME",iso_time)
        self.ModifyXml("RESERVATION","NUMERIC_ID",strNID)
        if((min_video != 0) | (min_audio != 0)):
        	self.ModifyXml("MEET_ME_PER_CONF","MIN_NUM_OF_PARTIES",str(min_video))
        	self.ModifyXml("MEET_ME_PER_CONF","MIN_NUM_OF_AUDIO_PARTIES",str(min_audio))
        	print "Setting min participants for video: " + str(min_video) + " for audio: " + str(min_audio)
        if(phone1 != "None"):
       		self.ModifyXml("MEET_ME_PER_CONF","ON","true")
		self.ModifyXml("SERVICE","NAME","ISDN")
	        self.ModifyXml("SERVICE","PHONE1",phone1)
        self.Send(expected_status)

        #if we got until here, it means we got the expected status, print it unless it's OK
        if expected_status!= "Status OK":
           print "Indeed received required status of - " + expected_status
           if(bIsWarning == 0):
           	   return ""
        res_id = self.GetTextUnder("RESERVATION","ID")
        print "Reservation named: " + resName + ", ID = " + res_id + ", is added"
        return res_id
#------------------------------------------------------------------------------
    def CreateConfTemplate(self,confName,ProfId,fileName="Scripts/AddRemoveConfTemplate/StartConfTemplate.xml",rate=-1):
        """Create new Conf Template.

        confName  - name of Conf Template to be created.
        ProfId - Id of profile used in reservation.
        fileName - XML file
        """
        print "Adding a new Conf Template-" + confName + " Template..." + "Profile Id: "+str(ProfId)
        self.LoadXmlFile(fileName)
        self.ModifyXml("RESERVATION","NAME",confName)
        self.ModifyXml("RESERVATION","DISPLAY_NAME",confName)
        self.ModifyXml("RESERVATION","AD_HOC_PROFILE_ID",str(ProfId))
        if ProfId==-1:
           self.InitializeConfParamsWithoutProfile(rate)
        self.Send()
        #------------------------------------------------------------------------------
    def GetTemplateIdByName(self,templ_name):
        self.LoadXmlFile("Scripts/AddRemoveConfTemplate/GetConfTemplate.xml")
        self.Send()
        conference_list = self.xmlResponse.getElementsByTagName("GET_CONFERENCE_TEMPLATE")

        for index in range(len(conference_list)):
            confName = conference_list[index].getElementsByTagName("NAME")[0].firstChild.data
            print "confName: " + confName
            templateId = conference_list[index].getElementsByTagName("ID")[0].firstChild.data
            if confName == templ_name:
                return templateId
        return -1
#---------------------------------------------------------------------------
    def GetConfIdByName(self,conf_name):
        self.LoadXmlFile("Scripts/GetConferenceListMonitorTemplate.xml")
        self.Send()
        conference_list = self.xmlResponse.getElementsByTagName("GET_LS")
        
        if len(conference_list)==0:
            return -1

        for index in range(len(conference_list)):
            if len(conference_list[index].getElementsByTagName("NAME")) == 0:
                print "GetConfIdByName - no name exist, skip!"
                continue
            confName = conference_list[index].getElementsByTagName("NAME")[0].firstChild.data
            print "confName: " + confName
            confId = conference_list[index].getElementsByTagName("ID")[0].firstChild.data
            if confName == conf_name:
                return confId
        return -1
#------------------------------------------------------------------------------
    def InitializeConfParamsWithoutProfile(self,rate):
        print "InitializeConfParamsWithoutProfile, new Conf Rate is: "+str(rate)
        self.ModifyXml("RESERVATION","TRANSFER_RATE",str(rate))

#------------------------------------------------------------------------------
    def WaitUntilAllPartiesConnected(self,confid,num_of_parties,num_retries):
        print "Wait untill all parties will be connected ...",
        self.LoadXmlFile('Scripts/MoveUndef/TransConf2.xml')
        self.ModifyXml("GET","ID",confid)

        for retry in range(num_retries+1):
            self.Send()
            ongoing_party_list = self.xmlResponse.getElementsByTagName("ONGOING_PARTY")
            if (len(ongoing_party_list) ==  num_of_parties):
                print "Found " + str(num_of_parties) + " parties in conference " + str(confid)
                self.WaitAllOngoingConnected(confid)
                break
            if (retry == num_retries):
                print self.xmlResponse.toprettyxml(encoding="utf-8")
                self.Disconnect()
                ScriptAbort("Can not find all Parties in the target conf")
            sys.stdout.write(".")
            sys.stdout.flush()
            sleep(1)

#------------------------------------------------------------------------------
    def SimulationSetCaps(self, capSetName, CapsList):
        """Add a new capset to simulation

        capSetName - the new capSet name
        capsList - list of the capabilities.
        """
        print "Resetting the capSet!"
        self.LoadXmlFile("Scripts/SimAddCapSet.xml")
        self.ModifyXml("ADD_CAP_SET","NAME",capSetName)
        self.ModifyXml("ADD_CAP_SET","AUDIO_SIREN14","false")
        self.ModifyXml("ADD_CAP_SET","AUDIO_G7221","false")
        self.ModifyXml("ADD_CAP_SET","AUDIO_G722","false")
        self.ModifyXml("ADD_CAP_SET","AUDIO_G7231","false")
        self.ModifyXml("ADD_CAP_SET","AUDIO_G729","false")
        self.ModifyXml("ADD_CAP_SET","AUDIO_G728","false")
        self.ModifyXml("ADD_CAP_SET","AUDIO_G711","false")
        self.ModifyXml("ADD_CAP_SET","VIDEO_H264","false")
        self.ModifyXml("ADD_CAP_SET","VIDEO_H263","false")
        self.ModifyXml("ADD_CAP_SET","VIDEO_VP8","false") ##N.A. DEBUG VP8

        print "adding new capSet to simulation "+capSetName+"..."

        ###self.LoadXmlFile("Scripts/SimAddCapSet.xml")
        ###self.ModifyXml("ADD_CAP_SET","NAME",capSetName)

        for cap in CapsList:
            print "adding new cap : " + cap
            self.ModifyXml("ADD_CAP_SET",cap,"true")

        self.Send()


#------------------------------------------------------------------------------
    def SimulationDelCaps(self, capSetName):
        """del capset from simulation

        capSetName - the new capSet name
        """
        print "delete caps set - " + capSetName
        self.LoadXmlFile("Scripts/CapabilitiesUtils/SimDelCapsSet.xml")
        self.ModifyXml("DEL_CAP_SET","NAME",capSetName)
        self.Send()

#------------------------------------------------------------------------------
    def ResetCapSet(self,capSetName):
         """Reset the capabilities set to false
        """

         print "Resetting the capSet!"
         self.LoadXmlFile("Scripts/SimAddCapSet.xml")
         self.ModifyXml("ADD_CAP_SET","NAME",capSetName)
         self.ModifyXml("ADD_CAP_SET","AUDIO_SIREN14","false")
         self.ModifyXml("ADD_CAP_SET","AUDIO_G7221","false")
         self.ModifyXml("ADD_CAP_SET","AUDIO_G722","false")
         self.ModifyXml("ADD_CAP_SET","AUDIO_G7231","false")
         self.ModifyXml("ADD_CAP_SET","AUDIO_G729","false")
         self.ModifyXml("ADD_CAP_SET","AUDIO_G728","false")
         self.ModifyXml("ADD_CAP_SET","AUDIO_G711","false")
         self.ModifyXml("ADD_CAP_SET","VIDEO_H264","false")
         self.ModifyXml("ADD_CAP_SET","VIDEO_H263","false")
         self.ModifyXml("ADD_CAP_SET","VIDEO_VP8","false") ##N.A. DEBUG VP8

         self.Send()


#------------------------------------------------------------------------------
    def SetSystemTime(self, newTime):
        print "set system time"
        self.LoadXmlFile('Scripts/SetSystemTime.xml')
        self.ModifyXml("MCU_TIME", "MCU_BASE_TIME",newTime)
        self.Send()

#------------------------------------------------------------------------------

# When Process runs under Valgring in the Links Dir we will see
# Link of the process with the process name
    def IsProcessUnderValgrind( self, processName ):
        path_to_watch = "Links"
        bFound = False
        file_list = os.listdir ( path_to_watch )
        print file_list
        if processName in file_list:
            bFound = True
        return bFound

#------------------------------------------------------------------------------
#  check a list of process are under valgrind
    def IsAnyProcessUnderValgrind( self, processlist ):
        bFound = False
        for processName in processlist:
            bFound = self.IsProcessUnderValgrind( processName )
            if(bFound):
            	return bFound
        return bFound

#------------------------------------------------------------------------------

    def Sleep(self, secondsToSleep):
        index = 0
        sys.stdout.write("Sleep " + str(secondsToSleep) + " seconds : ")
        sys.stdout.flush()
        while index < secondsToSleep:
            sys.stdout.write(".")
            sys.stdout.flush()
            sleep(1)
            index = index + 1
        print ""


#check if imported


#------------------------------------------------------------------------------
    def AddIpV6Party(self,confid, partyname, partyFile,expected_status="Status OK"):
        """Add a new party.

        confid - destination conference.
        partyname - party name.
        partyFile - xml file which will be used to define the party
        """
        print "Adding Party..." + partyname
        self.LoadXmlFile(partyFile)
        self.ModifyXml("PARTY","NAME",partyname)
        self.ModifyXml("ADD_PARTY","ID",confid)
        self.Send(expected_status)    

#------------------------------------------------------------------------------
    def AddProfileWithTelepresenceLayoutMode(self, profileName, telepresenceModeConfiguration, telepresenceLayoutMode, fileName="Scripts/TelepresenceLayoutsProf.xml"):
        """Create new Profile.
        
        profileName - name of profile to be created.
        fileName - XML file
        """
        print "Adding new Profile..."
        self.LoadXmlFile(fileName)
        self.ModifyXml("RESERVATION","NAME", profileName)
        self.ModifyXml("RESERVATION","TELEPRESENCE_MODE_CONFIGURATION", telepresenceModeConfiguration)
        self.ModifyXml("RESERVATION","TELEPRESENCE_LAYOUT_MODE", telepresenceLayoutMode)
        self.Send()
        ProfId = self.GetTextUnder("RESERVATION","ID")
        print "Profile, named: " + profileName + " ,ID = " + ProfId + ", is added"
        if (self.IsProcessUnderValgrind("Resource")):
            sleep(1)
        return ProfId   
#------------------------------------------------------------------------------
if __name__ == '__main__':
    help(McmsConnection)


