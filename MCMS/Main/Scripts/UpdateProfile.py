#!/mcms/python/bin/python

# For list of profiles look at RunTest.sh
#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_8
#-LONG_SCRIPT_TYPE
#*PROCESSES_NOT_FOR_VALGRIND=Authentication Resource Faults CertMngr GideonSim EndpointsSim Logger QAAPI MplApi CSApi EncryptionKeyServer
#############################################################################
# Test Script which adds profile, then updates it and deletes it.
#                 UpdateProfile.py
#
# Date: 12/12/11
# By  : Yossi G.

#############################################################################

from McmsConnection import *

###------------------------------------------------------------------------------
def CraeteAndUpdateProfile(connection, numRetries):

    print "Starting test UpdateProfile ... "
#add a new profile
    success = False
    connection.LoadXmlFile("Scripts/UpdateProfile/Addprofile.xml")
    my_rate = 1472
    connection.ModifyXml("RESERVATION","TRANSFER_RATE", my_rate)
    print "Adding new Profile, TRANSFER_RATE: " + str(my_rate)
    connection.Send("")
    sleep(5)
    
    ## sleep(1) #for profile to be monitored
    for retry in range(numRetries):
        sleep(1)
        connection.SendXmlFile('Scripts/UpdateProfile/GetProfileList.xml')
        profile_list = connection.xmlResponse.getElementsByTagName("PROFILE_SUMMARY")
        
        for index in range(len(profile_list)):
            TRANSFER_RATE = profile_list[index].getElementsByTagName("TRANSFER_RATE")[0].firstChild.data
            currentProfName = profile_list[index].getElementsByTagName("NAME")[0].firstChild.data
            profId = profile_list[index].getElementsByTagName("ID")[0].firstChild.data

            if str(my_rate) == TRANSFER_RATE:
                print "Profile " + currentProfName + " added profile list id " + profId + " with rate " + str(TRANSFER_RATE)
                success = True
                break # for profile_list
        if success == True:
            break # for numRetries
   
    if success == False:
        connection.Disconnect()
        sys.exit("Failed to add the profile!!!")
		
	
    connection.LoadXmlFile("Scripts/UpdateProfile/UpdateProfile.xml")
    my_rate = 768
    print "Updating the Profile to be with with rate " + str(my_rate) 
    connection.ModifyXml("RESERVATION","ID", profId)    
    connection.ModifyXml("RESERVATION","TRANSFER_RATE", my_rate)
    connection.ModifyXml("RESERVATION","NAME",currentProfName)
    connection.Send("")
    sleep(5)
    success = False
    for retry in range(numRetries):
        sleep(1)
        connection.SendXmlFile('Scripts/UpdateProfile/GetProfileList.xml')
        profile_list = connection.xmlResponse.getElementsByTagName("PROFILE_SUMMARY")
        for index in range(len(profile_list)):
            TRANSFER_RATE = profile_list[index].getElementsByTagName("TRANSFER_RATE")[0].firstChild.data
            updateId = profile_list[index].getElementsByTagName("ID")[0].firstChild.data
            if str(my_rate) == TRANSFER_RATE and profId == updateId:
                print "Profile " + currentProfName + " ID "+ profId + " updated to be with rate " + TRANSFER_RATE
                success = True
                break # for profile_list
        if success == True:
            break # for num_retries
            
    if success == False:
#        connection.Disconnect()
#        sys.exit("Failed to update the profile!!!")  
         print "Failed to update the profile!!!"
            
#Remove the profile
    print "removing the profile in 5 seconds"
    sleep(5)
    connection.DelProfile(profId, "Scripts/UpdateProfile/RemoveNewProfile.xml")


##--------------------------------------- TEST ---------------------------------

c = McmsConnection()
c.Connect()

CraeteAndUpdateProfile(c, 10) #num of retries

c.Disconnect()
##------------------------------------------------------------------------------

