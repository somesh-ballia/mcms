#!/mcms/python/bin/python

import sys
import getopt


# Import PySNMP modules
from pysnmp import asn1, v1, v2c
from pysnmp import role




class SnmpAgentProxy:
    m_TargetIpAddress = 'localhost'
    m_TargetPort = 8090
    m_Community = 'public'
    m_ErrorVal = 'Connection refused'


    def __init__(self, errorVal, ip = 'localhost', port = 8090, community = 'public'):

        self.m_TargetIpAddress = ip
        self.m_TargetPort = port
        self.m_Community = community
        self.m_ErrorVal = errorVal

        
    def Get(self, oid, version = 2, community_name="public",timeout = .25, retryCnt = 5):
        
        self.m_Community = community_name
        
        # Create SNMP manager object
        client = role.manager((self.m_TargetIpAddress, self.m_TargetPort))

        # Pass it a few options
        client.timeout = timeout;
        client.retries = retryCnt

        if (version == 1):
            req = v1.GETREQUEST()
        else:
            req = v2c.GETREQUEST()
        
        encoded_oids = []
        encoded_oids.append(asn1.OBJECTID().encode(oid))
        
        encoded_req = req.encode(community=self.m_Community, encoded_oids=encoded_oids)

        try:            
            (answer, src) = client.send_and_receive(encoded_req)
        except   Exception, inst:
            return self.m_ErrorVal
        
        rsp = v2c.GETRESPONSE()
        rsp.decode(answer)

        # Make sure response matches request (request IDs, communities, etc)
        if req != rsp:
            raise 'Unmatched response: %s vs %s' % (str(req), str(rsp))                                      
        
        # Decode BER encoded Object IDs.
        oids = map(lambda x: x[0], map(asn1.OBJECTID().decode, rsp['encoded_oids']))

        # Decode BER encoded values associated with Object IDs.
        vals = map(lambda x: x[0](), map(asn1.decode, rsp['encoded_vals']))

        # Check for remote SNMP agent failure
        if rsp['error_status']:
            return self.m_ErrorVal
            raise 'SNMP error #' + str(rsp['error_status']) + ' for OID #' \
                  + str(rsp['error_index'])
                 

        (oid, val) = map(None, oids, vals)[0]

        for (oid, val) in map(None, oids, vals): 
            print oid + ' ---> ' + str(val)

        return val
    



    def GetAlarmTable(self, oid, timeout = .25, retryCnt = 5):

        # Create SNMP manager object
        client = role.manager((self.m_TargetIpAddress, self.m_TargetPort))

        # Pass it a few options
        client.timeout = timeout;
        client.retries = retryCnt

        req = v2c.GETNEXTREQUEST()
        
        encoded_oids = []
        encoded_oids.append(asn1.OBJECTID().encode(oid))
        
        encoded_req = req.encode(community=self.m_Community, encoded_oids=encoded_oids)

        try:            
            (answer, src) = client.send_and_receive(encoded_req)
        except   Exception, inst:
            return self.m_ErrorVal
        
        rsp = v2c.GETRESPONSE()
        rsp.decode(answer)

        oids = map(lambda x:x[0], map(asn1.OBJECTID().decode, rsp['encoded_oids']))
        vals = map(lambda x:x[0](), map(asn1.decode, rsp['encoded_vals']))
        print str(oids) + " --> " + str(vals)

        return ""
        
        # Make sure response matches request (request IDs, communities, etc)
        if req != rsp:
            raise 'Unmatched response: %s vs %s' % (str(req), str(rsp))                                      


        
        # Decode BER encoded Object IDs.
        oids = map(lambda x: x[0], map(asn1.OBJECTID().decode, rsp['encoded_oids']))

        # Decode BER encoded values associated with Object IDs.
        vals = map(lambda x: x[0](), map(asn1.decode, rsp['encoded_vals']))

        # Check for remote SNMP agent failure
        if rsp['error_status']:
            raise 'SNMP error #' + str(rsp['error_status']) + ' for OID #' \
                  + str(rsp['error_index'])
                 

        (oid, val) = map(None, oids, vals)[0]

##         for v in vals:
##             print "v " + str(v)
        
  ##       for (oid, val) in map(None, oids, vals): 
##             print "N:" + str(src) + str(oid) + ' ---> ' + str(val)
     
        return oid
    

    def GetFirst(self, oid, timeout = .25, retryCnt = 5):
        # Create SNMP manager object
        client = role.manager((self.m_TargetIpAddress, self.m_TargetPort))

        # Pass it a few options
        client.timeout = timeout;
        client.retries = retryCnt

        req = v2c.GETREQUEST()
        
        encoded_oids = []
        encoded_oids.append(asn1.OBJECTID().encode(oid))
        
        encoded_req = req.encode(community=self.m_Community, encoded_oids=encoded_oids)

        try:            
            (answer, src) = client.send_and_receive(encoded_req)
        except   Exception, inst:
            return self.m_ErrorVal
        
        rsp = v2c.GETRESPONSE()
        rsp.decode(answer)

        oids = map(lambda x:x[0], map(asn1.OBJECTID().decode, rsp['encoded_oids']))
        vals = map(lambda x: x[0](), map(asn1.decode, rsp['encoded_vals']))
        print str(oids) + " --> " + str(vals)

        return ""
        
        # Make sure response matches request (request IDs, communities, etc)
        if req != rsp:
            raise 'Unmatched response: %s vs %s' % (str(req), str(rsp))                                      
        

        # Decode BER encoded Object IDs.
        oids = map(lambda x: x[0], map(asn1.OBJECTID().decode, rsp['encoded_oids']))

        # Decode BER encoded values associated with Object IDs.
        vals = map(lambda x: x[0](), map(asn1.decode, rsp['encoded_vals']))

        # Check for remote SNMP agent failure
        if rsp['error_status']:
            raise 'SNMP error #' + str(rsp['error_status']) + ' for OID #' \
                  + str(rsp['error_index'])
                 

        (oid, val) = map(None, oids, vals)[0]

        for (oid, val) in map(None, oids, vals): 
            print "F:" + oid + ' ---> ' + val
     
        return oid
