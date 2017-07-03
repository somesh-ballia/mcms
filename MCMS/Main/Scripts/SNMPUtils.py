#!/mcms/python/bin/python

import sys
import getopt


# Import PySNMP modules
from pysnmp import asn1, v1, v2c
from pysnmp import role


# Import our modules
from SNMPAgentProxy import *




#--------------------------------------------------------------------------------------

class SNMPTrapListenerUtils:

    def StdoutPrint(self, message):
        sys.stdout.write(message + '\n')
        sys.stdout.flush()

    
    def Listen(self):
        
        # Initialize defaults
        port = 8091
        iface = 'localhost'
        community = None
    
        # Create SNMP agent object
        server = role.agent( [ (iface, port) ] )
        
        
        # Listen for SNMP messages from remote SNMP managers
        while 1:
            # Receive a request message
            (question, src) = server.receive()

            self.StdoutPrint("")
            self.StdoutPrint("----------------------------------------")
            self.StdoutPrint("")
            
            # Decode request of any version
            (req, rest) = v2c.decode(question)

            # Decode BER encoded Object IDs.
            oids = map(lambda x: x[0], map(asn1.OBJECTID().decode, req['encoded_oids']))

            # Decode BER encoded values associated with Object IDs.
            vals = map(lambda x: x[0](), map(asn1.decode, req['encoded_vals']))

            # Print it out
            self.StdoutPrint( 'SNMP message from: ' + str(src))
            self.StdoutPrint( 'Version: ' + str(req['version']+1) + ', type: ' + str(req['tag']))
            if req['version'] == 0:
                self.StdoutPrint( 'Enterprise OID: ' + str(req['enterprise']))
                self.StdoutPrint( 'Trap agent: ' + str(req['agent_addr']))
                for t in v1.GENERIC_TRAP_TYPES.keys():
                    if req['generic_trap'] == v1.GENERIC_TRAP_TYPES[t]:
                        self.StdoutPrint( 'Generic trap: %s (%d)' % (t, req['generic_trap']))
                        break
                else:
                    self.StdoutPrint( 'Generic trap: ' + str(req['generic_trap']))
                        
                self.StdoutPrint( 'Specific trap: ' + str(req['specific_trap']))
                self.StdoutPrint( 'Time stamp (uptime): ' + str(req['time_stamp']))
                        
            for (oid, val) in map(None, oids, vals):
                self.StdoutPrint( oid + ' ---> ' + str(val))

            # Verify community name if needed
            if community is not None and req['community'] != community:
                self.StdoutPrint( 'WARNING: UNMATCHED COMMUNITY NAME: ' + str(community))
                continue









#--------------------------------------------------------------------------------------

class SNMPGetUtils:
    m_Proxy = SnmpAgentProxy('Internal Communication Problem')
    m_ErrorVal = 'Internal Communication Problem'
    
    m_oidDesc           = '.1.3.6.1.2.1.1.1.0'
    m_oidObjId          = '.1.3.6.1.2.1.1.2.0'
    m_oidUpTime         = '.1.3.6.1.2.1.1.3.0'
    m_oidContact        = '.1.3.6.1.2.1.1.4.0'
    m_oidName           = '.1.3.6.1.2.1.1.5.0'
    m_oidLocation       = '.1.3.6.1.2.1.1.6.0'
    m_oidServices       = '.1.3.6.1.2.1.1.7.0'
    m_oidAlarmTable     = '.1.3.6.1.2.1.118.1.2.2.0'
    m_GatekeeperAddress = '.0.0.8.341.1.1.4.2.1.1.4.1'
 

    m_oidMCU_State ='1.3.6.1.4.1.13885.9.1.1.1.1.0'
    m_GatekeeperAddress ='0.0.8.341.1.1.4.2.1.1.4.1'
    m_oidNumber_OfInterfaces = '.1.3.6.1.2.1.2.1.0'
    def GetErrorVal(self):
    	return self.m_ErrorVal
    	
    def GetDescription(self, version = 2):
        val = self.m_Proxy.Get(self.m_oidDesc, version)
        return val
        
    def GetObjectId(self, version = 2):
        val = self.m_Proxy.Get(self.m_oidObjId, version)
        return val
        
    def GetUpTime(self, version = 2):
        val = self.m_Proxy.Get(self.m_oidUpTime, version)
        return val
   
    def GetContact(self, version = 2):
        val = self.m_Proxy.Get(self.m_oidContact, version)
        return val

    def GetName(self, version = 2):
        val = self.m_Proxy.Get(self.m_oidName, version)
        return val

    def GetLocation(self, version = 2):
        val = self.m_Proxy.Get(self.m_oidLocation, version)
        return val
    
    def GetServices(self, version = 2):
        val = self.m_Proxy.Get(self.m_oidServices, version)
        return val
    def GetMCU_State(self, version = 2):
        val = self.m_Proxy.Get(self.m_oidMCU_State, version)
        return val
    
    def GetGatekeeperAddress(self, version = 2):
        val = self.m_Proxy.Get(self.m_GatekeeperAddress, version)
        return val
    
    def GetDescriptionBadCommunityName(self, version = 2):
        val = self.m_Proxy.Get(self.m_oidDesc, version,"public1")
        return val
  
    def GetNumber_OfInterfaces(self, version = 2):
        val = self.m_Proxy.Get(self.m_oidNumber_OfInterfaces, version,"public")
        return val

    def GetAlarmTable(self, version = 2):
        i = 0
        len = 5

        val = self.m_Proxy.GetFirst(self.m_oidAlarmTable)
        print "=++++++++="
        
        while(i <> len):
            val = self.m_Proxy.GetAlarmTable(self.m_oidAlarmTable)

            # get on received oid 
#            val = self.m_Proxy.GetAlarmTable(val)
            
#            self.m_Proxy.GetFirst(val)
            
            i = i + 1


            
#            print '.'
#            print str(val)

        return val
