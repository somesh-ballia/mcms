from ResourceUtilities import *
from SMCUConference import *

class SMcuCallGenConf:
    	"""This is a conference class."""
    	cgConf = None
    	cgIP = None 
	profId = None
#--------------------

	def Create(self, name ="CG_conf", confID = 1111, cg_ip = "127.0.0.1", profileRate = "default"):
	  print "SMcuCallGenConf:Create(name =" + name +", cg_ip= " + cg_ip +")"
          if cg_ip == "127.0.0.1":
            try:
                self.cgIP = os.environ["CALLGEN_IP"]
            except KeyError:
	        self.cgIP = cg_ip
          print "SMcuCallGenConf:Create - cg_ip set to " + self.cgIP
	  c = ResourceUtilities()
	  c.Connect(self.cgIP)
	#profile
	  if profileRate != "default":
		profileName = "Profile" + str(profileRate)
		profId = c.TryAddProfileWithRate(profileName, profileRate, "sharpness")
		if profId == -1:
   			err = "Failed to create profile with bit rate " + profileRate
   			ScriptAbort(err)
	  self.profId = profId

	  self.cgConf = SMCUConference()
	  #---Create Conference
	  self.cgConf.Create(c, name, confID, profId)
	  self.profId = profId
	  print "Added CG conference ID: " + str(self.cgConf.pubConfId)

#----------------------

     # Add Dial In Party
	def AddDialInParty(self, mcuConf, name, partyip, brate, maxres = "auto"): 
	  print "AddDialInParty(mcuConfID =" + mcuConf.pubConfId +",... partyip= " + partyip +" bit rate = " + brate + ")"
          partyid = -1
          conn = self.cgConf.connection
          cg_internalConfId = self.cgConf.internalConfId
          cg_pubConfId = self.cgConf.pubConfId
          conn.LoadXmlFile("Scripts/SMCUDialOutParty.xml")
          partyname = name+"_##FORCE_MEDIA_AG722"
          print "Calling to conf: " + str(mcuConfID) + "@" + partyip
          conn.ModifyXml("PARTY","NAME",partyname)
          conn.ModifyXml("PARTY","IP",partyip)
          conn.ModifyXml("ADD_PARTY","ID",cg_internalConfId)
          conn.ModifyXml("ALIAS","NAME",mcuConf.pubConfId)
	  conn.ModifyXml("PARTY","VIDEO_BIT_RATE",brate)
     	  conn.ModifyXml("PARTY","VIDEO_PROTOCOL","h264")
    	  conn.ModifyXml("PARTY","MAX_RESOLUTION", maxres)
    	  print conn.loadedXml.toprettyxml(encoding="utf-8")
    	  status = conn.Send()
    	  partyid = conn.GetPartyId(cg_internalConfId,partyname)
    	  conn.WaitAllOngoingConnected(cg_internalConfId,20)
    	  conn.WaitAllOngoingNotInIVR(cg_internalConfId) 
    	  return partyid
	#--------

	def DeleteAllConf(self):
	  print "**** Delete all CG parties...  ************"
	  self.cgConf.connection.DeleteAllConf()


	def Disconnect(self):
	  print "**** Delete all CG parties...  ************"
	  if cg_ip == "127.0.0.1":
		self.cgConf.connection.DelProfile(self.profId)
	  self.cgConf.connection.DeleteAllConf()
	  self.cgConf.connection.Disconnect()
