#!/mcms/python/bin/python
# -*- coding: utf-8 -*-

# Write date = 29/8/13
# Write name = Uri A.


import sys
import os
import httplib, urllib
import xml.dom.minidom
from time import *
import socket
import string
#from datetime import date
from datetime import *

from xml.dom.minidom import parse, parseString, Document

#The following redirects error stream to the standard stream, 
#to allow capture of interpreter errors in script logs
sys.stderr = sys.stdout

#from McmsConnection import *
#from CapabilitiesSetsDefinitions import *
#from PartyUtils.H323PartyUtils import *
#from PartyUtils.SipPartyUtils import *

#class PartyActions:
#    """This class activate all the party actions.
#    It uses H323, SIp and Party classes as members for that porpuse
#    """
#    m_H323PartyUtil = H323PartyUtils()
#    m_SipPartyUtil = SipPartyUtils()
#    m_PartyUtil = Party()
    
    
    
#------------------------------------------------------------------------------
#adding parties with capsets that are already defined by EP-SIM
#    def AddH323DefineCapSetParties(self, util, confName, confid, PartyNameMold = "PartyDef"):  
