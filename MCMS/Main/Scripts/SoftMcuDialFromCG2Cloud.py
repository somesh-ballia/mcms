#!/mcms/python/bin/python

# # NAME: SoftMcuDialInFromCG
# # OWNER: SOFT
# # GROUP: VideoParty
# # NIGHT: FALSE
# # VERSION: FALSE

# Add Dial In Party
def AddDialInParty(cg_conf, conf, name, partyip, brate): 
    print "AddDialInParty:Calling to conf " + str(conf.pubConfId) + "@" + partyip
    partyid = -1
    conn = cg_conf.connection
    cg_internalConfId = cg_conf.internalConfId
    cg_pubConfId = cg_conf.pubConfId
    conn.LoadXmlFile("/mcms/Scripts/SMCUDialOutParty.xml")
    partyname = name+"_##FORCE_MEDIA_AG722"
    conn.ModifyXml("PARTY","NAME",partyname)
    conn.ModifyXml("PARTY","IP",partyip)
    conn.ModifyXml("ADD_PARTY","ID",cg_internalConfId)
    conn.ModifyXml("ALIAS","NAME",conf.pubConfId)
    conn.ModifyXml("PARTY","VIDEO_BIT_RATE",brate)
    conn.ModifyXml("PARTY","VIDEO_PROTOCOL","h264")
    #print conn.loadedXml.toprettyxml(encoding="utf-8")
    status = conn.Send()
    partyid = conn.GetPartyId(cg_internalConfId,partyname)
    conn.WaitAllOngoingConnected(cg_internalConfId,20)
    conn.WaitAllOngoingNotInIVR(cg_internalConfId) 
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
    MCU_IP=os.environ["MCU_IP"]
except KeyError:
    print "Error: cannot get MCU_IP env variable"
    sys.exit("cannot get MCU_IP environment")

try:
    MCU_CONF=os.environ["MCU_CONF"]
except KeyError:
    print "Error: cannot get MCU_CONF env variable"
    sys.exit("cannot get MCU_CONF environment")


try:
    MCU_PWD=os.environ["MCU_PWD"]
except KeyError:
    print "Error: cannot get MCU_PWD env variable"
    sys.exit("cannot get MCU_PWD environment")


#CallGen connection
cg_c = ResourceUtilities()
cg_c.Connect(CG_IP, "80", "Status OK", "SUPPORT", "SUPPORT")
profileName = "Profile768"
cg_profId = cg_c.TryAddProfileWithRate(profileName,768,"sharpness")
if cg_profId == -1:
	err = "Failed to crate " + profileName
	ScriptAbort(err)

#SoftMCU connection
c = ResourceUtilities()
#c.Connect(MCU_IP, 8080, "Status OK", "SUPPORT", MCU_PWD)
c.Connect(MCU_IP, 80, "Status OK", "SUPPORT", MCU_PWD)

print "**** Prepare CG...  ************"
cg_c.DeleteAllConf()
from SMCUConference import *
from SoftMcuVideoParty import *
#CG conf
cg_conf = SMCUConference()
#---Create Conference
cg_conf.Create(cg_c, "CG_Test_cloud_Conf", "1111", cg_profId)
print "Added CG conference ID: " + str(cg_conf.pubConfId)

#mcu conf
conf = SMCUConference()
conf.pubConfId = MCU_CONF

#create dial in party
num_of_calls = 1
print "Creating " + str(num_of_calls)+ " cal(s) from a call generator:"
for party_num in range(num_of_calls):
  party_name = "p" + str(party_num)
  partyid = AddDialInParty(cg_conf, conf, party_name, MCU_IP, "768")
  if partyid != -1:
	print "Added party " + party_name + " ID: " + partyid
	sleep(1)
  else:
	ScriptAbort("Failed to add party " + party_name + " ID: " + partyid)


#let it play a while
sleep(10) 

#conf.Disconnect()
#cg_conf.Disconnect()

#c.Disconnect()
#cg_c.Disconnect()


print "**** Delete CG parties...  ************"
cg_c.DeleteAllConf()
cg_c.DelProfile(cg_profId)
