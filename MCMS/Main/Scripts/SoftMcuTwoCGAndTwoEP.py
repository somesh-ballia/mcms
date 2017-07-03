#!/mcms/python/bin/python

# # NAME: SoftMcuDialInFromCG
# # OWNER: SOFT
# # GROUP: VideoParty
# # NIGHT: FALSE
# # VERSION: FALSE
# # VRESTART_AFTER: FALSE

# Add Dial In Party
def AddDialInParty(cg_conf, conf, name, partyip, brate, maxres = "auto"): 
    print "AddDialInParty:Calling to conf " + str(conf.pubConfId) + "@" + partyip
    partyid = -1
    conn = cg_conf.connection
    cg_internalConfId = cg_conf.internalConfId
    cg_pubConfId = cg_conf.pubConfId
    conn.LoadXmlFile("Scripts/SMCUDialOutParty.xml")
    partyname = name+"_##FORCE_MEDIA_AG722"
    conn.ModifyXml("PARTY","NAME",partyname)
    conn.ModifyXml("PARTY","IP",partyip)
    conn.ModifyXml("ADD_PARTY","ID",cg_internalConfId)
    conn.ModifyXml("ALIAS","NAME",conf.pubConfId)
    conn.ModifyXml("PARTY","VIDEO_BIT_RATE",brate)
    conn.ModifyXml("PARTY","VIDEO_PROTOCOL","h264")
    conn.ModifyXml("PARTY","MAX_RESOLUTION", maxres)
    #print conn.loadedXml.toprettyxml(encoding="utf-8")
    status = conn.Send("")
    if status != "Status OK":
	print "AddDialInParty: Call Gen dialout with error - " + status
    partyid = conn.GetPartyId(cg_internalConfId,partyname)
    return partyid

#----------------------

def CreateEndPoint(conf, EP_EnvVar, name, sigpro, videopro, brate):
    partyid = -1
    try:
	ip=os.environ[EP_EnvVar]
	print "CreateEndPoint:Adding party " + name + " IP: " + ip
	ep = SMCUVideoParty()
	status =ep.Add(conf, name, ip, sigpro, videopro, brate)
	if status == "Statuss OK":
	   partyid = ep.partyid
	   print "Added party ID: " + partyid

    except KeyError:
	print "Error: cannot get " + EP_EnvVar + " env variable"

    return partyid       

#--------------------------------
#from McmsConnection import *
from ResourceUtilities import *
from SystemCapacityFunctions import *

try:
    CG_IP=os.environ["CALLGEN_IP"]
except KeyError:
    CG_IP="10.227.2.137"
    print "cannot get CALLGEN_IP environment"
print "CallGenerator IP set to " + CG_IP

try:
    MCU_IP=os.environ["my_ip"]
except KeyError:
    print "Error: cannot get my_ip env variable"
    sys.exit("cannot get my_ip environment")


#CallGen connection
cg_c = ResourceUtilities()
cg_c.Connect(CG_IP)
profileName = "Profile1024"
#cg_profId = cg_c.TryAddProfileWithRate(profileName,1024,"sharpness")
#if cg_profId == -1:
#	err = "Failed to crate " + profileName
#	ScriptAbort(err)
#SoftMCU connection
c = ResourceUtilities()
c.Connect()
c.DeleteAllConf()

mcu_profId = c.TryAddProfileWithRate(profileName,1024,"sharpness")

if mcu_profId == -1:
	err = "Failed to crate " + profileName
	ScriptAbort(err)
print "**** Delete all MCU parties... ************" 
c.DeleteAllConf()
#print "**** Delete all CG parties...  ************"
#cg_c.DeleteAllConf()
from SMCUConference import *
from SoftMcuVideoParty import *
#CG conf
cg_conf = SMCUConference()
#---Create Conference
#cg_conf.Create(cg_c, "CG_Conf", "1111", cg_profId)
cg_conf.CreateWRandomId(cg_c, "CG_Conf" )
print "Added CG conference ID: " + str(cg_conf.pubConfId)

#mcu conf
conf = SMCUConference()
#---Create Conference
conf.Create(c, "VideoConf", "12345", mcu_profId)
print "Added MCU conference ID: " + str(conf.pubConfId)
print "Changing layout"
c.ChangeConfLayoutType(conf.internalConfId, "2x2")

#create dial in party
num_of_calls = 2
print "Creating " + str(num_of_calls)+ " cal(s) from a call generator:"
#brate = "1024"
brate = "384"
for itr in range(num_of_calls):
  range_low = 100*itr
  range_hi = 100*itr + 99
  party_num = random.randint(range_low,range_hi)
  party_name = "p" + str(party_num)
  partyid = AddDialInParty(cg_conf, conf, party_name, MCU_IP, brate, "cif")
  if partyid != -1:
	print "Added party " + party_name + " ID: " + partyid
	sleep(2)
  else:
	ScriptAbort("Failed to add party " + party_name + " ID: " + partyid)
#wait until CallGen calls are connected
cg_conf.connection.WaitAllOngoingConnected(cg_conf.internalConfId,num_of_calls*30)
cg_conf.connection.WaitAllOngoingNotInIVR(cg_conf.internalConfId)


sigpro = "h323"
videopro = "h264"
#create real EP1
#party1_id = CreateEndPoint(conf, "EP1", "EP1", sigpro, videopro, brate)
p1 = SMCUVideoParty()
p1.TryAddFromEP1env(conf, "EP1", "EP1", sigpro, videopro, brate)

#create real EP2
#party2_id = CreateEndPoint(conf, "EP2", "EP2")
p2 = SMCUVideoParty()
p2.TryAddFromEP1env(conf, "EP2", "EP2", sigpro, videopro, brate)

#let it play a while
sleep(40) 

#conf.Disconnect()
#cg_conf.Disconnect()

#c.Disconnect()
#cg_c.Disconnect()
print "**** Delete all MCU parties... ************" 
c.DeleteAllConf()
c.DelProfile(mcu_profId)
#print "**** Delete CG conf...  ************"
cg_c.DeleteConf(cg_conf.internalConfId)
#cg_c.DeleteAllConf()
#cg_c.DelProfile(cg_profId)
