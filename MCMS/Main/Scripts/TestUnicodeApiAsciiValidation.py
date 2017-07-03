#!/mcms/python/bin/python
# -*- coding: utf-8 -*-

from McmsConnection import *


def CheckService1elem(conn, elemName, expectedName):
    strResult = str(connection.loadedXml.getElementsByTagName(elemName)[0].firstChild)
    if(strResult <> "None"):
        print strResult + " <> None"
        sys.exit(1)

def CheckService2elem(conn, elemName1, elemName2, expectedName):
    strResult = str(connection.loadedXml.getElementsByTagName(elemName1)[0].getElementsByTagName(elemName2)[0].firstChild)
    if(strResult <> "None"):
        print strResult + " <> None"
        sys.exit(1)    


def TestIpServiceFields(connection):

    print "Test IP Service ASCII validation"
    
    connection.LoadXmlFile("Scripts/UpdateIPService.xml")


    utf8SendName = u"Олигархи - удивительно сообразительные зверьки"
    expectedName = ""
    
    connection.ModifyXml("IP_SERVICE","GATEKEEPER_NAME", utf8SendName)
    connection.ModifyXml("IP_SERVICE","ALT_GATEKEEPER_NAME", utf8SendName)
    connection.ModifyXml("ALIAS","NAME", utf8SendName)
    connection.ModifyXml("IP_SPAN","HOST_NAME", utf8SendName)
    connection.ModifyXml("DNS","DOMAIN_NAME", utf8SendName)
    connection.ModifyXml("SIP","NAME", utf8SendName)
    connection.ModifyXml("ALTERNATE_SIP_SERVER","DOMAIN_NAME", utf8SendName)
    
    connection.Send("Status OK")
    
    connection.LoadXmlFile("Cfg/IPServiceListTmp.xml")
    
    CheckService1elem(connection, "GATEKEEPER_NAME", expectedName)
    CheckService1elem(connection, "ALT_GATEKEEPER_NAME", expectedName)
    CheckService2elem(connection, "ALIAS", "NAME", expectedName)
    CheckService2elem(connection, "IP_SPAN","HOST_NAME", expectedName)
    CheckService2elem(connection, "DNS","DOMAIN_NAME", expectedName)
    CheckService2elem(connection, "SIP","NAME", expectedName)
    CheckService2elem(connection, "ALTERNATE_SIP_SERVER","DOMAIN_NAME", expectedName)
    
    print "All not ascii strings were rejected as expected, Very Good"
    print
    

def TestCfgFlields(connection):

    print "Test Cfg ASCII validation"

    connection.LoadXmlFile('Scripts/SetSystemCfg.xml')

#    utf8SendName = "cucu-lulu"
#    utf8SendName = u"שגיא"
    utf8SendName =  u"Олигархи - удивительно сообразительные зверьки" 
    
    connection.ModifyXml("CFG_PAIR","DATA", utf8SendName)

 #   print connection.loadedXml.toprettyxml(encoding="utf-8")

    connection.Send("STATUS_NODE_VALUE_NOT_ASCII")

    print "All not ascii Cfg flags were rejected as expected, Very Good"
    print
    

def TestCreateDirectoryFields(connection):

    print "Test create directory"

    connection.LoadXmlFile('Scripts/CreateDirectory.xml')

    utf8SendName = u"Олигархи - удивительно сообразительные зверьки"

    connection.ModifyXml("CREATE_DIRECTORY","PATH", utf8SendName)
   
    connection.Send("STATUS_NODE_VALUE_NOT_ASCII")

    print "Bad request for Create Directory was rejected, Very Good"
    print
    

def TestRemoveDirectoryFields(connection):

    print "Test remove directory"

    connection.LoadXmlFile('Scripts/RemoveDirectory.xml')

    utf8SendName = u"Олигархи - удивительно сообразительные зверьки"

    connection.ModifyXml("REMOVE_DIRECTORY","PATH", utf8SendName)
    
    connection.Send("STATUS_NODE_VALUE_NOT_ASCII")

    print "Bad request for Remove  Directory was rejected, Very Good"
    print
    




#---------------------------------------------------------------------------
#   Test
#---------------------------------------------------------------------------

connection = McmsConnection()
connection.Connect()


TestIpServiceFields(connection)
TestCfgFlields(connection)
TestCreateDirectoryFields(connection)
TestRemoveDirectoryFields(connection)

