#!/mcms/python/bin/python/home/ron/versions/ron_MCMS-Amos-V1.0_dev/vob/MCMS/Main/
#############################################################################
#FUNCTIONS fo Amos Capacity Scripts
#  
# Date: 04/2008
# By  : Ron
#############################################################################
from McmsConnection import *
from time import *




#------------------------------------------------------------------------------
# Amos capacity
#------------------------------------------------------------------------------

## def MaxMixedPartiesTest(self,num_of_confrences=1,
##                         num_of_h323_voip_dial_out_per_conf=1,num_of_h323_voip_dial_in_per_conf=0,
##                         num_of_h323_video_dial_out_per_conf=0,num_of_h323_video_dial_in_per_conf=0,
##                         num_of_pstn_dial_out_per_conf=0,num_of_pstn_dial_in_per_conf=0,
##                         num_of_isdn_dial_out_per_conf=0,num_of_isdn_dial_in_per_conf=0,
##                         num_of_sip_voip_dial_out_per_conf=0,num_of_sip_voip_dial_in_per_conf=0,
##                         num_of_sip_video_dial_out_per_conf=0,num_of_sip_video_dial_in_per_conf=0,
##                         confFile='Scripts/AddVoipConf.xml',
##                         h323VoipDialOutPartyFile='Scripts/AddVoipParty1.xml',
##                         h323VoipDialInPartyFile='Scripts/SimAdd323Party.xml',
##                         h323VideoDialOutPartyFile='Scripts/AddVoipParty1.xml',
##                         h323VideoDialInPartyFile='Scripts/SimAdd323Party.xml',
##                         pstnDialOutPartyFile='Scripts/AddVoipParty1.xml',
##                         pstnDialInPartyFile='Scripts/AddVoipParty1.xml',
##                         isdnDialOutPartyFile='Scripts/AddVoipParty1.xml',
##                         isdnDialInPartyFile='Scripts/AddVoipParty1.xml',
##                         sipVoipDialOutPartyFile='Scripts/AddVoipParty1.xml',
##                         sipVoipDialInPartyFile='Scripts/SimAdd323Party.xml',
##                         sipVideoDialOutPartyFile='Scripts/AddVoipParty1.xml',
##                         sipVideoDialInPartyFile='Scripts/SimAdd323Party.xml',
##                         deleteAllConf="FALSE"):

##     ## constants
##     num_retries = 10



#!/mcms/python/bin/python/home/ron/versions/ron_MCMS-Amos-V1.0_dev/vob/MCMS/Main/
#############################################################################
#FUNCTIONS fo Amos Capacity Scripts
#  
# Date: 04/2008
# By  : Ron
#############################################################################
from McmsConnection import *
from time import *

#------------------------------------------------------------------------------
# Amos capacity
#------------------------------------------------------------------------------


