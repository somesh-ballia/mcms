#!/mcms/python/bin/python

#############################################################################
# This Script do:
# Date: 15/02/06
# By  : Ron S.
#############################################################################

import os
import sys
import shutil
from CheckPerformances import *
from datetime import date
###------------------------------------------------------------------------------
current_dir = os.getcwd()  #/home/rons/projects/vob/MCMS/Main
print 'the current dir is : ' + current_dir
path = current_dir
now = date.today()  #now is in format : 2006-02-15
day = now.weekday() # day is from 0-monday till 6-sunday
#day = 0
print now
print 'the day is :' + str(day)
if(day == 0):
    yesterday = 6
else:
    yesterday = day-1

today,yesterday_name = GetDayName(day)
print today
###------------------------------------------------------------------------------

#RunProfTest('Create5ConfWith3Participants')
###------------------------------------------------------------------------------

def RunProfTest(script_name,process_name,ver,check_num):    
    
#    path = current_dir

    today_top_ten = 'Scripts/CheckPerformance/sort_' + script_name + '_' + process_name + '_' + str(day) + '.txt'
    yesterday_top_ten = 'Scripts/CheckPerformance/sort_' + script_name + '_' + process_name + '_' + str(yesterday) + '.txt' 
    command_line = 'cp '+ path +'/' + today_top_ten + ' ' + path + '/' + yesterday_top_ten
    os.system(command_line)
    
    
    command_line = 'rm ' + path + '/callgrind.out.*' + '\n'
    os.system(command_line)
    sleep(2)
    print '\n\n'
    print 'runnig prof test : ' + process_name + ' : ' + script_name
    print '\n'
    command_line = 'Scripts/Startup.sh ' + process_name + ' prof\n'
    os.system(command_line)
    sleep(2)
    command_line = 'Scripts/' + script_name + '.py' + '\n'
    
    
    new_command_line = '(time Scripts/' + script_name  + '.py)>/' + path + '/Scripts/CheckPerformance/time_' + process_name + '_' + script_name + '.txt 2>&1'
#    (time Scripts/AutoRealVoip.py)>/home/rons/ron_time.txt 2>&1
    os.system(new_command_line)
#    new_command_line = 'grep "real" ' +  path + '/Scripts/CheckPerformance/time_' + process_name + '_' + script_name + '.txt > ' + path + '/Scripts/CheckPerformance/time2_' + process_name + '_' + script_name + '.txt'
    new_command_line = 'tail -3 ' +  path + '/Scripts/CheckPerformance/time_' + process_name + '_' + script_name + '.txt > ' + path + '/Scripts/CheckPerformance/time2_' + process_name + '_' + script_name + '.txt'
    
#    grep "real" /home/rons/ron_time.txt > /home/rons/ron_time2.txt
    os.system(new_command_line)       
        
    
    
#    os.system(command_line)
    
    
    sleep(2)
    command_line = 'Scripts/Destroy.sh' + '\n'
    os.system(command_line)
    sleep(2)
    print 'copy the callgrind files '
    
    command_line = 'rm ' + path + '/Scripts/CheckPerformance/*.out.*' + '\n'
    os.system(command_line)
    #os.system('rm /home/rons/projects/vob/MCMS/Main/Scripts/CheckPerformance/*.out.*')
    
    dst_file = path + '/Scripts/CheckPerformance/callgrind.out.555'    
    #dst_file = '/home/rons/projects/vob/MCMS/Main/Scripts/CheckPerformance/callgrind.out.555'
    
    command_line = 'cp '+ path +'/callgrind.out.* ' + path + '/Scripts/CheckPerformance/callgrind.out.555'
    os.system(command_line)
    
    command_line = 'cp '+ path +'/callgrind.out.* ' + path + '/TestResults/' + process_name + '.out.' + str(ver)
    os.system(command_line)
    
    #os.system('cp /home/rons/projects/vob/MCMS/Main/callgrind.out.* /home/rons/projects/vob/MCMS/Main/Scripts/CheckPerformance/callgrind.out.555')
    
    sleep(1)
    print 'running the CheckPerformance Script ... '
    ParseFile(dst_file,script_name,process_name) 
    print 'running the Compare2LastPerformanceFiles func ... '
    Equalize2LastFile(script_name,process_name)
    
    
    total_time = TotalResults(script_name,process_name,check_num)
    return total_time
    
