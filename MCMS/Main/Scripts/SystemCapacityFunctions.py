#!/mcms/python/bin/python
#############################################################################
# FUNCTIONS of System Capacity Scripts 
# Programer: Ron, Keren 
# Date: 06/2008
#############################################################################
from McmsConnection import *
from time import *

#------------------------------------------------------------------------------
# Barak capacity
#------------------------------------------------------------------------------
#   try to connect maximum participant
def MaxPartiesInConfTest(self,confFile,partyFile,parties_in_group,max_party_in_conf=800,num_retries=60,deleteConf="TRUE",conf_serial_number = 1):
    
    start_time = time()

    ## create conf
    confName = "Conf_"+str(conf_serial_number)
    add_conf_ok = TryAddConf(self,confName,confFile,num_retries)
    if add_conf_ok == False:
        print "Failed to add conf " + confName
    confId = GetConfId(self,confName,num_retries)

    ## parties group
    #parties_in_group = 100
    num_of_groups = max_party_in_conf/parties_in_group
    add_party_group = 20
    sleep_after_party = 20

    number_of_parties_added = 0
    for group in range(num_of_groups):
        print "connect group " + str(group+1)
        for party in range(parties_in_group):

            partyName = "Party_" + str(number_of_parties_added+1)
            aliasName = "Alias " + str(number_of_parties_added+1)
            partyIp = GetIpAdressString(self,number_of_parties_added+1)

            self.LoadXmlFile(partyFile)
            print "Adding Party " + partyName + " (ip " + partyIp + ") to confID " + str(confId)
            #print "..........."
            #print "Alias Name " + aliasName
            self.ModifyXml("PARTY","NAME",partyName)
            self.ModifyXml("PARTY","IP",partyIp)
            self.ModifyXml("ADD_PARTY","ID",confId)
            self.ModifyXml("ALIAS","NAME",aliasName)
            self.Send()
            number_of_parties_added = number_of_parties_added+1
            if number_of_parties_added == sleep_after_party:
                sleep(1)
                sleep_after_party = sleep_after_party + add_party_group


        print "waiting 2 seconds for all parties to be connected"
        sleep(2)
        print "continuing ... "
        
        
    self.WaitAllPartiesWereAdded(confId,number_of_parties_added,30)
    self.WaitAllOngoingConnected(confId,num_retries,2)
        
    end_time = time()
    total_time = end_time-start_time
    print "All Parties connected in " + ('%.03f' %(total_time)) + " secondes"

    if deleteConf == "TRUE":
        self.DeleteAllConf(1)
#------------------------------------------------------------------------------
def MaxPartiesTest1(self,confFile,partyFile,delayBetweenParticipants=0,max_num_of_parties_in_conf=80,max_party_to_try=400,num_retries=60,deleteConf="TRUE"):
    
    start_time = time()
    num_confs = max_party_to_try / max_num_of_parties_in_conf
    parties_in_group = 20#max_num_of_parties_in_conf / 2
    for conf_index in range(num_confs):
        MaxPartiesInConfTest(self,confFile,partyFile,parties_in_group,max_num_of_parties_in_conf,num_retries,"FALSE",conf_index+1)
     
    end_time = time()
    total_time = end_time-start_time
    print "All Parties connected in " + ('%.03f' %(total_time)) + " secondes"   
    
    self.DeleteAllConf(3)
    
#------------------------------------------------------------------------------
def MaxPartiesTest3(self,confFile,partyFile,delayBetweenParticipants=0,max_num_of_parties_in_conf=80,max_party_to_try=400,num_retries=60,deleteConf="TRUE"):
    
    start_time = time()
    num_confs = max_party_to_try / max_num_of_parties_in_conf
    parties_in_group = max_num_of_parties_in_conf / 2
    for conf_index in range(num_confs):
        MaxPartiesInConfTest(self,confFile,partyFile,parties_in_group,max_num_of_parties_in_conf,num_retries,"FALSE",conf_index+1)
     
    end_time = time()
    total_time = end_time-start_time
    print "All Parties connected in " + ('%.03f' %(total_time)) + " secondes"   
    
    self.DeleteAllConf(3)