def MaxMixedPartiesTest(self,num_of_confrences=1,
                        num_of_h323_voip_dial_out_per_conf=1,num_of_h323_voip_dial_in_per_conf=0,
                        num_of_h323_video_dial_out_per_conf=0,num_of_h323_video_dial_in_per_conf=0,
                        num_of_pstn_dial_out_per_conf=0,num_of_pstn_dial_in_per_conf=0,
                        num_of_isdn_dial_out_per_conf=0,num_of_isdn_dial_in_per_conf=0,
                        num_of_sip_voip_dial_out_per_conf=0,num_of_sip_voip_dial_in_per_conf=0,
                        num_of_sip_video_dial_out_per_conf=0,num_of_sip_video_dial_in_per_conf=0,
                        confFile='Scripts/AddVoipConf.xml',
                        h323VoipDialOutPartyFile='Scripts/AddVoipParty1.xml',
                        h323VoipDialInPartyFile='Scripts/SimAdd323Party.xml',
                        h323VideoDialOutPartyFile='Scripts/AddVideoParty1.xml',
                        h323VideoDialInPartyFile='Scripts/SimAdd323Party.xml',
                        ## pstnDialOutPartyFile='Scripts/AddVoipParty1.xml',
                        ## pstnDialInPartyFile='Scripts/AddVoipParty1.xml',
                        ## isdnDialOutPartyFile='Scripts/AddVoipParty1.xml',
                        ## isdnDialInPartyFile='Scripts/AddVoipParty1.xml',
                        ## sipVoipDialOutPartyFile='Scripts/AddVoipParty1.xml',
                        ## sipVoipDialInPartyFile='Scripts/SimAdd323Party.xml',
                        ## sipVideoDialOutPartyFile='Scripts/AddVoipParty1.xml',
                        ## sipVideoDialInPartyFile='Scripts/SimAdd323Party.xml',
                        deleteAllConf="FALSE"):

    ## constants
    num_retries = 10

    start_time = time()
    last_party_succeeded = True

    
    
    number_of_parties_connected = 0


    num_of_parties_in_conf = 0

    confId = 0
    confName = "empty"
    
    for conf_index in range(0,num_of_confrences):

        ## create new conf
        confName = "Conf_"+str(conf_index+1)
        add_conf_ok = TryAddConf(self,confName,confFile,num_retries)
        if add_conf_ok == False:
            print "Failed to add conf " + confName
            return number_of_parties_connected
        confId = GetConfId(self,confName,num_retries)

        ## connect h323 voip dial out parties
        if num_of_h323_voip_dial_out_per_conf>0:
            print "connecting h323 voip dial out parties"
        for h323_voip_party_out_index in range(0,num_of_h323_voip_dial_out_per_conf):
            
            partyName = "Party_" + str(number_of_parties_connected+1)
            partyIp = GetIpAdressString(self,number_of_parties_connected+1) 

            last_party_succeeded = TryAddParty(self,h323VoipDialOutPartyFile,partyName,partyIp,confId)
            if last_party_succeeded == False:
                print "Failed to add party " + partyName
                return number_of_parties_connected
            number_of_parties_connected = number_of_parties_connected + 1


        ## connect h323 voip dial in parties
        if num_of_h323_voip_dial_in_per_conf>0:
            print "connecting h323 voip dial in parties"    
        for h323_voip_party_in_index in range(0,num_of_h323_voip_dial_in_per_conf):
            
            partyName = "Party_" + str(number_of_parties_connected+1)
            last_party_succeeded = TryAddDialInParty(self,partyName,confName,h323VoipDialInPartyFile,"VOIP_CAPS")
            if last_party_succeeded == False:
                print "Failed to add party " + partyName
                return number_of_parties_connected
                break
            number_of_parties_connected = number_of_parties_connected + 1
            
        ## connect h323 video dial out parties
        if num_of_h323_video_dial_out_per_conf>0:
            print "connecting h323 video dial out parties"
        for h323_video_party_out_index in range(0,num_of_h323_video_dial_out_per_conf):
            
            partyName = "Party_" + str(number_of_parties_connected+1)
            partyIp = GetIpAdressString(self,number_of_parties_connected+1) 

            last_party_succeeded = TryAddParty(self,h323VideoDialOutPartyFile,partyName,partyIp,confId)
            if last_party_succeeded == False:
                print "Failed to add party " + partyName
                return number_of_parties_connected
            number_of_parties_connected = number_of_parties_connected + 1

        ## connect h323 video dial in parties
        if num_of_h323_video_dial_in_per_conf>0:
            print "connecting h323 video dial in parties"    
        for h323_video_party_in_index in range(0,num_of_h323_video_dial_in_per_conf):
            
            partyName = "Party_" + str(number_of_parties_connected+1)
            last_party_succeeded = TryAddDialInParty(self,partyName,confName,h323VoipDialInPartyFile)
            if last_party_succeeded == False:
                print "Failed to add party " + partyName
                return number_of_parties_connected
                break
            number_of_parties_connected = number_of_parties_connected + 1




    print "Result: succeeded to connect " + str(number_of_parties_connected) + " participants"
    
    end_time = time()
    total_time = end_time-start_time
    print "All Parties connected in " + ('%.03f' %(total_time)) + " secondes"

    if deleteAllConf == "TRUE":
        self.DeleteAllConf(1)

    return number_of_parties_connected



    

#   try to connect maximum participant
def MaxPartiesInConfTest(self,confFile,partyFile,max_party_in_conf=800,num_retries=60,deleteConf="TRUE"):
    ##print "MaxPartiesTest"

    start_time = time()

    ## create conf
    num_of_confs = 0
    confName = "Conf_"+str(num_of_confs+1)
    add_conf_ok = TryAddConf(self,confName,confFile,num_retries)
    if add_conf_ok == False:
        print "Failed to add conf " + confName
    confId = GetConfId(self,confName,num_retries)

    ## parties group
    parties_in_group = 800
    num_of_groups = max_party_in_conf/parties_in_group
    add_party_group = 10
    sleep_after_party = 10


    number_of_parties_added = 0
    for group in range(num_of_groups):
        print "connect group " + str(group+1)
        for party in range(parties_in_group):

            partyName = "Party_" + str(number_of_parties_added+1)
            partyIp = GetIpAdressString(self,number_of_parties_added+1)

            self.LoadXmlFile(partyFile)
            print "Adding Party " + partyName + " (ip " + partyIp + ") to confID " + str(confId)
            self.ModifyXml("PARTY","NAME",partyName)
            self.ModifyXml("PARTY","IP",partyIp)
            self.ModifyXml("ADD_PARTY","ID",confId)
            self.Send()
            number_of_parties_added = number_of_parties_added+1
            if number_of_parties_added == sleep_after_party:
                sleep(1)
                sleep_after_party = sleep_after_party + add_party_group


        print "waiting 40 seconds for all parties to be connected"
        sleep(40)
        print "continuing ... "
    
        ## get conf xml
        self.LoadXmlFile('Scripts/TransConf2.xml')
        self.ModifyXml("GET","ID",confId)
        self.Send()

        ongoing_parties = self.xmlResponse.getElementsByTagName("ONGOING_PARTY_STATUS")
        ongoing_party_list = self.xmlResponse.getElementsByTagName("ONGOING_PARTY")

        number_of_parties_connected = 0
        number_of_parties_connected_successfully = 0
        party_index = 0
        for party in ongoing_parties:
                ## find party name in party list
                current_party_name = ongoing_party_list[party_index].getElementsByTagName("NAME")[0].firstChild.data
                ## print "current_party_name = " + current_party_name + " , partyName = " + partyName
                status = party.getElementsByTagName("DESCRIPTION")[0].firstChild.data
                print current_party_name + ": " + status
                if status == "connected":
                    number_of_parties_connected_successfully = number_of_parties_connected_successfully+1                        
                party_index =  party_index + 1
                number_of_parties_connected = number_of_parties_connected+1

        if number_of_parties_connected == number_of_parties_added:
            print "all added parties in list (" + str(number_of_parties_connected) + ")"
        else:
            print str(number_of_parties_connected) + "/" + str(number_of_parties_added) + " found in list"

        if number_of_parties_connected_successfully == number_of_parties_added:
            print "all added parties connected successfully (" + str(number_of_parties_connected) + ")"
        else:
            print str(number_of_parties_connected_successfully) + "/" + str(number_of_parties_added) + " connected successfully"
        
        
 






