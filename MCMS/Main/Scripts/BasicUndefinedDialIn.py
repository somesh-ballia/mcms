#!/mcms/python/bin/python

#Updated 8/2/2006
#By Yoella

from McmsConnection import *
from ISDNFunctions import *
from PartyUtils.H323PartyUtils import *
H323PartyUtilsClass = H323PartyUtils() # H323Party utils util class

#------------------------------------------------------------------------------
def BasicUndefinedDialIn(connection,num_of_parties,num_retries,disconnectBy):
    isCop = "TRUE"
    confName = "undefConf"
    connection.CreateConf(confName, 'Scripts/UndefinedDialIn/AddCpConf.xml')
    confid = connection.WaitConfCreated(confName,num_retries)
    
    #add a new profile
    ProfId = connection.AddProfile("profile")
    if isCop != "TRUE":    # no ISDN in COP version
	    #create the target Conf and wait untill it connected
	    targetEqName = "IsdnEQ"
	    eqPhone="3344"
	    connection.CreatePSTN_EQ(targetEqName, eqPhone,ProfId)
	    eqId, eqNID = connection.WaitMRCreated(targetEqName)
	    
	    #print "eqId=" +str(eqId)
    
    # SLTODO test move of h323 instead of isdn for cop
    
    delay = 1
    if(connection.IsProcessUnderValgrind("ConfParty")):
        delay = 4
    
    ### Add parties to EP Sim and connect them
    for x in range(num_of_parties):
    	if x < num_of_parties/2 or isCop == "TRUE":	# no ISDN in COP version
        	partyname = "Party"+str(x+1)
        	connection.SimulationAddH323Party(partyname, confName)
        	#print "SimulationAddH323Party name:"+partyname
        else:
        	isdnpartyname = "IsdnParty"+str(x+1)
        	phone=eqPhone
        	SimulationAddIsdnParty(connection,isdnpartyname,phone)
        	#print "SimulationAddIsdnParty name:"+isdnpartyname
        	
        	
    for y in range(num_of_parties):
    	if y < num_of_parties/2  or isCop == "TRUE":	# no ISDN in COP version
            partyname = "Party"+str(y+1) 
            connection.SimulationConnectH323Party(partyname)
            #print "SimulationConnectH323Party name:"+partyname
            sleep(delay)
        else:
        	isdnpartyname = "IsdnParty"+str(y+1)
        	#print "SimulationConnectIsdnParty name:"+isdnpartyname
        	SimulationConnectIsdnParty(connection,isdnpartyname)
        	
        	eqConfId = connection.WaitUntillEQorMRAwakes(targetEqName,1,num_retries,True)
	        print "eqConfId="+str(eqConfId)
	        
	    	#send the TDMF to the EPsim with the numeric id of the target conf
	    	targetConfNumericId = connection.GetConfNumericId(confid)
	    	connection.SimulationH323PartyDTMF(isdnpartyname, targetConfNumericId)
	    	
	    	#Sleeping for IVR messages
	    	print "Sleeping until IVR finishes..."
	    	sleep(5)
	    	

            
    connection.WaitAllPartiesWereAdded(confid,num_of_parties,num_retries*num_of_parties)
    connection.WaitAllOngoingConnected(confid,num_retries*num_of_parties, delay)

    #sleep for 3 seconds
    print "Sleeping for 3 seconds"
    sleep(3)

    # Check if all parties were added and save their IDs
    party_id_list = [0]*num_of_parties 
    party_id_list_rev = [0]*num_of_parties
    connection.LoadXmlFile('Scripts/UndefinedDialIn/TransConf2.xml')
    connection.ModifyXml("GET","ID",confid)
    connection.Send()
    ongoing_party_list = connection.xmlResponse.getElementsByTagName("ONGOING_PARTY")
    if len(ongoing_party_list) < num_of_parties:
        errMsg= "some parties are lost, find only " +str(len(ongoing_party_list)) + " parties in conf"
        sys.exit(errMsg )
    for index in range(num_of_parties):
        party_id_list_rev[(num_of_parties-index)-1]=ongoing_party_list[index].getElementsByTagName("ID")[0].firstChild.data
        #print "party_id_list_rev"+str(party_id_list_rev[index])
        
    for index in range(num_of_parties):
		party_id_list[index]=party_id_list_rev[(num_of_parties-index)-1]
		#print "party_id_list"+str(party_id_list[index])
        
    for index in range(num_of_parties):
        partyId = int(party_id_list[index])
        #print "partyId=" + str(partyId)
        partyIdByParty = partyId
        #YOELLA LEGACY was partyIdByParty = partyId+1
        #print "partyIdByParty=" + str(partyIdByParty)
        if index < num_of_parties/2 or isCop == "TRUE":	# no ISDN in COP version
        	partyName = "Party"+str(partyIdByParty)
	        if disconnectBy == "discoByParty":
        		print "Disconnecting party name: "+ partyName +" by EPSIM"
        		connection.SimulationDisconnectH323Party(partyName)
	        sleep(delay)
	            
	        if disconnectBy == "discoByEMA": 
	            print "Disconnecting party id "+ str(party_id_list[index]) +" By EMA"
	            connection.LoadXmlFile('Scripts/ReconnectParty/TransSetConnect.xml')
	            connection.ModifyXml("SET_CONNECT","ID",confid)
	            connection.ModifyXml("SET_CONNECT","CONNECT","false")
	            connection.ModifyXml("SET_CONNECT","PARTY_ID",partyId)
	       
	        connection.Send()
	        connection.SimulationDeleteH323Party(partyName)
	        
        else:
        	isdnpartyName = "IsdnParty"+str(partyIdByParty)
        	        	
        	if disconnectBy == "discoByParty":
        		print "Disconnecting party name: "+ isdnpartyName +" by EPSIM"
        		connection.SimulationDisconnectPSTNParty(isdnpartyName)
	        sleep(delay)
	        
	        if disconnectBy == "discoByEMA": 
	            print "Disconnecting party id "+ str(party_id_list[index]) +" By EMA"
	            connection.LoadXmlFile('Scripts/ReconnectParty/TransSetConnect.xml')
	            connection.ModifyXml("SET_CONNECT","ID",confid)
	            connection.ModifyXml("SET_CONNECT","CONNECT","false")
	            connection.ModifyXml("SET_CONNECT","PARTY_ID",partyId)
	            connection.Send()

	        connection.DeletePSTNPartyFromSimulation(isdnpartyName)


    connection.WaitAllOngoingDisConnected(confid,num_retries)
    connection.DeleteConf(confid)
    connection.DelProfile(ProfId, "Scripts/RemoveNewProfile.xml")
    
    if isCop != "TRUE":
	    #remove the EQ Reservation
	    connection.DeleteConf(eqConfId)
	    connection.WaitConfEnd(eqConfId)
	    connection.DelReservation(eqId, "Scripts/AwakeMrByUndef/RemoveMr.xml")
	    connection.DelProfile(ProfId)
    
    # remove all simulation parties
    H323PartyUtilsClass.SimulationDeleteAllSimParties(connection)
    
    connection.WaitAllConfEnd()
    return

#------------------------------------------------------------------------------