#------------------------------------------------------------------------------    
def MaxPartiesTest(self,confFile,partyFile,delayBetweenParticipants=0,max_num_of_parties_in_conf=80,max_party_to_try=400,num_retries=60,deleteConf="TRUE"):
    ##print "MaxPartiesTest"

    start_time = time()
    last_party_succeeded = True
    number_of_parties_connected = 0

    num_of_confs = 0
    num_of_parties_in_conf = 0

    confId = 0
    while last_party_succeeded and (number_of_parties_connected < max_party_to_try):

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
        last_party_succeeded = TryAddParty(self,partyFile,partyName,partyIp,confId,num_retries)
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
          
        sleep(delayBetweenParticipants)

    print "Result: succeeded to connect " + str(number_of_parties_connected) + " participants"
    
    end_time = time()
    total_time = end_time-start_time
    print "All Parties connected in " + ('%.03f' %(total_time)) + " secondes"

    if deleteConf == "TRUE":
        self.DeleteAllConf(1)
        
        
    return number_of_parties_connected
#------------------------------------------------------------------------------
def MaxMixPartiesTest(self,confFile,partyFile,delayBetweenParticipants=0,max_num_of_parties_in_conf=80,max_num_of_video_parties_in_conf=80, max_party_to_try=400,max_video_parties_to_try =80,num_retries=60,deleteConf="TRUE"):
##max_video_parties is the number of video parties from the max num of parties

    start_time = time()
    last_party_succeeded = True
    number_of_parties_connected = 0
    number_of_video_parties_connected = 0

    num_of_confs = 0
    num_of_parties_in_conf = 0
    isVideoParty = "NO"

    confId = 0
    if(max_video_parties_to_try>max_party_to_try):
        max_video_parties_to_try = max_party_to_try
    
    while last_party_succeeded and (number_of_parties_connected < max_party_to_try):

        ## if num of parties is 0 we are creating new conf
        if num_of_parties_in_conf == 0:
            confName = "Conf_"+str(num_of_confs+1)
            num_of_video_parties_in_conf = 0
            add_conf_ok = TryAddConf(self,confName,confFile,num_retries)
            if add_conf_ok == False:
                print "Failed to add conf " + confName
                break
            confId = GetConfId(self,confName,num_retries)
            num_of_confs = num_of_confs + 1

        partyName = "Party_" + str(number_of_parties_connected+1)
        partyIp = GetIpAdressString(self,number_of_parties_connected+1) 
        ##partyIp =  "1.2." + str(num_of_confs) + "." + str(num_of_parties_in_conf+1) 
        if((number_of_video_parties_connected<max_video_parties_to_try) and (num_of_video_parties_in_conf < max_num_of_video_parties_in_conf) ):
            isVideoParty = "YES"
        else:
            isVideoParty = "NO"
        last_party_succeeded = TryAddParty(self,partyFile,partyName,partyIp,confId,num_retries,isVideoParty)
        if last_party_succeeded == False:
            print "Failed to add party " + partyName
            break
        number_of_parties_connected = number_of_parties_connected + 1
        num_of_parties_in_conf = num_of_parties_in_conf + 1
        if(isVideoParty == "YES"):
            number_of_video_parties_connected = number_of_video_parties_connected + 1
            num_of_video_parties_in_conf = num_of_video_parties_in_conf + 1
        ## set num of parties to 0 will create a new conf
        if num_of_parties_in_conf == max_num_of_parties_in_conf:
            num_of_parties_in_conf = 0
        if number_of_parties_connected ==  max_party_to_try:
            print "Max party to try succeeded"
            break
        sleep(delayBetweenParticipants)
        print "num_of_parties_in_conf = " +str(num_of_parties_in_conf)
        print "max_num_of_parties_in_conf = " + str(max_num_of_parties_in_conf)

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
def TryAddParty(self,partyFile,partyName,partyIp,confId,num_retries=60, isVideoParty = "AUTO"):
    ##print "TryAddParty"
    ##isVideoParty the options are: "AUTO"(according to the party file), "YES"(video party), "NO"(audio only party)

    success = False

    ## send add party
    start_time = time()
    self.LoadXmlFile(partyFile)
    print "Adding Party " + partyName + " (ip " + partyIp + ") to confID " + str(confId)
    aliasName = "Alias_" + partyName
    self.ModifyXml("PARTY","NAME",partyName)
    self.ModifyXml("PARTY","IP",partyIp)
    self.ModifyXml("ADD_PARTY","ID",confId)
    self.ModifyXml("ALIAS","NAME",aliasName)
    if(isVideoParty=="YES"):
        self.ModifyXml("PARTY","CALL_CONTENT","framed")
    if(isVideoParty=="NO"):
        self.ModifyXml("PARTY","CALL_CONTENT","voice")
    self.Send()

    ## sleep(1) #for party to be connected
    for retry in range(num_retries):

        ## get conf
        self.LoadXmlFile('Scripts/TransConf2.xml')
        self.ModifyXml("GET","ID",confId)
        self.Send()


        ongoing_parties = self.xmlResponse.getElementsByTagName("ONGOING_PARTY_STATUS")
        ongoing_party_list = self.xmlResponse.getElementsByTagName("ONGOING_PARTY")
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
                    print status
                    #break
                else:
                    print status
                    #sleep(1/100)
                break    
            party_index =  party_index + 1
        if success == True:
            break
    if (success == False):
        print "fail status = "+str(status)
        self.Disconnect()
        #ScriptAbort("Can not connect party: " + partyName)   
    end_time = time()
    total_time = end_time-start_time
    print "Party connected in " + ('%.03f' %(total_time)) + " secondes"

    return success
    