##         for party in range(parties_in_group):

##             partyName = "Party_" + str(number_of_parties_connected+1)
##             partyIp = GetIpAdressString(self,number_of_parties_connected+1)

##             party_index = 0
##             party_found = False
##             for party in ongoing_parties:
##                 find party name in party list
##                 current_party_name = ongoing_party_list[party_index].getElementsByTagName("NAME")[0].firstChild.data
##                 print "current_party_name = " + current_party_name + " , partyName = " + partyName
##                 if partyName == current_party_name:
##                     party_found = True
##                     wait for party to be connected
##                     status = party.getElementsByTagName("DESCRIPTION")[0].firstChild.data
##                     if status == "connected":
##                         number_of_parties_connected_successfully = number_of_parties_connected_successfully+1
##                         print partyName + ": " + status
##                         break
##                     else:
##                         print partyName + ": " + status
##                 party_index =  party_index + 1

##             if party_found == False:
##                 print partyName + " not found on list"
##             number_of_parties_connected = number_of_parties_connected+1
            

    print "Result: succeeded to connect " + str(number_of_parties_connected_successfully) + "/" + str(number_of_parties_connected) + " participants"
    
    end_time = time()
    total_time = end_time-start_time
    print "All Parties connected in " + ('%.03f' %(total_time)) + " secondes"

    if deleteConf == "TRUE":
        self.DeleteAllConf(1)

    return number_of_parties_connected
#------------------------------------------------------------------------------
#   try to connect maximum participant
def MaxDialInPartiesTest(self,confFile,partyFile,delayBetweenParticipants=0,max_num_of_parties_in_conf=100,max_party_to_try=2000,num_retries=60,deleteConf="TRUE"):
    ##print "MaxPartiesTest"

    start_time = time()
    last_party_succeeded = True
    number_of_parties_connected = 0

    num_of_confs = 0
    num_of_parties_in_conf = 0

    confId = 0
    confName = "empty"
    while last_party_succeeded:

        ## if num of parties is 0 we are creating new conf
        if num_of_parties_in_conf == 0:
            confName = "Conf_"+str(num_of_confs+1)
            add_conf_ok = TryAddConf(self,confName,confFile,num_retries)
            if add_conf_ok == False:
                print "Failed to add conf " + confName
                break
            confId = GetConfId(self,confName,num_retries)
            num_of_confs = num_of_confs + 1

        partyName = "Party_" + str(number_of_parties_connected+1)
        partyIp = GetIpAdressString(self,number_of_parties_connected+1) 
        ##partyIp =  "1.2." + str(num_of_confs) + "." + str(num_of_parties_in_conf+1)
        print "Try to add party " + partyName
        ##TryAddDialInParty(self,partyName,confName,partyFile="Scripts/SimAdd323Party.xml",capSetName = "FULL CAPSET",num_retries=60)
        last_party_succeeded = TryAddDialInParty(self,partyName,confName,partyFile)
        if last_party_succeeded == False:
            print "Failed to add party " + partyName
            break
        number_of_parties_connected = number_of_parties_connected + 1
        num_of_parties_in_conf = num_of_parties_in_conf + 1
        ## set num of parties to 0 will create a new conf
        if num_of_parties_in_conf == max_num_of_parties_in_conf:
            num_of_parties_in_conf = 0
        if number_of_parties_connected ==  max_party_to_try:
            print "Max party to try succeeded"
            break
        sleep(delayBetweenParticipants)

    print "Result: succeeded to connect " + str(number_of_parties_connected) + " participants"
    
    end_time = time()
    total_time = end_time-start_time
    print "All Parties connected in " + ('%.03f' %(total_time)) + " secondes"

    if deleteConf == "TRUE":
        self.DeleteAllConf(1)

    return number_of_parties_connected
#------------------------------------------------------------------------------        
def GetIpAdressString(self,party_number):
    p1 = 0
    p2 = 0
    p3 = 0
    p4 = party_number
    if p4 > 255:
        p3 = int(p4/256)
        p4 = p4 - p3*256
        if p3 > 255:
            p2 = int(p3/256)
            p3 = p3 - p2*256
            if p2 > 255:
                p1 = int(p2/256)
                p2 = p2 - p1*256
                if p1 > 255:
                    print "too many parties  - no more ip addresses"

    ip_string = str(p1) + "." + str(p2) + "." + str(p3) + "." +str(p4)
    return ip_string
                    
#------------------------------------------------------------------------------    
def TryAddConf(self,confName,confFile,retires = 60):
    ##print "TryAddConf"
    start_time = time()

    success = False
    print "Adding Conf " + confName + " ..."
    self.LoadXmlFile(confFile)
    self.ModifyXml("RESERVATION","NAME",confName)
    self.ModifyXml("RESERVATION","DISPLAY_NAME",confName)
    self.Send()

    print "Wait untill Conf \'" + confName + "\' is created...",
    for retry in range(retires+1):
        self.LoadXmlFile('Scripts/TransConfList.xml')
        self.Send()
        ##status = self.SendXmlFileNoResponse('Scripts/TransConfList.xml')
        ongoing_conf_list = self.xmlResponse.getElementsByTagName("CONF_SUMMARY")
        for index in range(len(ongoing_conf_list)):  
            if(confName == ongoing_conf_list[index].getElementsByTagName("NAME")[0].firstChild.data):
                confid = ongoing_conf_list[index].getElementsByTagName("ID")[0].firstChild.data
                if confid != "":
                    success = True
                    break
        if success == True:
            break

    end_time = time()
    total_time = end_time-start_time
    print "Created Conf, ID:" + str(confid) + " in " + ('%.03f' %(total_time)) + " secondes"
    return success
    