##----------------------------- Test -------------------------------------------------


#SCRIPTS_TO_RUN_1 = 'AutoRealVoip AutoRealVideo AddRemoveOperator AddUpdateDeleteIpService AddRemoveProfile AddRemoveMrNew AddRemoveOperator Add20ConferenceNew GetCardsMonitor GetOperatorsMonitor '
#SCRIPTS_TO_RUN_2 = 'Create5ConfWith3Participants CreateCPConfWith4DialInParticipants ReconnectParty UndefinedDialIn ConfVideoLayout MuteVideo AutoLayout PersonalLayout SameLayout PresentationMode '
#SCRIPTS_TO_RUN_3 = 'AwakeMrByUndef CreateConfWith3SipParticipants SpeakerChange EncryConf AddDeleteNewIvr AddDeleteEqService ResourceFullCapacity FECCTokenTest ShortSocketDisconnectConnect SimMfaComponentFatal '
#SCRIPTS_TO_RUN_4 = 'AddRemoveConfWrongNumericID CreateAdHoc3BlastUndefParties SipDialInToMR UndefinedSIPDialIn ResourceConfsFullCapacity UpdateParty ResourceDetailParty Capacity40UndefinedDialIn'
#SCRIPTS_TO_RUN_5 = ' VideoConfWith40Participants ForceVideo'
#SCRIPTS_TO_RUN = SCRIPTS_TO_RUN_1 + SCRIPTS_TO_RUN_2 + SCRIPTS_TO_RUN_3 + SCRIPTS_TO_RUN_4 #+ SCRIPTS_TO_RUN_5
SCRIPTS_TO_RUN = 'AutoRealVideo Create5ConfWith3Participants'
PROCESS_TO_RUN = 'Resource MplApi CSApi ConfParty'
#PROCESS_TO_RUN = 'ConfParty'

SCRIPTS_TO_RUN_LIST = SCRIPTS_TO_RUN.split(" ")
PROCESS_TO_RUN_LIST = PROCESS_TO_RUN.split(" ")

script_list_length = len(SCRIPTS_TO_RUN_LIST)
processes_list_length = len(PROCESS_TO_RUN_LIST)

command_line = 'rm ' + path + '/Scripts/CheckPerformance/performance_results_*'  + '.txt'
os.system(command_line)

ver = 400

for check_num in range(10):

    file_name = 'Scripts/CheckPerformance/performance_results_' + str(check_num) + '.txt'
    TotalResultFile = open(file_name ,'w')
    TotalResultFile.close()
    
    for x in range(script_list_length):
        
        TotalTen = open('Scripts/CheckPerformance/top_ten_funcs.txt','w')
        TotalTen.close()   
        total_time = 0
        cnt = 0     
        script_name = SCRIPTS_TO_RUN_LIST[x]    
        for y in range(processes_list_length):
            process_name = PROCESS_TO_RUN_LIST[y]
    #        file = 'Scripts/CheckPerformance/sort_' + script_name + '_' + process_name + '_' + str(yesterday) + '.txt'
    #        yesterday_file = open(file,'w')
    #        yesterday_file.close()
            
            total_time = total_time + RunProfTest(script_name,process_name,ver,check_num)
            cnt = cnt + 1
            sleep(2)
            ver = ver + 1
    
        ver = 500
        total_time_avg = float(total_time)/cnt        
        TotalTenResults(total_time_avg,script_name,check_num)

