#!/mcms/python/bin/python
#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_1

from ResourceUtilities import *

############################################
# test is for 2 undef and 3 real parties !!!
############################################

num_of_undef_parties = 2   

r = ResourceUtilities()

r.Connect()

r.SendXmlFile("Scripts/Resource2Undef3RealParties/ResourceMonitorCarmel.xml")
num_of_resource = r.GetTotalCarmelParties()

r.TestFreeCarmelParties(int(num_of_resource))

######################
print "Adding Conf..."

r.LoadXmlFile('Scripts/Resource2Undef3RealParties/AddCpConf.xml')
r.ModifyXml("MEET_ME_PER_CONF","ON","true")
r.ModifyXml("MEET_ME_PER_CONF","MIN_NUM_OF_PARTIES",int(num_of_undef_parties))
r.Send()

confid = r.GetTextUnder("RESERVATION","ID")

print "confId = " + confid


r.TestFreeCarmelParties( int(num_of_resource) - 2)
r.TestReservedCarmelParties(2)
r.TestOccupiedCarmelParties(0)

###########################
print "Adding 3 Parties..."

r.LoadXmlFile('Scripts/Resource2Undef3RealParties/AddVideoParty1.xml')
r.ModifyXml("ADD_PARTY","ID",int(confid))
r.Send()
sleep(1)
r.TestFreeCarmelParties( int(num_of_resource) - 2)
r.TestReservedCarmelParties(1)
r.TestOccupiedCarmelParties(1)

print "first added..."

r.LoadXmlFile('Scripts/Resource2Undef3RealParties/AddVideoParty2.xml')
r.ModifyXml("ADD_PARTY","ID",int(confid))
r.Send()
sleep(2)
r.TestFreeCarmelParties( int(num_of_resource) - 2)
r.TestReservedCarmelParties(0)
r.TestOccupiedCarmelParties(2)

print "second added..."


r.LoadXmlFile('Scripts/Resource2Undef3RealParties/AddVideoParty3.xml')
r.ModifyXml("ADD_PARTY","ID",int(confid))
r.Send()
sleep(2)
r.TestFreeCarmelParties( int(num_of_resource) - 3)
r.TestReservedCarmelParties(0)
r.TestOccupiedCarmelParties(3)

print "third added..."

#############################
print "Deleting 3 Parties..."

r.LoadXmlFile('Scripts/Resource2Undef3RealParties/TransConf2.xml')
r.ModifyXml("GET","ID",confid)
r.Send()

party_id = r.GetTextUnder("PARTY","ID")
r.DeleteParty(confid, party_id)

sleep(1)

r.TestFreeCarmelParties( int(num_of_resource) - 2)
r.TestReservedCarmelParties(0)
r.TestOccupiedCarmelParties(2)

print "first deleted..."
##

r.LoadXmlFile('Scripts/Resource2Undef3RealParties/TransConf2.xml')
r.ModifyXml("GET","ID",confid)
r.Send()

party_id = r.GetTextUnder("PARTY","ID")
r.DeleteParty(confid, party_id)

sleep(1)

r.TestFreeCarmelParties( int(num_of_resource) - 2)
r.TestReservedCarmelParties(1)
r.TestOccupiedCarmelParties(1)

print "second deleted..."
##

r.LoadXmlFile('Scripts/Resource2Undef3RealParties/TransConf2.xml')
r.ModifyXml("GET","ID",confid)
r.Send()

party_id = r.GetTextUnder("PARTY","ID")
r.DeleteParty(confid, party_id)

sleep(1)

r.TestFreeCarmelParties( int(num_of_resource) - 2)
r.TestReservedCarmelParties(2)
r.TestOccupiedCarmelParties(0)

print "third deleted..."
#############################
print "Deleting Conference..."

r.DeleteAllConf()
r.WaitAllConfEnd()

r.TestFreeCarmelParties(int(num_of_resource))
r.TestReservedCarmelParties(0)
r.TestOccupiedCarmelParties(0)

sleep(1)
r.Disconnect()