#------------------------------------------------------------------------------
def GetConfId(self,confName,retires = 60):
    ##print "GetConfId"

    """Monitor conferences list until 'confName' is found.
    Returns conference ID.

    confName - lookup conf name 
    """
    bFound = False
    for retry in range(retires+1):
        status = self.SendXmlFile('Scripts/TransConfList.xml',"Status OK")
        ongoing_conf_list = self.xmlResponse.getElementsByTagName("CONF_SUMMARY")
        for index in range(len(ongoing_conf_list)):  
            if(confName == ongoing_conf_list[index].getElementsByTagName("NAME")[0].firstChild.data):
                confid = ongoing_conf_list[index].getElementsByTagName("ID")[0].firstChild.data
                if confid != "":
                    bFound = True
                    break
        if(bFound):
            break
        if (retry == retires):
            print self.xmlResponse.toprettyxml(encoding="utf-8")
            self.Disconnect()                
            ScriptAbort("GetConfId: Can not monitor conf:" + status)
        sys.stdout.write(".")
        sys.stdout.flush()
        sleep(1)
    return confid


#------------------------------------------------------------------------------
def TryAddParty(self,partyFile,partyName,partyIp,confId,num_retries=60):
    ##print "TryAddParty"


    success = False

    ## send add party
    start_time = time()
    self.LoadXmlFile(partyFile)
    print "Adding Party " + partyName + " (ip " + partyIp + ") to confID " + str(confId)
    self.ModifyXml("PARTY","NAME",partyName)
    self.ModifyXml("PARTY","IP",partyIp)
    self.ModifyXml("ADD_PARTY","ID",confId)
    self.Send()

    ## sleep(1) #for party to be connected
    for retry in range(num_retries):

        ## get conf
        self.LoadXmlFile('Scripts/TransConf2.xml')
        self.ModifyXml("GET","ID",confId)
        self.Send()


        ongoing_parties = self.xmlResponse.getElementsByTagName("ONGOING_PARTY_STATUS")
        ongoing_party_list = self.xmlResponse.getElementsByTagName("ONGOING_PARTY")
        
        list_time = time()
        total_list_time = list_time-start_time
        ## print "list received after " + ('%.03f' %(total_list_time)) + " secondes"

        
        party_index = 0
        for party in ongoing_parties:
            ## find party name in party list
            current_party_name = ongoing_party_list[party_index].getElementsByTagName("NAME")[0].firstChild.data
            ## print "current_party_name = " + current_party_name + " , partyName = " + partyName
            if partyName == current_party_name:
                ## wait for party to be connected
                status = party.getElementsByTagName("DESCRIPTION")[0].firstChild.data
                if status == "connected":
                    success = True
                    ## print status
                    break
                else:
                    ## print status
                    sleep(1/100)
            party_index =  party_index + 1
        if success == True:
            break

        retry_time = time()
        total_try_time = retry_time-start_time
        ## print "retry after " + ('%.03f' %(total_try_time)) + " secondes"

        
    end_time = time()
    total_time = end_time-start_time
    print "Party connected in " + ('%.03f' %(total_time)) + " secondes"

    return success
#------------------------------------------------------------------------------
def TryAddDialInParty(self,partyName,confName,partyFile="Scripts/SimAdd323Party.xml",capSetName = "FULL CAPSET",num_retries=60):

    success = False

## get conf
    confId = GetConfId(self,confName,num_retries)
    self.LoadXmlFile('Scripts/TransConf2.xml')
    self.ModifyXml("GET","ID",confId)
    self.Send()

    ongoing_party_list = self.xmlResponse.getElementsByTagName("ONGOING_PARTY")
    test_len = len(ongoing_party_list)
    ##print "-before-------------------"
    ##print "parties list len = " + str(test_len)
    ##print "--------------------------"

    
    ## send add party
    start_time = time()

    if capSetName=="VOIP_CAPS":
        capSetName = AddVoipCapSet(self, CapSetName="VOIP_CAPS")
    self.LoadXmlFile(partyFile)
    print "Adding Dial in Party " + partyName + " to conf " + confName
    self.ModifyXml("H323_PARTY_ADD","PARTY_NAME",partyName)
    self.ModifyXml("H323_PARTY_ADD","CONF_NAME",confName)
    self.ModifyXml("H323_PARTY_ADD","CAPSET_NAME",capSetName)
    self.Send()
    ## connect dial in party from simulation
    self.SimulationConnectH323Party(partyName);

    # check if new party connected
    for retry in range(num_retries):
        ## get conf
        confId = GetConfId(self,confName,num_retries)
        self.LoadXmlFile('Scripts/TransConf2.xml')
        self.ModifyXml("GET","ID",confId)
        self.Send()

        ongoing_party_list = self.xmlResponse.getElementsByTagName("ONGOING_PARTY")
        after_len = len(ongoing_party_list)
        ##print "--retry" + str(retry)
        ##print "parties list len = " + str(after_len)

        ongoing_parties = self.xmlResponse.getElementsByTagName("ONGOING_PARTY_STATUS")
        party_index = 0
        num_of_connected_parties = 0
        for party in ongoing_parties:
            status = party.getElementsByTagName("DESCRIPTION")[0].firstChild.data
            if status == "connected":
                num_of_connected_parties = num_of_connected_parties +1
            else:
                sleep(1/100)
                break

        ##print "num_of_connected_parties" + str(num_of_connected_parties)
        ##print "--------------------------"
        
        if num_of_connected_parties == len(ongoing_party_list):
            success = True
            
        if success == True:
            break
        
    end_time = time()
    total_time = end_time-start_time
    print "Party connected in " + ('%.03f' %(total_time)) + " secondes"
    ##print

    return success    