#------------------------------------------------------------------------------
def TryAddPartyRev(self,partyFile,partyName,partyIp,confId,num_retries=60, isVideoParty = "AUTO"):
    ##print "TryAddParty"
    ##isVideoParty the options are: "AUTO"(according to the party file), "YES"(video party), "NO"(audio only party)

    success = False

    ## send add party
    start_time = time()
    self.LoadXmlFile(partyFile)
    print "Adding Party " + partyName + " (ip " + partyIp + ") to confID " + str(confId)
    self.ModifyXml("PARTY","NAME",partyName)
    self.ModifyXml("PARTY","IP",partyIp)
    self.ModifyXml("ADD_PARTY","ID",confId)
    if(isVideoParty=="YES"):
        self.ModifyXml("PARTY","CALL_CONTENT","framed")
    if(isVideoParty=="NO"):
        self.ModifyXml("PARTY","CALL_CONTENT","voice")
    self.Send()

    ## sleep(1) #for party to be connected
    for retry in range(num_retries):

        ## get conf
        self.LoadXmlFile('Scripts/TransConf2.xml')
        self.ModifyXml("GET","ID",confId)
        self.Send()


        ongoing_parties = self.xmlResponse.getElementsByTagName("ONGOING_PARTY_STATUS")
        ongoing_party_list = self.xmlResponse.getElementsByTagName("ONGOING_PARTY")
        party_index = len(ongoing_party_list) - 1
        for party in reversed(ongoing_parties):
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
                    print status
                    #sleep(1/100)
            party_index =  party_index - 1
        if success == True:
            break
    if (success == False):
        print "fail status = "+str(status)
        self.Disconnect()
        #ScriptAbort("Can not connect party: " + partyName)   
    end_time = time()
    total_time = end_time-start_time
    print "Party connected in " + ('%.03f' %(total_time)) + " secondes"

    return success
    
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
    id_of_profile = 0

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
            self.Disconnect()
            break
        num_of_profiles = num_of_profiles +1
        if num_of_profiles ==  max_profiles_to_try:
            print "Max profiles to try succeeded (" + str(num_of_profiles) + " profiles)"
            break

    if delete_profiles == "TRUE":
        self.SendXmlFile('Scripts/GetProfileList.xml')
        profile_list = self.xmlResponse.getElementsByTagName("PROFILE_SUMMARY")
        for index in range(len(profile_list)):
            print "index " + str(index)
            if (index<num_of_profiles_already_exist):
                  continue 
            profName = profile_list[index].getElementsByTagName("NAME")[0].firstChild.data
            profId = profile_list[index].getElementsByTagName("ID")[0].firstChild.data
            if profName == "Factory_Video_Profile":
                  continue
            if profName == "Factory_GW_Profile":
                  continue
            print "Deleting Profile: " + profName + " " + str(profId)
            self.DelProfile(profId)

    print "Result: succeeded to add " + str(num_of_profiles) + " profiles"            
    return num_of_profiles
            
