#!/mcms/python/bin/python

from ResourceUtilities import *
import random
class SMCUConference:
    """This is a conference class."""
    connection = None
    pubConfId = -1
    internalConfId = -1

#------------------------------------------------------------------------------  
    def Create(self, conn = None, confName="Conf1", confID = 1111, profileID = None, num_retries = 10):
            if conn != None:
                        self.connection = conn
            else: 
                        self.connection = ResourceUtilities()
                        self.connection.Connect()

            print "SMCUConference::Create() - Adding conference " + confName + " confId = " + str(confID) + " profileID = " + str(profileID)
            confFile = 'Scripts/AddCpConf.xml'
            self.connection.LoadXmlFile(confFile)
            self.connection.ModifyXml("RESERVATION","NAME",confName)
            self.connection.ModifyXml("RESERVATION","ID",confID)
            if profileID != None:
                        self.connection.ModifyXml("RESERVATION","AD_HOC_PROFILE_ID",profileID)
                        print "SMCUConference::Create - Non default profile ID:  " + str(profileID)

            #print self.connection.loadedXml.toprettyxml(encoding="utf-8")
            status = self.connection.Send()
            #print self.connection.xmlResponse.toprettyxml(encoding="utf-8")

            self.pubConfId = self.connection.GetTextUnder("RESERVATION","NUMERIC_ID")
            print "RESERVATION ID :" + self.pubConfId
            print "SMCUConference::Wait untill Conf create..."

            internalConfId = ""
            for retry in range(num_retries+1):
                        sleep(0.2)
                        status = self.connection.SendXmlFile('Scripts/TransConfList.xml',"Status OK")
                        #find internal ConfId
                        ongoing_conf_list = self.connection.xmlResponse.getElementsByTagName("CONF_SUMMARY")
                        for index in range(len(ongoing_conf_list)):  
                        	numericId = ongoing_conf_list[index].getElementsByTagName("NUMERIC_ID")[0].firstChild.data
                        	if numericId == self.pubConfId:
                                    internalConfId = ongoing_conf_list[index].getElementsByTagName("ID")[0].firstChild.data
                                    print "internal ID = " + str(internalConfId)
                                    break
                        
                        if internalConfId != "":
                                self.internalConfId = internalConfId
                                break
                        if (retry == num_retries):
                                print self.connection.xmlResponse.toprettyxml(encoding="utf-8")
                                print "MCUConference::Cannot monitor conf:" + status
                                self.connection.Disconnect()                
                                return status
                        sys.stdout.write(".")
                        sys.stdout.flush()

            print "MCUConference::Create conf with id " + str(self.pubConfId) 
            return status

#------------------------------------------------------------------------------  
 
    def CreateWRandomId(self, conn = None, confName="Conf1", profileID = None, num_retries = 10):
	confNameRand = confName + str(random.randint(1,99))
	confID = str(random.randint(1000, 999999))
	return self.Create(conn, confNameRand, confID, profileID, num_retries)
#----------------------------------------------------------
    def DeleteParty(self,partyId,expected_status="Status OK"):
	print "Delete Party ID:" + str(partyId)
	self.connection.DeleteParty(self.internalConfId, partyId, expected_status)

#------------------------------------------------------------------	
    def Disconnect(self):
	print "SMCUConference::Close Conference " + str(self.internalConfId)
	self.connection.DeleteConf(self.internalConfId)
	self.connection.WaitAllConfEnd()