#------------------------------------------------------------------------------


def AddVoipCapSet(self, CapSetName="VOIP_CAPS"):
    self.LoadXmlFile('Scripts/SimAddCapSet.xml')
    self.ModifyXml("ADD_CAP_SET","NAME",CapSetName)
    self.ModifyXml("ADD_CAP_SET","FECC","false")
    self.ModifyXml("ADD_CAP_SET","H239","false")
    self.ModifyXml("ADD_CAP_SET","VIDEO_H264","false")
    self.ModifyXml("ADD_CAP_SET","VIDEO_H263","false")
    self.ModifyXml("ADD_CAP_SET","VIDEO_VP8","false") ##N.A. DEBUG VP8
    self.Send()
    return CapSetName

#------------------------------------------------------------------------------
def TryAddProfile(self, profileName,num_retries=60, fileName="Scripts/CreateNewProfile.xml"):

    success = False


    print "Adding new Profile..."
    self.LoadXmlFile(fileName)
    self.ModifyXml("RESERVATION","NAME", profileName)
    self.Send("")

    ## sleep(1) #for profile to be monitored
    for retry in range(num_retries):

        self.SendXmlFile('Scripts/GetProfileList.xml')
        profile_list = self.xmlResponse.getElementsByTagName("PROFILE_SUMMARY")
        for index in range(len(profile_list)):
            currentProfName = profile_list[index].getElementsByTagName("NAME")[0].firstChild.data
            currentProfId = profile_list[index].getElementsByTagName("ID")[0].firstChild.data
            if profileName == currentProfName:
                print "Profile " + profileName + " added to profile list with id " + str(currentProfId)
                success = True
                break
        if success == True:
            break

    return success

#------------------------------------------------------------------------------
def MaxProfilesTest(self,max_profiles_to_try=100,num_retries=60, delete_profiles = "TRUE"):

    last_profile_succeeded = True
    num_of_profiles = 0

    self.SendXmlFile('Scripts/GetProfileList.xml')
    profile_list = self.xmlResponse.getElementsByTagName("PROFILE_SUMMARY")
    num_of_profiles_already_exist = len(profile_list)
    print "num_of_profiles_already_exist = " + str(num_of_profiles_already_exist)
    num_of_profiles = num_of_profiles + num_of_profiles_already_exist


    while last_profile_succeeded:
        profileName = "Profile_"+str(num_of_profiles+1)
        last_profile_succeeded = TryAddProfile(self,profileName,num_retries)
        if last_profile_succeeded==False:
            print "Failed to add profile" + profileName
            break
        num_of_profiles = num_of_profiles +1
        if num_of_profiles ==  max_profiles_to_try:
            print "Max profiles to try succeeded (" + str(num_of_profiles) + " profiles)"
            break

    if delete_profiles == "TRUE":
        self.SendXmlFile('Scripts/GetProfileList.xml')
        profile_list = self.xmlResponse.getElementsByTagName("PROFILE_SUMMARY")
        for index in range(len(profile_list)):
            profName = profile_list[index].getElementsByTagName("NAME")[0].firstChild.data
            profId = profile_list[index].getElementsByTagName("ID")[0].firstChild.data
            if profName != "Factory_Video_Profile":
                print "Deleting Profile: " + profName
                self.DelProfile(profId)

    print "Result: succeeded to add " + str(num_of_profiles) + " profiles"
    if num_of_profiles ==  max_profiles_to_try:
        print "Max profiles to try succeeded (" + str(num_of_profiles) + " profiles)"
    else:
        ScriptAbort("Failed to add max profiles")
    

    
    return num_of_profiles
            
#------------------------------------------------------------------------------
#   try to connect maximum participant
def MaxConfrencesTest(self,max_cofrences_to_try=800,num_retries=60,confFile='Scripts/AddVoipConf.xml',deleteConf="TRUE"):
    ##print "MaxPartiesTest"

    last_conf_succeeded = True
    num_of_confs = 0

    while last_conf_succeeded:
        confName = "Conf_"+str(num_of_confs+1)
        add_conf_ok = TryAddConf(self,confName,confFile,num_retries)
        if add_conf_ok == False:
            print "Failed to add conf " + confName
            break
        num_of_confs = num_of_confs + 1
        if num_of_confs == max_cofrences_to_try:
            print "Max confrences to try succeeded (" + str(num_of_confs) + " confrences)"
            break

    if deleteConf == "TRUE":
        self.DeleteAllConf(1)

    print "Result: succeeded to add " + str(num_of_confs) + " confrences"
    return num_of_confs
 #------------------------------------------------------------------------------
