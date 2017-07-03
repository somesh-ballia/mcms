#!/mcms/python/bin/python

#############################################################################
# Creating Conf with a 4 defined Dial In SIP participants
#
# Date: 23/01/05
# By  : Ron S.

#############################################################################

from subprocess import call
import os
from McmsConnection import *
from RoomSwitch import *

##--------------------------------------- TEST ---------------------------------
os.environ["LD_LIBRARY_PATH"] = "/mcms/Bin"
os.environ["SASL_PATH"] = "/mcms/Bin"

call(["Bin/McuCmd", "set","all","ITP_CERTIFICATION","YES"])
call(["Bin/McuCmd", "set","all","MANAGE_TELEPRESENCE_ROOM_SWITCH_LAYOUTS","YES"])
        
c = McmsConnection()
c.Connect()

print "creating profile"
profileName = "SpeakerPriorityProf"
## telepresenceModeConfiguration = "auto"
telepresenceModeConfiguration = "yes"
telepresenceLayoutMode = "cp_speaker_priority"

profId = c.IsProfileNameExists(profileName)
if profId == -1:
    profId = c.AddProfileWithTelepresenceLayoutMode(profileName,telepresenceModeConfiguration,telepresenceLayoutMode)
    
print "Profile created: " + profileName + " with id: " + str(profId)
sleep(1)
print "creating Conference"
conf_name = "Speaker_Logic"
c.CreateConfFromProfile(conf_name, profId)
conf_id = c.WaitConfCreated(conf_name)
print "Conference created: " + conf_name + " with id: " + str(conf_id)
sleep(1)

IsConfPartyUnderValgrind = False; 

if (c.IsProcessUnderValgrind("ConfParty")):
     IsConfPartyUnderValgrind = True;

AddSIPPartyDialOut(c,conf_id,"Nizar_SIM_TIP",1)

AddSIPPartyDialOut(c,conf_id,"Ron_SIM_TIP",5)

AddSIPPartyDialOut(c,conf_id,"Anat_SIM_TIP",10)

AddSIPPartyDialOut(c,conf_id,"Jawad_SIM_TIP",15)

AddSIPPartyDialOut(c,conf_id,"Anat",20)

AddSIPPartyDialOut(c,conf_id,"Nizar",21)

AddSIPPartyDialOut(c,conf_id,"Jawad",22)

#RPX EP 2 screens
AddH323PartyDialIn(c,conf_name,2,"Ron_RPX", "Polycom RPX")

#RPX EP 3 screens
AddH323PartyDialIn(c,conf_name,3,"Nizar_RPX", "Polycom RPX")

#RPX EP 3 screens
AddH323PartyDialIn(c,conf_name,3,"Tsahi_RPX", "Polycom RPX")

#RPX EP 4 screens
AddH323PartyDialIn(c,conf_name,4,"Jawad_RPX", "Polycom RPX")

AddH323PartyDialIn(c,conf_name,4,"Anat_RPX", "Polycom RPX")

#c.Disconnect()

##-----------------------------------------------------------------------------------------------

