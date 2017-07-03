#!/usr/bin/python

#############################################################################
# This Script do:
# Date: 6/04/06
# By  : Romem S.
#############################################################################

import os
import sys
import shutil
import re
#from CheckPerformances import *
from datetime import date
###------------------------------------------------------------------------------


###------------------------------------------------------------------------------
def Parse(): 
    time_table_name =  sys.argv[1]
    time_table = open(time_table_name ,'r')
    #file_name = '/home/romem/code/Python/test_leak_report.txt'
    file_name =sys.argv[2]
    script_time_table = open(file_name,'w')
    processesDict = dict()

    line_tmp = time_table.readline()
    script_start_time_name = dict()  
    script_start_cur = 0 
    reg = re.compile('-------------------[A-Z]') 
    while(line_tmp != ''):
        script_start_prev = script_start_cur
        line_tmp = time_table.readline()
        if(line_tmp == ''):
            #script_time_table.write('EOF')
            break
          
        isProcFound = reg.match(line_tmp)
        if(isProcFound):                      
            processesDict[line_tmp] =list() 
            processName=line_tmp
            
        index = line_tmp.find('blocks are definitely lost')
        if(index != -1):
            large_line_tmp = ''
            isMatch=0
            for x in range (4):
              line_tmp_1 = time_table.readline()
	      index = line_tmp_1.find('0x')
              if(index!=-1):
                line_tmp = line_tmp_1[index:] 
              	large_line_tmp += line_tmp 

            listSize=len(processesDict[processName])
            
            if(listSize):
               for y in range (listSize):
                  if(large_line_tmp==processesDict[processName][y]):
                     isMatch=1
		     print processesDict[processName][y]
                     break
            #else:
             #  processesDict[line_tmp].append(large_line_tmp) 

            if(isMatch == 0):
                processesDict[processName].append(large_line_tmp)
     
    keyList = processesDict.keys()            
    for x in range (len(keyList)):
       localList = processesDict[keyList[x]]
       if(len(localList) !=0):
          script_time_table.write(keyList[x])
         #script_time_table.write(str(len(localList)))
          script_time_table.write('\n')
          for y in range (len(localList)):
                  script_time_table.write(localList[y])
                  script_time_table.write('\n')
        
    script_time_table.close()
    time_table.close()
        
        

if __name__ =='__main__':
	if len(sys.argv) != 3 :
		sys.exit("Usage: Parsing_Leaks_Dict.py InputFileName OutptuFile")
	Parse()
      