def TryAddEQ(self, EqName,num_retries=60, fileName="Scripts/SystemCapacityTests/AddEq.xml"):

    success = False
    print "Adding a new Entry Queue: " + EqName
    success = TryAddReservation(self,EqName,60,fileName,False)
    return success

#------------------------------------------------------------------------------
def MaxEntryQueuesTest(self,max_eqs_to_try=100,num_retries=60, delete_eqs = "TRUE"):

    last_eq_succeeded = True
    num_of_eqs = 0

    self.SendXmlFile('Scripts/GetMRList.xml')
    mr_list = self.xmlResponse.getElementsByTagName("MEETING_ROOM_SUMMARY")
    num_of_eqs_already_exist = 0
    for mr_index in range (len(mr_list)):
        if "true" == mr_list[mr_index].getElementsByTagName("ENTRY_QUEUE")[0].firstChild.data:
            eq_name = mr_list[mr_index].getElementsByTagName("NAME")[0].firstChild.data
            print "existing EQ " + eq_name
            num_of_eqs_already_exist = num_of_eqs_already_exist + 1

    print "num_of_entry_queue_exist = " + str(num_of_eqs_already_exist)
    if num_of_eqs_already_exist >= max_eqs_to_try:
        print "num_of_entry_queue_exist >= max_eqs_to_try"
        return num_of_eqs_already_exist

    num_of_eqs = num_of_eqs + num_of_eqs_already_exist

    while last_eq_succeeded:

        EqName = "Eq_"+str(num_of_eqs+1)
        last_eq_succeeded = TryAddEQ(self,EqName,num_retries)
        if last_eq_succeeded==False:
            print "Failed to add EQ" + EqName
            break
        num_of_eqs = num_of_eqs +1
        if num_of_eqs ==  max_eqs_to_try:
            print "Max EQs to try succeeded (" + str(num_of_eqs) + " EQs)"
            break

    if delete_eqs == "TRUE":
        self.SendXmlFile('Scripts/GetMRList.xml')
        mr_list = self.xmlResponse.getElementsByTagName("MEETING_ROOM_SUMMARY")
        for mr_index in range(len(mr_list)):
            if "true" == mr_list[mr_index].getElementsByTagName("ENTRY_QUEUE")[0].firstChild.data:
                eq_name = mr_list[mr_index].getElementsByTagName("NAME")[0].firstChild.data
                if eq_name != "DefaultEQ":
                    eqId = mr_list[mr_index].getElementsByTagName("ID")[0].firstChild.data
                    print "Deleting EQ: " + eq_name
                    self.DelReservation(eqId, "Scripts/SystemCapacityTests/RemoveMr.xml")


    print "Result: succeeded to add " + str(num_of_eqs) + " EQs"
    if num_of_eqs ==  max_eqs_to_try:
        print "Max EQs to try succeeded (" + str(num_of_eqs) + " EQs)"
    else:
        ScriptAbort("Failed to add max EQs")

    return num_of_eqs

#------------------------------------------------------------------------------


def MaxSipFactoriesTest(self,max_eqs_to_try=100,num_retries=60, delete_eqs = "TRUE"):

    last_eq_succeeded = True
    num_of_eqs = 0


    self.SendXmlFile('Scripts/GetMRList.xml')
    mr_list = self.xmlResponse.getElementsByTagName("MEETING_ROOM_SUMMARY")
    num_of_eqs_already_exist = 0
    
    for mr_index in range (len(mr_list)):
        if "true" == mr_list[mr_index].getElementsByTagName("SIP_FACTORY")[0].firstChild.data:
            eq_name = mr_list[mr_index].getElementsByTagName("NAME")[0].firstChild.data
            print "existing Sip Factoty " + eq_name
            num_of_eqs_already_exist = num_of_eqs_already_exist + 1

    print "num_of_sip_factories_exist = " + str(num_of_eqs_already_exist)
    if num_of_eqs_already_exist >= max_eqs_to_try:
        print "num_of_entry_queue_exist >= max_eqs_to_try"
        return num_of_eqs_already_exist

    num_of_eqs = num_of_eqs + num_of_eqs_already_exist

    while last_eq_succeeded:

        EqName = "SipSip__"+str(num_of_eqs+1)
        last_eq_succeeded = TryAddSipFactory(self,EqName,num_retries)
        if last_eq_succeeded==False:
            print "Failed to Sip Factory" + EqName
            break
        num_of_eqs = num_of_eqs +1
        if num_of_eqs ==  max_eqs_to_try:
            print "Max Sip Factories to try succeeded (" + str(num_of_eqs) + " EQs)"
            break

    if delete_eqs == "TRUE":
        self.SendXmlFile('Scripts/GetMRList.xml')
        mr_list = self.xmlResponse.getElementsByTagName("MEETING_ROOM_SUMMARY")
        for mr_index in range(len(mr_list)):
            if "true" == mr_list[mr_index].getElementsByTagName("SIP_FACTORY")[0].firstChild.data:
                eq_name = mr_list[mr_index].getElementsByTagName("NAME")[0].firstChild.data
                if eq_name != "DefaultFactory":
                    eqId = mr_list[mr_index].getElementsByTagName("ID")[0].firstChild.data
                    print "Deleting Sip Factory: " + eq_name
                    self.DelReservation(eqId, "Scripts/SystemCapacityTests/RemoveMr.xml")


    print "Result: succeeded to add " + str(num_of_eqs) + " Sip Factories"
    if num_of_eqs ==  max_eqs_to_try:
        print "Max Sip Factories to try succeeded (" + str(num_of_eqs) + " Sip Factories)"
    else:
        ScriptAbort("Failed to add max Sip Factories")

    return num_of_eqs