#------------------------------------------------------------------------------
#   try to connect maximum participant
def MaxConfrencesTest(self,confFile,max_cofrences_to_try=80,num_retries=60,deleteConf="TRUE"):
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
        sleep(1)

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
def MaxEntryQueuesTest(self,max_eqs_to_try=20,num_retries=60, delete_eqs = "TRUE"):

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
   # max_eqs_to_try = max_eqs_to_try - num_of_eqs_already_exist
    print "max_eqs_to_try " + str(max_eqs_to_try)
    while last_eq_succeeded and (num_of_eqs < max_eqs_to_try):

        EqName = "Eq_"+str(num_of_eqs+1)
        last_eq_succeeded = TryAddEQ(self,EqName,num_retries)
        if last_eq_succeeded==False:
            print "Failed to add EQ" + EqName
            self.Disconnect()
            break
        num_of_eqs = num_of_eqs +1
        print "num_of_eqs " + str(num_of_eqs)
    
        if num_of_eqs > max_eqs_to_try:
            print "Max EQs to try succeeded (" + str(num_of_eqs) + " EQs)"
            
                        
    if delete_eqs == "TRUE":
        self.SendXmlFile('Scripts/GetMRList.xml')
        mr_list = self.xmlResponse.getElementsByTagName("MEETING_ROOM_SUMMARY")
        for mr_index in range (len(mr_list)):
            if "true" == mr_list[mr_index].getElementsByTagName("ENTRY_QUEUE")[0].firstChild.data:
                eqName = mr_list[mr_index].getElementsByTagName("NAME")[0].firstChild.data
                eqId = mr_list[mr_index].getElementsByTagName("ID")[0].firstChild.data
                print "Deleting EQ: " + eqName
                self.DelReservation(eqId, "Scripts/SystemCapacityTests/RemoveMr.xml")
    
    print "Result: succeeded to add " + str(num_of_eqs) + " EQs"    

    return num_of_eqs

#------------------------------------------------------------------------------


def MaxSipFactoriesTest(self,max_sip_factories_to_try=20,num_retries=60, delete_factories = "TRUE"):

    last_factory_succeeded = True
    num_of_factories = 0


    self.SendXmlFile('Scripts/GetMRList.xml')
    mr_list = self.xmlResponse.getElementsByTagName("MEETING_ROOM_SUMMARY")
    num_of_factories_already_exist = 0
    
    for mr_index in range (len(mr_list)):
        if "true" == mr_list[mr_index].getElementsByTagName("SIP_FACTORY")[0].firstChild.data:
            factory_name = mr_list[mr_index].getElementsByTagName("NAME")[0].firstChild.data
            print "existing Sip Factoty " + factory_name
            num_of_factories_already_exist = num_of_factories_already_exist + 1

    print "num_of_sip_factories_exist = " + str(num_of_factories_already_exist)
    if num_of_factories_already_exist >= max_sip_factories_to_try:
        print "num_of_factories_already_exist >= max_sip_factories_to_try"
        return num_of_factories_already_exist

    num_of_factories = num_of_factories + num_of_factories_already_exist

    while last_factory_succeeded and (num_of_factories < max_sip_factories_to_try):

        factoryName = "SipSip__"+str(num_of_factories+1)
        last_factory_succeeded = TryAddSipFactory(self,factoryName,num_retries)
        if last_factory_succeeded==False:
            print "Failed to Sip Factory" + factoryName
        num_of_factories = num_of_factories +1
        if num_of_factories ==  max_sip_factories_to_try:
            print "Max Sip Factories to try succeeded (" + str(num_of_factories) + " sip factories)"


    if delete_factories == "TRUE":
        self.SendXmlFile('Scripts/GetMRList.xml')
        mr_list = self.xmlResponse.getElementsByTagName("MEETING_ROOM_SUMMARY")
        for mr_index in range (len(mr_list)):
            if "true" == mr_list[mr_index].getElementsByTagName("SIP_FACTORY")[0].firstChild.data:
                factoryName = mr_list[mr_index].getElementsByTagName("NAME")[0].firstChild.data
                factoryId = mr_list[mr_index].getElementsByTagName("ID")[0].firstChild.data
                #if profName != "Factory_Video_Profile":
                print "Deleting Sip Factory: " + factoryName
                self.DelReservation(factoryId, "Scripts/SystemCapacityTests/RemoveMr.xml")

    print "Result: succeeded to add " + str(num_of_factories) + " sip facoties"    

    return num_of_factories
