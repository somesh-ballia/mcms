#!/mcms/python/bin/python
#############################################################################
# FUNCTIONS of System Capacity Scripts 
# Programer: Ron, Keren 
# Date: 06/2008
#############################################################################
from McmsConnection import *
from time import *
from SystemCapacityFunctions import *


c = McmsConnection()
c.Connect()

#def MaxPartiesInConfTest(self,confFile,partyFile,parties_in_group,max_party_in_conf=800,num_retries=60,deleteConf="TRUE",conf_serial_number = 1):
#------------------------------------------------------------------------------
# Barak capacity
#------------------------------------------------------------------------------
#   try to connect maximum participant
def FullStatisticsTest(self,confFile,partyFile,parties_in_conf=120,num_retries=60,deleteConf="TRUE",conf_serial_number = 1):
    
    start_time = time()

    ## create conf
    confName = "Conf_"+str(conf_serial_number)
    add_conf_ok = TryAddConf(self,confName,confFile,num_retries)
    if add_conf_ok == False:
        print "Failed to add conf " + confName
    confId = GetConfId(self,confName,num_retries)

    number_of_parties_added = 0

    for party in range(parties_in_conf):

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
        if number_of_parties_added % 2:
            sleep(1)


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


FullStatisticsTest(c,'Scripts/SystemCapacityTests/AddVideoCpConf.xml','Scripts/AddVideoParty1.xml',120)

c.Disconnect()

