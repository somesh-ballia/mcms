#!/mcms/python/bin/python


from McmsConnection import *
import sys
       
#--------------------------
def SendXmlFileUsingScheme(connection, xml_file,expected_statuses="STATUS_OK",encoding="",python_encoding=""):
   
    xmlstring = ""
    infile = open(xml_file, "r" )
    for line in infile:
        xmlstring += line

    infile.close()

    connection.SendXmlAction(xmlstring,expected_statuses, encoding,python_encoding)
    
#------------------------------------------------------------





c = McmsConnection()

c.Connect()

SendXmlFileUsingScheme(c,"Scripts/Unicode/ASCI.xml","Status OK","")
SendXmlFileUsingScheme(c,"Scripts/Unicode/ASCI.xml","Status OK","UTF-8")
SendXmlFileUsingScheme(c,"Scripts/Unicode/ASCI.xml","Status OK","ASCII")
SendXmlFileUsingScheme(c,"Scripts/Unicode/ASCI.xml","Cannot encode unknown character set","BLABLABLA","UTF-8")


SendXmlFileUsingScheme(c,"Scripts/Unicode/UTF8.xml","Status OK","UTF-8")
SendXmlFileUsingScheme(c,"Scripts/Unicode/UTF8.xml","Status OK")
SendXmlFileUsingScheme(c,"Scripts/Unicode/UTF8.xml","Failed to convert encoding","ASCII")
SendXmlFileUsingScheme(c,"Scripts/Unicode/UTF8.xml","Failed to convert encoding","ISO_8859","UTF-8")

SendXmlFileUsingScheme(c,"Scripts/Unicode/extended_asci_hebrew.xml","Failed to convert encoding","ASCII")
SendXmlFileUsingScheme(c,"Scripts/Unicode/extended_asci_hebrew.xml","Status OK","ISO_8859-8")
SendXmlFileUsingScheme(c,"Scripts/Unicode/extended_asci_hebrew.xml","Failed to validate encoding","UTF-8")
SendXmlFileUsingScheme(c,"Scripts/Unicode/extended_asci_hebrew.xml","Failed to validate encoding","")


#sleep(1)
#c.DeleteAllConf()
#c.WaitAllConfEnd(20 * timeuot )




c.Disconnect()