#------------------------------------------------------------------------------


def MaxMeetingRoomsTest(self,max_meetingrooms_to_try=100,num_retries=60, delete_meetingrooms = "TRUE"):

    last_meetingroom_succeeded = True
    num_of_meeting_rooms = 0


    self.SendXmlFile('Scripts/GetMRList.xml')
    mr_list = self.xmlResponse.getElementsByTagName("MEETING_ROOM_SUMMARY")
    num_of_meeting_rooms_already_exist = 0
    
    for mr_index in range (len(mr_list)):
        if "false" == mr_list[mr_index].getElementsByTagName("SIP_FACTORY")[0].firstChild.data and "false" == mr_list[mr_index].getElementsByTagName("ENTRY_QUEUE")[0].firstChild.data:
            mr_name = mr_list[mr_index].getElementsByTagName("NAME")[0].firstChild.data
            print "existing Meeting Room " + mr_name
            num_of_meeting_rooms_already_exist = num_of_meeting_rooms_already_exist + 1

    print "num_of_meeting_rooms_already_exist = " + str(num_of_meeting_rooms_already_exist)
    if num_of_meeting_rooms_already_exist >= max_meetingrooms_to_try:
        print "num_of_meeting_rooms_already_exist >= max_meetingrooms_to_try"
        return num_of_meeting_rooms_already_exist

    num_of_meeting_rooms = num_of_meeting_rooms + num_of_meeting_rooms_already_exist

    while last_meetingroom_succeeded and (num_of_meeting_rooms < max_meetingrooms_to_try):

        EqName = "MeetingRoom__"+str(num_of_meeting_rooms+1)
        last_meetingroom_succeeded = TryAddMeetingRoom(self,EqName,num_retries)
        if last_meetingroom_succeeded==False:
            print "Failed to add Meeting Room" + EqName

        num_of_meeting_rooms = num_of_meeting_rooms +1
        if num_of_meeting_rooms ==  max_meetingrooms_to_try:
            print "Max Meeting Rooms to try succeeded (" + str(num_of_meeting_rooms) + " EQs)"
        
    if delete_meetingrooms == "TRUE":
        self.SendXmlFile('Scripts/GetMRList.xml')
        mr_list = self.xmlResponse.getElementsByTagName("MEETING_ROOM_SUMMARY")
        for mr_index in range (len(mr_list)):
            if "false" == mr_list[mr_index].getElementsByTagName("SIP_FACTORY")[0].firstChild.data and "false" == mr_list[mr_index].getElementsByTagName("ENTRY_QUEUE")[0].firstChild.data:
                meetingRoomName = mr_list[mr_index].getElementsByTagName("NAME")[0].firstChild.data
                meetingRoomId = mr_list[mr_index].getElementsByTagName("ID")[0].firstChild.data
                print "Deleting Meeting Room: " + meetingRoomName
                self.DelReservation(meetingRoomId,"Scripts/SystemCapacityTests/RemoveMr.xml")


    return num_of_meeting_rooms

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
    success = TryAddReservation(self,MrName,60,fileName,False)
    sleep(0.2)
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


    ##sleep(1) ## for EQ to be monitored
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
    if success == True:
        print "Created Reservation: " + RsrvName + ", ID = " + str(mrId)
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
# Barak capacity
#------------------------------------------------------------------------------
