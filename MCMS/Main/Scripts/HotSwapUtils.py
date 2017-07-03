#!/mcms/python/bin/python

# For list of profiles look at RunTest.sh
#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_1


from McmsConnection import *
from ContentFunctions import *
import string 
import sys
import shutil


class HotSwapUtils:


#------------------------------------------------------------------------------

    def IsCardExistInHW_List(k, boardId, subBoardId, cardState):
        
        c = McmsConnection()
        c.Connect()
        
        print "boardId: " + str(boardId) + "\n"
        
        cardType = "mpm-f"        
        #MPM
        if subBoardId == 1:
           print "cardType=mpm-f"
        
        #RTM-PSTN
        if subBoardId == 2:
           print "cardType=rtm_isdn"
           cardType = "rtm_isdn"
           
        
        
        print "CardListMonitor"
        print "+++++++++++++++"
        c.SendXmlFile('Scripts/CardListMonitor.xml')
        #c.PrintLastResponse()
        cards_list = c.xmlResponse.getElementsByTagName("CARD_SUMMARY_DESCRIPTOR")
        #print cards_list
        
        num_cards_list = len(cards_list)
        print "num_cards_list: " + str(num_cards_list) + "\n"
        
        #iterate over all cards in list
        for index in range(num_cards_list):
            print "index: " + str(index)
            
            slot_number = cards_list[index].getElementsByTagName("SLOT_NUMBER")[0].firstChild.data
            print "slot_number: " + slot_number
            
            card_type = cards_list[index].getElementsByTagName("CARD_TYPE")[0].firstChild.data
            print "card_type: " + card_type
            
            card_state = cards_list[index].getElementsByTagName("CARD_STATE")[0].firstChild.data
            print "card_state: " + card_state
            
            print "\n"
            
            if int(slot_number) == boardId:
                if card_type == cardType:
                    if card_state == cardState:
                        print "\nslot_number:" + slot_number + " cardType:" + cardType + " cardState: " + cardState + " FOUND"
                        print "IsCardExistInHW_List"
                        c.Disconnect()
                        return True
                    
                   
       
        print "boardId:" + str(boardId) + " cardType:" + cardType + " cardState: " + cardState + " NOT FOUND"
        
        print "IsCardExistInHW_List\n"
        c.Disconnect()
        
        return False

#------------------------------------------------------------------------------


    def IsCardRemovedFromHW_List(k, boardId, subBoardId):
        
        c = McmsConnection()
        c.Connect()
        
        cardType = "mpm-f"        
        #MPM
        if subBoardId == 1:
           print "cardType=mpm-f"
        
        #RTM-PSTN
        if subBoardId == 2:
           print "cardType=rtm_isdn"
           cardType = "rtm_isdn"
           
        
        
        print "CardListMonitor"
        print "+++++++++++++++"
        c.SendXmlFile('Scripts/CardListMonitor.xml')
        #c.PrintLastResponse()
        cards_list = c.xmlResponse.getElementsByTagName("CARD_SUMMARY_DESCRIPTOR")
        #print cards_list
        
        num_cards_list = len(cards_list)
        print "num_cards_list: " + str(num_cards_list) + "\n"
        
        #iterate over all cards in list
        for index in range(num_cards_list):
            print "index: " + str(index)
            
            slot_number = cards_list[index].getElementsByTagName("SLOT_NUMBER")[0].firstChild.data
            print "slot_number: " + slot_number
            
            card_type = cards_list[index].getElementsByTagName("CARD_TYPE")[0].firstChild.data
            print "card_type: " + card_type
            
            card_state = cards_list[index].getElementsByTagName("CARD_STATE")[0].firstChild.data
            print "card_state: " + card_state
            
            print "\n"
            
            if int(slot_number) == boardId:
                if card_type == cardType:
                   print "\nslot_number:" + slot_number + " cardType:" + cardType + " FOUND"
                   print "IsCardRemovedFromHW_List"
                   c.Disconnect()
                   return False
                    
                   
       
        print "boardId:" + str(boardId) + " cardType:" + cardType + " NOT FOUND"
        
        print "IsCardRemovedInHW_List\n"
        c.Disconnect()
        
        return True

#------------------------------------------------------------------------------


    def SimRemoveCard(k,boardId, subBoardId):
        
        c = McmsConnection()
        c.Connect()
            
        print "<Remove Card> Board Id: " + str(boardId) + " SubBoard Id: " + str(subBoardId) 
        #c.SendXmlFile('Scripts/SimRemoveCardEvent.xml')     
        
        c.LoadXmlFile('Scripts/SimRemoveCardEvent.xml')
        c.ModifyXml("REMOVE_CARD_EVENT", "BOARD_ID", boardId)
        c.ModifyXml("REMOVE_CARD_EVENT", "SUB_BOARD_ID", subBoardId)    
        c.Send()    
        
        print "Remove Card"
        c.Disconnect()
        
        return
    
    
#------------------------------------------------------------------------------

    def SimInsertCard(k, boardId, subBoardId):
        
        c = McmsConnection()
        c.Connect()
            
        print "<Insert Card> Board Id: " + str(boardId) + " SubBoard Id: " + str(subBoardId)
        
        
        c.LoadXmlFile('Scripts/SimInsertCardEvent.xml')
        c.ModifyXml("INSERT_CARD_EVENT", "BOARD_ID", boardId)
        c.ModifyXml("INSERT_CARD_EVENT", "SUB_BOARD_ID", subBoardId)    
        c.Send()    
        
        print "Insert Card "
        c.Disconnect()       
       
        return
    
    
#------------------------------------------------------------------------------
    
    def SimDeadCard(k, boardId, subBoardId):
    
        c = McmsConnection()
        c.Connect()
        
        print "Board Id: " + str(boardId) + " SubBoard Id: " + str(subBoardId)
                
        
        c.LoadXmlFile('Scripts/SimRemoveCardEvent.xml')
        c.ModifyXml("REMOVE_CARD_EVENT", "BOARD_ID", boardId)
        c.ModifyXml("REMOVE_CARD_EVENT", "SUB_BOARD_ID", subBoardId)
        c.ModifyXml("REMOVE_CARD_EVENT", "IS_HANDLE_REMOVE", "false")
        c.Send()    
        

        print "Dead Card"
        c.Disconnect()       
       
        return
    
    
    

    
    