#------------------------------------------------------------------------------


def MaxMeetingRoomsTest(self,max_eqs_to_try=100,num_retries=60, delete_eqs = "TRUE"):

    last_eq_succeeded = True
    num_of_eqs = 0


    self.SendXmlFile('Scripts/GetMRList.xml')
    mr_list = self.xmlResponse.getElementsByTagName("MEETING_ROOM_SUMMARY")
    num_of_eqs_already_exist = 0
    
    for mr_index in range (len(mr_list)):
        if "false" == mr_list[mr_index].getElementsByTagName("SIP_FACTORY")[0].firstChild.data and "false" == mr_list[mr_index].getElementsByTagName("ENTRY_QUEUE")[0].firstChild.data:
            eq_name = mr_list[mr_index].getElementsByTagName("NAME")[0].firstChild.data
            print "existing Meeting Room " + eq_name
            num_of_eqs_already_exist = num_of_eqs_already_exist + 1

    print "num_of_meeting_rooms_exist = " + str(num_of_eqs_already_exist)
    if num_of_eqs_already_exist >= max_eqs_to_try:
        print "num_of_entry_queue_exist >= max_eqs_to_try"
        return num_of_eqs_already_exist

    num_of_eqs = num_of_eqs + num_of_eqs_already_exist

    while last_eq_succeeded:

        EqName = "MeetingRoom__"+str(num_of_eqs+1)
        last_eq_succeeded = TryAddMeetingRoom(self,EqName,num_retries)
        if last_eq_succeeded==False:
            print "Failed to add Meeting Room" + EqName
            break
        num_of_eqs = num_of_eqs +1
        if num_of_eqs ==  max_eqs_to_try:
            print "Max Meeting Rooms to try succeeded (" + str(num_of_eqs) + " EQs)"
            break


    if delete_eqs == "TRUE":
        self.SendXmlFile('Scripts/GetMRList.xml')
        mr_list = self.xmlResponse.getElementsByTagName("MEETING_ROOM_SUMMARY")
        for mr_index in range(len(mr_list)):
            if "false" == mr_list[mr_index].getElementsByTagName("SIP_FACTORY")[0].firstChild.data and "false" == mr_list[mr_index].getElementsByTagName("ENTRY_QUEUE")[0].firstChild.data:
                eq_name = mr_list[mr_index].getElementsByTagName("NAME")[0].firstChild.data
                if eq_name != "Fig_Room" and eq_name != "Maple_Room" and eq_name != "Oak_Room" and eq_name != "Junipar_Room":
                    eqId = mr_list[mr_index].getElementsByTagName("ID")[0].firstChild.data
                    print "Deleting MR: " + eq_name
                    self.DelReservation(eqId, "Scripts/SystemCapacityTests/RemoveMr.xml")


    print "Result: succeeded to add " + str(num_of_eqs) + " MRs"
    if num_of_eqs ==  max_eqs_to_try:
        print "Max MRs to try succeeded (" + str(num_of_eqs) + " MRs)"
    else:
        ScriptAbort("Failed to add max MRs")

    return num_of_eqs

#------------------------------------------------------------------------------
def TryAddSipFactory(self, FactoryName,num_retries=60, fileName="Scripts/SystemCapacityTests/AddSipFactory.xml"):

    success = False
    print "Adding a new Sip Factory: " + FactoryName
    success = TryAddReservation(self,FactoryName,60,fileName,False)
    return success
#------------------------------------------------------------------------------
def TryAddMeetingRoom(self, MrName,num_retries=60, fileName="Scripts/SystemCapacityTests/AddMr.xml"):

    success = False
    print "Adding a new Meeting Room: " + MrName
    start_time = time()
    success = TryAddReservation(self,MrName,60,fileName,False)
    if success==True:
        end_time = time()
        total_time = end_time-start_time
        print "Meeting room added in " + ('%.03f' %(total_time)) + " secondes"
    
    return success

#------------------------------------------------------------------------------
def TryAddReservation(self, RsrvName,num_retries=60, fileName="Scripts/AddSipFactory.xml",exit_on_failure=False):

    success = False

    self.LoadXmlFile(fileName)
    self.ModifyXml("RESERVATION","NAME",RsrvName)
    self.ModifyXml("RESERVATION","DISPLAY_NAME",RsrvName)
    if exit_on_failure==False:
        self.Send("")
    else:
        self.Send()


    ## sleep(1) ## for EQ to be monitored
    for retry in range(num_retries+1):
        status = self.SendXmlFile('Scripts/GetMRList.xml',"Status OK")
        mr_list = self.xmlResponse.getElementsByTagName("MEETING_ROOM_SUMMARY")
        for index in range(len(mr_list)):  
            if RsrvName == mr_list[index].getElementsByTagName("NAME")[0].firstChild.data:
                mrId = mr_list[index].getElementsByTagName("ID")[0].firstChild.data
                mrNumericId = mr_list[index].getElementsByTagName("NUMERIC_ID")[0].firstChild.data
                if mrId != "":
                    success = True
                    break
        if success:
            break
        
    if success:
        print "Created Reservation: " + RsrvName + ", ID = " + str(mrId)
    else:
        print "Failed To Created Reservation: " + RsrvName
        
    return success
