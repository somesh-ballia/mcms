#!/mcms/python/bin/python

# For list of profiles look at RunTest.sh
#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_1

from AmosCapacity_functions import *

## tests to run

max_confrences_test = False
max_EQs_test = False
max_sip_factories_test = False
max_profiles_test = False
max_MRs_test = False
max_dial_out_voip_parties = True
max_dial_out_voip_parties_per_conf = False
max_dial_out_video_parties_per_conf = False

if max_EQs_test==True:
    max_EQs = 80
    c = McmsConnection()
    c.Connect()
    MaxEntryQueuesTest(c,max_EQs)
    c.Disconnect()
    
if max_sip_factories_test==True:
    max_sip_factories = 80
    c = McmsConnection()
    c.Connect()
    MaxSipFactoriesTest(c,max_sip_factories)
    c.Disconnect()

if max_profiles_test==True:
    max_profiles = 80
    c = McmsConnection()
    c.Connect()
    MaxProfilesTest(c,max_profiles)
    c.Disconnect()
 
if max_MRs_test==True:
    max_MRs = 2000
    c = McmsConnection()
    c.Connect()
    MaxMeetingRoomsTest(c,max_MRs)
    c.Disconnect()

if max_confrences_test == True:
    max_confrences = 800
    c = McmsConnection()
    c.Connect()
    MaxConfrencesTest(c,max_confrences)
    c.Disconnect()

if max_dial_out_voip_parties == True:
    c = McmsConnection()
    c.Connect()
    ## configuration
    confrences = 20
    ## participants per conf by type
    h323_voip_out = 80
    h323_voip_in = 0
    h323_video_out = 0
    h323_video_in = 0

    MaxMixedPartiesTest(c,confrences,h323_voip_out,h323_voip_in,h323_video_out,h323_video_in)
  
if max_dial_out_voip_parties_per_conf == True:
    c = McmsConnection()
    c.Connect()
    ## configuration
    confrences = 1
    ## participants per conf by type
    h323_voip_out = 800
    h323_voip_in = 0
    h323_video_out = 0
    h323_video_in = 0

    MaxMixedPartiesTest(c,confrences,h323_voip_out,h323_voip_in,h323_video_out,h323_video_in)
    
    c.Disconnect()

if max_dial_out_video_parties_per_conf == True:
    c = McmsConnection()
    c.Connect()
    ## configuration
    confrences = 8
    ## participants per conf by type
    h323_voip_out = 100
    h323_voip_in = 0
    h323_video_out = 0
    h323_video_in = 0

    MaxMixedPartiesTest(c,confrences,h323_voip_out,h323_voip_in,h323_video_out,h323_video_in)
    
    c.Disconnect()


