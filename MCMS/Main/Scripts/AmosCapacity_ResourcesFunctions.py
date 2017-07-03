#!/mcms/python/bin/python
# -*- coding: utf-8 -*-#!/mcms/python/bin/python
#############################################################################
#FUNCTIONS fo Amos Capacity Scripts
#  
# Date: 04/2008
# By  : Ron
#############################################################################
from McmsConnection import *
from time import *


##===================================================================================================================##
def GetBoardsHardwareMonitoring(self):
    print "================================================================"
    print "GetBoardsHardwareMonitoring"
    self.LoadXmlFile('Scripts/hardware_monitor.xml')
    self.Send()

##        print self.xmlResponse.toprettyxml(encoding="utf-8")

    cards_list = self.xmlResponse.getElementsByTagName("CARD_SUMMARY_DESCRIPTOR")
    mpm_list = []
    for index in range(len(cards_list)):  
        card_type = cards_list[index].getElementsByTagName("CARD_TYPE")[0].firstChild.data
        slot  = cards_list[index].getElementsByTagName("SLOT_NUMBER")[0].firstChild.data
        print "Slot: " + slot + " , type: " + card_type
        if card_type == "mpm_plus_80":
            mpm_list.append(int(slot))

    print "mpm_list = " + str(mpm_list)
##    print mpm_list
    print "================================================================"    
    return mpm_list

##===================================================================================================================##
def GetMPMUnitsHardwareMonitoring(self,slot_number):

    self.LoadXmlFile('Scripts/board_monitoring.xml')
    self.ModifyXml("GET","BOARD_NUMBER",slot_number)
    self.Send()

##        print self.xmlResponse.toprettyxml(encoding="utf-8")

    print "GetMPMUnitsHardwareMonitoring"
    print "MPM+ card on slot " + str(slot_number) + ":"
    units_list = self.xmlResponse.getElementsByTagName("UNIT_RESOURCE_DESCRIPTOR")
    conf_parties_lists = GetConfsPartiesList(self)
    confs_id_name_list = conf_parties_lists[0]
    parties_id_name_list = conf_parties_lists[1]

    for index in range(len(units_list)):
        unit_type = units_list[index].getElementsByTagName("UNIT_TYPE")[0].firstChild.data
        unit_number = units_list[index].getElementsByTagName("UNIT_NUMBER")[0].firstChild.data
        unit_data = GetPortsHardwareMonitoring(self,slot_number,unit_number,confs_id_name_list,parties_id_name_list)
        util = unit_data[0]
        ports_list = unit_data[1]
        print unit_number + " ; " + unit_type + " ; " + str(util) + "/1000 ; " + str(ports_list)
##        print "unit_number: " + unit_number + " , type: " + unit_type + " , active ports[(port_id,conf_id,party_id)...]: " + str(ports_list)
    print "================================================================" 

##===================================================================================================================##
def GetPortsHardwareMonitoring(self,board_number,unit_number,confs_id_name_list,parties_id_name_list):

    self.LoadXmlFile('Scripts/unit_monitoring.xml')
    self.ModifyXml("UNIT_POSITION","BOARD_NUMBER",board_number)
    self.ModifyXml("UNIT_POSITION","UNIT_NUMBER",unit_number)
    self.Send()

    active_ports = self.xmlResponse.getElementsByTagName("PORTS_NUMBER")[0].firstChild.data
    ports_list = self.xmlResponse.getElementsByTagName("ACTIVE_PORT_DETAILS")

    self.LoadXmlFile('Scripts/TransConfList.xml')
    self.Send()
    
    active_resources_list = []
    if active_ports > 0:
        utilization=0
        for index in range(len(ports_list)):
            port_id = ports_list[index].getElementsByTagName("PORT_ID")[0].firstChild.data
            conf_id = ports_list[index].getElementsByTagName("CONF_ID")[0].firstChild.data
            party_id = ports_list[index].getElementsByTagName("PARTY_ID")[0].firstChild.data
            utilization = utilization + int(ports_list[index].getElementsByTagName("UTILIZATION")[0].firstChild.data)
            confName = "unknown_conf_name"
            partyName = "unknown_party_name"
            for c in confs_id_name_list:
                if int(conf_id) == c[0]:
                    confName = c[1]
            for p in parties_id_name_list:
                if int(conf_id)==p[0] and int(party_id)==p[1]:
                    partyName = p[2]

            active_resources_list.append((int(port_id),confName,partyName))

    return (utilization,active_resources_list)
##     print "================================================================" 
##===================================================================================================================##

def GetConfsPartiesList(self):
    
    self.LoadXmlFile('Scripts/TransConfList.xml')
    self.Send()
        ##status = self.SendXmlFileNoResponse('Scripts/TransConfList.xml')
    ongoing_conf_list = self.xmlResponse.getElementsByTagName("CONF_SUMMARY")

    confs_id_name_list = []
    parties_id_name_list = []
    for conf_index in range(len(ongoing_conf_list)):
        conf_id = ongoing_conf_list[conf_index].getElementsByTagName("ID")[0].firstChild.data
        confName = ongoing_conf_list[conf_index].getElementsByTagName("NAME")[0].firstChild.data
        confs_id_name_list.append((int(conf_id),confName))

        self.LoadXmlFile('Scripts/TransConf2.xml')
        self.ModifyXml("GET","ID",conf_id)
        self.Send()

        ongoing_parties = self.xmlResponse.getElementsByTagName("ONGOING_PARTY_STATUS")
        ongoing_party_list = self.xmlResponse.getElementsByTagName("ONGOING_PARTY")
        party_index = 0
        for party in ongoing_parties:
            current_party_id = ongoing_party_list[party_index].getElementsByTagName("ID")[0].firstChild.data
            current_party_name = ongoing_party_list[party_index].getElementsByTagName("NAME")[0].firstChild.data
            parties_id_name_list.append((int(conf_id),int(current_party_id),current_party_name))
            party_index =  party_index + 1

    return (confs_id_name_list,parties_id_name_list) 


##===================================================================================================================##

def DumpMPMCardsMonitoring(self):
    mpm_cards_list = GetBoardsHardwareMonitoring(self)
    for mpm_card_number in range(len(mpm_cards_list)):
        GetMPMUnitsHardwareMonitoring(self,mpm_cards_list[mpm_card_number])
##===================================================================================================================##


##c = McmsConnection()
##c.Connect()
##GetBoardsHardwareMonitoring(c)
##GetMPMUnitsHardwareMonitoring(c,1)
##DumpMPMCardsMonitoring(c)
##GetPortsHardwareMonitoring(c,1,1)
##c.Disconnect()
