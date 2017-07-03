#!/mcms/python/bin/python
#-LONG_SCRIPT_TYPE
# For list of profiles look at RunTest.sh
#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_4

#############################################################################
# Test Script For HD
# Date: 22/09/06
# By  : Inga
# Re-Write date = 27/8/13
# Re-Write name = Uri A.
#############################################################################

from ConfUtils.ConfCreation import *

import string 

#------------------------------------------------------------------------------

def TestHD(connection, num_retries):
    
    ConfActionsClass = ConfCreation() # conf util class
    
    # add profile
    print "Adding HD Profile"
    prof_id = ConfActionsClass.AddHdProfile(connection,"HD_profile_1920","1920", "Scripts/HD/XML/AddHdProfile.xml")
    
    # Add conf
    print "Adding Conf..."
    confName = "HD_conf_1920"
    ConfActionsClass.CreateConf(connection, confName, 'Scripts/ConfTamplates/AddConfTemplate.xml', "NONE", prof_id)
    confid = ConfActionsClass.WaitConfCreated(connection, confName)
    
    # connect parties
    print "Start connecting Parties..."
    num_of_parties = 6
    confLayoutType = "1x1" 
    
    # Adding parties
    numParties = ConfActionsClass.ConnectH323Parties(connection, confid, num_of_parties, "HdParty", "NONE")
    
    # All parties are connected
    ConfActionsClass.WaitAllOngoingConnected(connection, confid)

    # change layout
    ConfActionsClass.WaitAllOngoingChangedLayoutType(connection, confid, confLayoutType)
    
    # 6. delete Conf
    ConfActionsClass.DeleteConf(connection, confid)
    ConfActionsClass.WaitAllConferencesAreDeleted(connection)
    
    #7. delete profile
    ConfActionsClass.DelProfile(connection, prof_id, "Scripts/RemoveNewProfile.xml")


## ---------------------- Test --------------------------
McmsUtilClass = McmsConnection()
McmsUtilClass.Connect()

TestHD(McmsUtilClass,20)# retries
McmsUtilClass.Disconnect()

#------------------------------------------------------------------------------  
    
