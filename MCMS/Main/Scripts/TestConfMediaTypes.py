#!/usr/bin/python

from McmsConnection import *
import os


## ---------------------- Test --------------------------
c = McmsConnection()
c.Connect()
##-----------SVC Conf----------------------
#add a new profile
profId = c.AddProfile("SVC_PROFILE", "Scripts/ConfMediaType/NewSvcProfile.xml")

# Creating conference
confName = "SVCOnlyConf"
c.CreateConfFromProfile(confName, profId)
confId = c.WaitConfCreated(confName)

c.LoadXmlFile('Scripts/TransConf2.xml')
c.ModifyXml("GET","ID",confId)
c.Send()

confType = c.GetTextUnder("RESERVATION","CONF_MEDIA_TYPE")
print
print "New CONF MEDIA TYPE reservation is: " + confType
   

##-----------MIX Conf----------------------
#add a new profile
profId = c.AddProfile("MIX_PROFILE", "Scripts/ConfMediaType/NewMixAvcSvcProfile.xml")

# Creating conference
confName = "SvcAvcMixConf"
c.CreateConfFromProfile(confName, profId)
confId = c.WaitConfCreated(confName)

c.LoadXmlFile('Scripts/TransConf2.xml')
c.ModifyXml("GET","ID",confId)
c.Send()

confType = c.GetTextUnder("RESERVATION","CONF_MEDIA_TYPE")
print
print "New CONF MEDIA TYPE reservation is: " + confType
   
##-----------MIX Conf when VSW is On----------------------
#add a new profile
profId = c.AddProfile("MIX_VSW_PROFILE", "Scripts/ConfMediaType/NewMixAvcSvcProfileWithVSWOn.xml")

# Creating conference
confName = "SvcAvcMixWhenVSWOn"
c.CreateConfFromProfile(confName, profId)
confId = c.WaitConfCreated(confName)

c.LoadXmlFile('Scripts/TransConf2.xml')
c.ModifyXml("GET","ID",confId)
c.Send()

confType = c.GetTextUnder("RESERVATION","CONF_MEDIA_TYPE")
isVSW = c.GetTextUnder("RESERVATION","HD")

print
print "New CONF MEDIA TYPE reservation is: " + confType + " is VSW: " +  isVSW

##-----------MIX Conf when VSW is On----------------------
#add a new profile
profId = c.AddProfile("SVC_PROFILE_AND_VSW_ON", "Scripts/ConfMediaType/NewSvcProfileWithVSWOn.xml")

# Creating conference
confName = "SvcWhenVSWOn"
c.CreateConfFromProfile(confName, profId)
confId = c.WaitConfCreated(confName)

c.LoadXmlFile('Scripts/TransConf2.xml')
c.ModifyXml("GET","ID",confId)
c.Send()

confType = c.GetTextUnder("RESERVATION","CONF_MEDIA_TYPE")
isVSW = c.GetTextUnder("RESERVATION","HD")

print
print "New CONF MEDIA TYPE reservation is: " + confType + " is VSW: " +  isVSW

