#!/mcms/python/bin/python

# #############################################################################
# Change configuration of Exchange Module
#
# Date: 23/08/09
# By  : Vasily  
#
############################################################################
#
#

#from CapabilitiesSetsDefinitions import *
from McmsConnection import *

expected_status="In progress"

def SetCfg(isEnabled,url,user,password,domain,isDelegate,mailbox):
    c = McmsConnection()
    c.Connect()

    c.LoadXmlFile('Scripts/ExchangeModuleScripts/SetMcuExchangeConfigParams.xml')

    c.ModifyXml("MCU_EXCHANGE_CONFIG_PARAMS","SERVICE_ENABLED",isEnabled)
    c.ModifyXml("MCU_EXCHANGE_CONFIG_PARAMS","WEB_SERVICES_URL",url)
    c.ModifyXml("MCU_EXCHANGE_CONFIG_PARAMS","USER_NAME",user)
    c.ModifyXml("MCU_EXCHANGE_CONFIG_PARAMS","PASSWORD",password)
    c.ModifyXml("MCU_EXCHANGE_CONFIG_PARAMS","DOMAIN_NAME",domain)
    c.ModifyXml("MCU_EXCHANGE_CONFIG_PARAMS","WEB_SERVICES_DELEGATE",isDelegate)
    c.ModifyXml("MCU_EXCHANGE_CONFIG_PARAMS","PRIMARY_SMTP_MAILBOX",mailbox)

    c.Send(expected_status)

    c.Disconnect()


def TestSetCfg():
    print
    print "Start ChangeExchangeModuleConfiguration test"
    print

# bad request example (url is empty)	
#    SetCfg('true','','kobi','Mig12Pilot','accord-domain','false')
    
    print "set enabled = true "
    print
    
    SetCfg("true","https://172.22.187.253/EWS/Exchange.asmx","kobi","Mig12Pilot","exchlab.local","true","kobi@exchlab.local")
    #print "wait 10 "
    print
    #sleep (10)
    #print "set enabled = false "
    print
    #SetCfg("false","https://172.22.187.253/EWS/Exchange.asmx","vasily","Mig12Pilot","exchlab.local","false","kobi@exchlab.local")
    print
    print "End of ChangeExchangeModuleConfiguration test"
    print

TestSetCfg()