#------------------------------------------------------------------------------
def TryAddUser(self, UserName="USER_01",UserPassword="1000",num_retries=60, fileName="Scripts/AddNewOperator.xml",exit_on_failure=False):
    
    success = False

    self.LoadXmlFile(fileName)
    self.ModifyXml("OPERATOR","USER_NAME",UserName)
    self.ModifyXml("OPERATOR","PASSWORD",UserPassword)
    if exit_on_failure==False:
        self.Send("")
    else:
        self.Send()

    for retry in range(num_retries+1):
        status = self.SendXmlFile('Scripts/OperatorsListMonitor.xml',"Status OK")
        oper_list = self.xmlResponse.getElementsByTagName("OPERATOR")
        for index in range(len(oper_list)):
            current_name = oper_list[index].getElementsByTagName("USER_NAME")[0].firstChild.data
            if UserName == current_name:
                success = True
                break
        if success:
            break
        
    print "user: " + UserName + " added"
    return success
#------------------------------------------------------------------------------
#   try to connect maximum participant
def MaxUsersTest(self,max_users_to_try=100,num_retries=60,deleteConf="TRUE"):

    last_succeeded = True
    num_of_users = 0

    self.SendXmlFile('Scripts/OperatorsListMonitor.xml')
    users_list = self.xmlResponse.getElementsByTagName("OPERATOR")
    num_of_users_already_exist = len(users_list)

    print "num_of_users_already_exist = " + str(num_of_users_already_exist)
    if num_of_users_already_exist >= max_users_to_try:
        print "num_of_entry_queue_exist >= max_users_to_try"
        return num_of_users_already_exist

    num_of_users = num_of_users + num_of_users_already_exist



    while last_succeeded:
        userName = "USER_"+str(num_of_users+1)
        password = str(1000 + num_of_users)
        add_conf_ok = TryAddUser(self,userName,password,num_retries)
        if add_conf_ok == False:
            print "Failed to add user " + userName
            break
        num_of_users = num_of_users + 1
        if num_of_users == max_users_to_try:
            print "Max try to try succeeded (" + str(num_of_users) + " users)"
            break

##     if deleteConf == "TRUE":
##         self.DeleteAllConf(1)

    print "Result: succeeded to add " + str(num_of_users) + " users"
    return num_of_users
 #------------------------------------------------------------------------------
#------------------------------------------------------------------------------
#   try to connect maximum participant
def MaxConnectionsTest(self,max_connections_to_try=100,num_retries=60,cleanUp="TRUE"):

    last_succeeded = True
    num_of_users = 0

    self.SendXmlFile('Scripts/OperatorsListMonitor.xml')
    users_list = self.xmlResponse.getElementsByTagName("OPERATOR")
    num_of_users_already_exist = len(users_list)

    print "num_of_users_already_exist = " + str(num_of_users_already_exist)
    if num_of_users_already_exist >= max_users_to_try:
        print "num_of_entry_queue_exist >= max_users_to_try"
        return num_of_users_already_exist

    num_of_users = num_of_users + num_of_users_already_exist



    while last_succeeded:
        userName = "USER_"+str(num_of_users+1)
        password = str(1000 + num_of_users)
        add_conf_ok = TryAddUser(self,userName,password,num_retries)
        if add_conf_ok == False:
            print "Failed to add user " + userName
            break
        num_of_users = num_of_users + 1
        if num_of_users == max_users_to_try:
            print "Max try to try succeeded (" + str(num_of_users) + " users)"
            break

##     if deleteConf == "TRUE":
##         self.DeleteAllConf(1)

    print "Result: succeeded to add " + str(num_of_users) + " users"
    return num_of_users

 #------------------------------------------------------------------------------
def AddConnection(self,StationName="Ron"):
##    try:
##        ip=os.environ["TARGET_IP"]
##    except KeyError:
##        ip="127.0.0.1"
            
##    try:
##        port=os.environ["TARGET_PORT"]
##    except KeyError:
##        port="8080"
    
    ip="127.22.192.60"
    #port="8080"
    port="80"
    
    self.ip = ip
    self.port = string.atoi(port)
##    self.user = "SUPPORT"
##    self.password = "SUPPORT"
           
    # logindom = parse('Scripts/login.xml')
    self.LoadXmlFile('Scripts/login1.xml')
        
    #if self.port == 8080:
    if self.port == 80:
        self.connection = McmsHttpConnection(ip,self.port)
    else:
        self.connection = McmsHttpsConnection(ip,self.port)
        #logindom.getElementsByTagName("IP")[0].firstChild.data = ip
       # self.getElementsByTagName("IP")[0].firstChild.data = ip
##    self.ModifyXml("LOGIN","IP",ip)
##    self.ModifyXml("LOGIN","USER_NAME","POLYCOM")
##    self.ModifyXml("LOGIN","PASSWORD","POLYCOM")
##    self.ModifyXml("LOGIN","STATION_NAME" ,StationName)
        
    #self.SendXml(logindom)
    #self.Send()
    self.Send("Status OK")
        
    #self.connection.SetTimeOut(60) # set timeout for each transaction
##    self.mcutoken = self.GetTextUnder("LOGIN","MCU_TOKEN")        
##    self.usertoken = self.GetTextUnder("LOGIN","MCU_USER_TOKEN")
        
        


 #------------------------------------------------------------------------------
#------------------------------------------------------------------------------
# Amos capacity
#------------------------------------------------------------------------------
