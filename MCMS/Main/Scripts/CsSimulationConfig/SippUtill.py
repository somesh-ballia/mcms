#!/mcms/python/bin/python


import os, string
import sys
import subprocess
import socket
import fcntl,struct
from subprocess import call
#The following redirects error stream to the standard stream, 
#to allow capture of interpreter errors in script logs
sys.stderr = sys.stdout

class SippUtill:
      
    def __init__(self):
        self.cs_ip =  os.getenv('CS_STACK_IP_ADDRESS')
        self.host_name = os.getenv('HOSTNAME')
        self.num_calls = 1                                             # 0 is not use this option (-m               : Stop the test and exit when 'calls' calls are processed)
        self.scenario_file ="Scripts/CsSimulationConfig/rtcp-fb2.xml"  # Loads an alternate XML scenario file.
        self.call_rate = 1                                              # Set the call rate (in calls per seconds).
        self.ip_protocol = "u1"                                         # Set the transport mode:  - u1: UDP with one socket (default),t1  TCP with one socket .... (see help)                             
        self.AutoOK=1                                                   # -aa              : Enable automatic 200 OK answer for INFO, UPDATE and NOTIFY messages.
        self.trace_msg=1                                                #-trace_msg       : Displays sent and received SIP messages in <scenario filename>_<pid>_messages.log
        self.user_name="service"                                        #-s Set the username part of the request URI. Default is 'service'.
        self.DialDirection="DialIn"                                     # values DialIn or DialOut
        self.command =""                
        self.use_remote_host = 0                                        # indication to run sipp from local machine or remote machine
        self.remote_machine_host ="lnx-vm-86"
        self.remote_machine_ipv4 = "10.227.2.188"
        self.remote_share_folder = "/Carmel-Versions/TEMP/Sipp_Scripts/"    
        self.fileLock = "" 
        self.ValidateIpAndHost()
        self.testName = "UnknownTestName"
        self.authorMail = "UnknownAuthorMail"
    
    def ValidateIpAndHost( self ):
        if  (self.host_name is None):
            print "SippUtill :: warning could not extract Hostname from enviroment trying from pyton api"    
            self.host_name = socket.gethostname() 
            print self.host_name
            
        if  (self.cs_ip is None):
            print "SippUtill :: warning could not extract cs ip from enviroment trying from pyton api"    
            self.cs_ip = self.get_lan_ip()
            print self.cs_ip                

    def BuildCommandDialIn( self ):
        self.command = "/usr/local/bin/sipp "+ self.cs_ip+":5060 -s " + self.user_name +" -i " + self.host_name+":5070"
        self.command +=" -sf "+ self.scenario_file +" -t  " + self.ip_protocol
        
        if self.num_calls !=0:
            self.command +=" -m " + str(self.num_calls)
        if self.AutoOK ==1:
            self.command += " -aa"
            
        if self.trace_msg !=0:
            self.command += " -trace_msg"            
        self.command += " -r " + str(self.call_rate) +   " -bg"
        
        print "Sipp Command : " + self.command
    
    def BuildCommandDialOut( self ):
        self.command = "/usr/local/bin/sipp  -sf "+ self.scenario_file +" -t  " + self.ip_protocol 
        
        if self.num_calls !=0:
            self.command +=" -m " + str(self.num_calls)
        if self.AutoOK ==1:
            self.command += " -aa"
            
        if self.trace_msg !=0:
            self.command += " -trace_msg"            
        self.command += " -r " + str(self.call_rate) + " -i " +  self.host_name + " -bg"
        
        print "Sipp Command : " + self.command
            
    def Run(self):
        if self.use_remote_host == 0:
            if self.DialDirection == "DialIn":
                self.BuildCommandDialIn()
            else:
                self.BuildCommandDialOut()    
            self.process = subprocess.Popen(self.command , shell=True, stdout=subprocess.PIPE)
        else:
            if self.DialDirection == "DialIn":
                if( self.findAndLockRemoteMachine()==1) :
                     self.BuildCommandDialInRemoteMachine()
                     print "remote command " + self.command
                     call(["Scripts/CsSimulationConfig/sipp_proxy.sh", self.remote_machine_host,self.command])   
                else:
                    print "failed to allocate remote machine please try again later or check locks on machines"
            else:
               print "currently remote dialout not checked " 
               # self.BuildCommandDialOut()   
            
  # not supported in   mcms/python/bin/python versions
  #  def Terminate(self):
  #      self.process.terminate()
         
    def KillAllSippProcess(self):
        if self.use_remote_host == 0:
            os.system("killall sipp")
            os.system("mv Scripts/CsSimulationConfig/*.log TestResults/")
        else:
            print "clean sipp process on remote machine"
            self.RelaseRemoteMachine()
            os.system("mv /Carmel-Versions/TEMP/Sipp_Scripts/*.log TestResults/")
            
    
    def SetCommonParams(self,num_call=1,user_name='service',ip_protocol='u1',scenario_file='Scripts/CsSimulationConfig/rtcp-fb2.xml',dial_direction='DialIn'):
        self.num_calls =num_call
        self.user_name=user_name
        self.ip_protocol=ip_protocol
        self.scenario_file =scenario_file
        self.DialDirection =dial_direction
        
    def SetSippParams(self,ip_protocol='u1',num_call=1,trace_msg=1,autoOk=1):
        self.num_calls =num_call        
        self.ip_protocol=ip_protocol
        self.AutoOK=autoOk                                                 
        self.trace_msg=trace_msg
        
    def SetCallParams(self,scenario_file='Scripts/CsSimulationConfig/rtcp-fb2.xml',user_name='service',dial_direction='DialIn'):
        self.user_name=user_name        
        self.scenario_file =scenario_file
        self.DialDirection =dial_direction    
        
    def get_interface_ip(self, ifname):

        s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

        return socket.inet_ntoa(fcntl.ioctl(

                s.fileno(),

                0x8915,  # SIOCGIFADDR

                struct.pack('256s', ifname[:15])

            )[20:24])



    def get_lan_ip( self ):

        ip = socket.gethostbyname(socket.gethostname())

        if ip.startswith("127.") and os.name != "nt":

            interfaces = ["eth0","eth1","eth2"]

            for ifname in interfaces:
    
                try:
    
                    ip = self.get_interface_ip(ifname)
    
                    break;
    
                except IOError:
    
                    pass

        return ip
    
    def BuildCommandDialInRemoteMachine( self ):
        os.system("cp -u " + self.scenario_file + " " + self.remote_share_folder)
        sfile = os.path.basename(self.scenario_file)
        self.scenario_file = self.remote_share_folder + sfile
        self.command = "/usr/local/bin/sipp "+ self.cs_ip+":5060 -s " + self.user_name +" -i "+self.remote_machine_ipv4+":5060"
        self.command +=" -sf "+ self.scenario_file +" -t  " + self.ip_protocol
        
        if self.num_calls !=0:
            self.command +=" -m " + str(self.num_calls)
        if self.AutoOK ==1:
            self.command += " -aa"
            
        if self.trace_msg !=0:
            self.command += " -trace_msg"            
        self.command += " -r " + str(self.call_rate)  + " -bg"
        
    def RelaseRemoteMachine( self ):        
        if os.path.exists(self.fileLock) :
             os.system("rm " + self.fileLock)
        os.system("Scripts/CsSimulationConfig/sipp_clean_proxy.sh " + self.remote_machine_host )
            
    def findAndLockRemoteMachine( self ):
        remote_machines_names = [{"host":"lnx-vm-86","ipv4":"10.227.2.188"},{"host":"lnx-vm-53","ipv4":"10.227.2.153"}]                                         
        lock =0;
        for machine in remote_machines_names:
            
            if machine["host"] == self.host_name :
                continue
            filelock = self.remote_share_folder + machine["host"] + ".lock"
            
            if os.path.exists(filelock) :
                continue
            else :
                os.system("touch " + filelock)
                lock = 1
                self.fileLock = filelock
                self.remote_machine_host = machine["host"]
                self.remote_machine_ipv4 = machine["ipv4"]
                break
        
        return lock        
        

#for dialout
# /usr/local/bin/sipp -sf Scripts/CsSimulationConfig/effi_RMXdialout.xml -t  u1 -m 1 -aa -r 1 -i lnx-vm-ipv6-06
            