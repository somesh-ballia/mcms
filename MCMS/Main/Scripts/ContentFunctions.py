#!/mcms/python/bin/python
#FUNCTIONS fo Content Scripts
#  
# Date: 12/10/06
# By  : Yoella
#############################################################################
from McmsConnection import *
import string 

#------------------------------------------------------------------------------
def AddDialInParty(connection,partyname,partyid,confName,confid,num_retries,capName="FULL CAPSET"):
    connection.SimulationAddH323Party(partyname, confName,capName)
    connection.SimulationConnectH323Party(partyname)
    #Get the party id
    currPartyID = connection.GetCurrPartyID(connection,confid,partyid,num_retries)
    if (currPartyID < 0):
       connection.Disconnect()                
       sys.exit("Error:Can not find partry id of party: "+partyname)
    print "party id="+str(currPartyID)+" found in Conf"
  
#------------------------------------------------------------------------------
def ActivatePresentation(connection,partyName):
    
  connection.LoadXmlFile('Scripts/ContentPresentation/SimH239TokenRequest.xml')
  connection.ModifyXml("H239_TOKEN_REQUEST","PARTY_NAME",partyName)
  connection.Send()  
  
#------------------------------------------------------------------------------
def ActivatePresentationByPartyNo(connection, partyName, confId, partyNo):
    
  connection.LoadXmlFile('Scripts/ContentPresentation/SimH239TokenRequest.xml')
  connection.ModifyXml("H239_TOKEN_REQUEST","PARTY_NAME",partyName)
  connection.ModifyXml("H239_TOKEN_REQUEST","CONF_ID",confId)
  connection.ModifyXml("H239_TOKEN_REQUEST","PARTY_NO",partyNo)
  connection.Send()  
  
#------------------------------------------------------------------------------
def ClosePresentation(connection,partyName):
    
  connection.LoadXmlFile('Scripts/ContentPresentation/SimH239TokenRelease.xml')
  connection.ModifyXml("H239_TOKEN_RELEASE","PARTY_NAME",partyName)
  connection.Send()     
   

#------------------------------------------------------------------------------
def CheckPresentationSpeaker(connection,partyName,confid,partyid,num_retries):
  #make sure the party got the content token
  print "Checking that Presentation Party is : "+ partyName + " , partyId="+ str(partyid)
  sleep(1)
  connection.LoadXmlFile('Scripts/ContentPresentation/TransConf2.xml')
  connection.ModifyXml('GET','ID',confid)
  for retry in range(num_retries+1):
      connection.Send()
      contentSourceIdList = connection.xmlResponse.getElementsByTagName('EPC_CONTENT_SOURCE_ID')
      activePartyId= contentSourceIdList[0].firstChild.data
           
      if (activePartyId != ""  and (string.atoi(activePartyId) == partyid) ):
         print partyName + " with id: "+str(partyid)+ " is the current content token holder"
         break
      if (retry == num_retries):
         print
         connection.Disconnect()                
         sys.exit("Setting Party:"+ partyName +" as Owner of Content Token Failed!!!")
      sys.stdout.write(".")
      print
      sys.stdout.flush()
      sleep(1)
      
#------------------------------------------------------------------------------      
def CheckNoPresentation(connection,confid,num_retries):
  #make sure NO party is token holder
  print "Checking that NO token holder in conf"
  connection.LoadXmlFile('Scripts/ContentPresentation/TransConf2.xml')
  connection.ModifyXml('GET','ID',confid)
  for retry in range(num_retries+1):
      connection.Send()
      contentSourceIdList = connection.xmlResponse.getElementsByTagName('EPC_CONTENT_SOURCE_ID')
      activePartyId= contentSourceIdList[0].firstChild.data
      if (activePartyId == ""  or (string.atoi(activePartyId) == (-1)) ) :
         print "Conf has NO content token holder"
         break
      if (retry == num_retries):
         print
         connection.Disconnect()                
         sys.exit("Release of Content Token Failed!!!")
      sys.stdout.write(".")
      sys.stdout.flush()
      sleep(1)

#------------------------------------------------------------------------------
def DelSimParty(connection,partyName):
    print "Deleting SIM party "+partyName+"..."
    connection.LoadXmlFile("Scripts/SimDel323Party.xml")
    connection.ModifyXml("H323_PARTY_DEL","PARTY_NAME",partyName)
    connection.Send()
    return

