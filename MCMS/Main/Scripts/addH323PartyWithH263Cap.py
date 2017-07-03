#!/mcms/python/bin/python

# #############################################################################
# Creating Conf with Dial In H323 participants, which has unikue capabilities 
#
# Date: 7/8/06
# By  : Eitan 
# Re-Write date = 11/8/13
# Re-Write name = Uri A.
#
############################################################################

#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_4
#-LONG_SCRIPT_TYPE

# import Python classes
from CapabilitiesSetsDefinitions import *
from McmsConnection import *
from ConfUtils.ConfCreation import *
from PartyUtils.H323PartyUtils import *

McmsUtilClass = McmsConnection() # MCMS connection and XML_API file changes class 
McmsUtilClass.Connect()
ConfActionsClass = ConfCreation() # conf util class
H323PartyUtilsClass = H323PartyUtils() # H323Party utils util class

# 1. set delay between parties - not require any more
# delayBetweenParticipants = H323PartyUtilsClass.SetDelayBetweenParties(McmsUtilClass)

# 2. Create Conf
confName = "undefConf"
ConfActionsClass.CreateConf(McmsUtilClass, confName, 'Scripts/ConfTamplates/AddConfTemplate.xml', "NONE")
confid = ConfActionsClass.WaitConfCreated(McmsUtilClass, confName)

# 3. Adding parties with caps defined in EP-SIM
numParties = H323PartyUtilsClass.AddH323DefineCapSetParties(McmsUtilClass, confName, confid, "PartyH263TestDef")

# 4. Adding parties with UNDEFINED caps in EP-SIM
numParties = H323PartyUtilsClass.AddH323UndefineCapSetParties(McmsUtilClass, confName, confid, numParties, "PartyH263TestUndef")        

# 5. delete all parties 
#H323PartyUtilsClass.DeleteAllParties(McmsUtilClass, confid, numParties) 
 
# 6. check that everything (conference and parties are deleted successfully 
ConfActionsClass.DeleteConf(McmsUtilClass, confid)
ConfActionsClass.WaitAllConferencesAreDeleted(McmsUtilClass)

H323PartyUtilsClass.SimulationDeleteAllSimParties(McmsUtilClass)
H323PartyUtilsClass.SimulationDeleteCapsSets(McmsUtilClass)# need to build capabilities class and not use McmsConnection

McmsUtilClass.Disconnect()

