#!/usr/bin/python

import os
import sys


class ParseException:
    
    #----------------------------------------------------------
    def __init__(self,path,inputFile,outputFile):
        self.path=path
        self.m_inputFile = inputFile
        self.m_outputFile = outputFile
        self.m_dataList = list()
        self.m_tableDict = dict()

    #----------------------------------------------------------
    
    def ReadAllExceptions(self):
        #Run over all the output files
        #For each file find the exceptions and keep:
        # Scripts Name | Process Name | File Name | Line number | Text
        # ------------------------------------------------------------
        for directory in os.listdir(self.path):
            if ( os.path.isdir(os.path.join(self.path,directory))) :
                #For each Directory Read the test_run file
                self.GetDataFromFile(directory)

    #----------------------------------------------------------
    def GetDataFromFile(self,folderName):
        #print "Geting Data from test script: " + folderName
        folderPath = os.path.join(self.path,folderName)
        try:
            testRunFd = open(os.path.join(folderPath,self.m_inputFile),"r")
        except IOError:
            sys.stderr.write( "Error:Problem opening file: " + os.path.join(folderPath,self.m_inputFile))
            return
        
        line = testRunFd.readline()
        while line:
            if ( line .find('* EXCEPTION *') != -1):
                #Parse the Exception lines
                self.ParseException(testRunFd,folderName)
            line = testRunFd.readline()
        testRunFd.close() 

    #----------------------------------------------------------
    def ParseException(self,fd,scriptName):
        #print " Find Exception"
        fd.readline() # Read the ASSERT-FAILED line
        #Read process Name and tsk name
        line = fd.readline()
        line = line.rstrip('\n') #Remove the '\n' from the end
        
        #Get the process name
        nameIndex = line.find('Name:')
        if ( nameIndex == -1):
            #print "Error: Failed to get process name Exception lines are not ordered in line: " + line
            sys.stderr.write("Error: Script " + scriptName +" Failed to get process name Exception lines are not ordered in line: " + line)
            return
        
        processName = line[nameIndex + len('Name:') :line.find(',',nameIndex)]
        #print "1.) ProcessName = " + processName
        #Get the task name
        nameIndex = line.rfind('Name:')
        taskName = line[nameIndex + len('Name:') :]
        #print "2.) Task Name = " + taskName

        #Get File and line number
        line = fd.readline()
        line = line.lstrip('\r') #Remove the '\r' from the begining
        line = line.rstrip('\n') #Remove the '\n' from the end
        #print line
        wordList = line.split(':')
        if (len(wordList) < 3):
            #print "Error: Failed to get the file number Exception lines are not ordered in line " + line
            sys.stderr.write("Error: Script " + scriptName + " Failed to get the file number Exception lines are not ordered in line " + line)
            return
        fileName = wordList[0]
        #print "3.) File Name = " + fileName
        lineNumber = wordList[1]
        #print "4.) Line number = " + lineNumber
        #Get the text error
        descriptionPrefix = wordList[2]+ ":" #Get the description prefix
        tmpIndex = line.find(descriptionPrefix)
        tmpIndex+= len(descriptionPrefix)
        textError =  line[tmpIndex:]
        textError.rstrip('\n')
        #print "Process Name is :" + processName+ ", task name is:" +  taskName+ ",File Name="+fileName+", line:"+lineNumber+", text Error=" + textError
        #print "5.) Text Error = " +textError
        #input("Stoping:")
        tmpList = list()
        tmpList.append(scriptName)
        tmpList.append(fileName)
        tmpList.append(lineNumber)
        tmpList.append(processName)
        tmpList.append(taskName)
        tmpList.append(textError)
        
        self.m_dataList.append(tmpList)

    #----------------------------------------------------------
    def BuildDataTable(self):
        for data in self.m_dataList:
            #check if we do not got the script name in the dict create it
            if (self.m_tableDict.has_key(data[0]) == False):
                self.m_tableDict[data[0]] = dict()
                
             #Crete fileName:lineNumber
            exceptionKey = data[1]+ ":" + data[2]
             
            #Check if we already got this exception in the script data
            if (self.m_tableDict[data[0]].has_key(exceptionKey) == False) :
                tmpList = list()
                tmpList.append(data[3])
                tmpList.append(data[4])
                tmpList.append(data[5])
                (self.m_tableDict[data[0]])[exceptionKey] = tmpList
                
    #----------------------------------------------------------
    def CreateOutputFile(self):
        outFd = open(self.m_outputFile,"w+")
        
        outFd.write( "+----------------------\n")
        outFd.write( "# Exception List\n")
        outFd.write("\n")
        numOfScript=1
        for key,val in self.m_tableDict.iteritems():
            outFd.write( str(numOfScript)+". "+key+".py\n")
            outFd.write( "        -----------------------------------------------------------------------------------------------------------------------\n")
            outFd.write( "        | Process Name       |        Task Name        |                  File Name                |       Text Messsage     \n")
            outFd.write( "        ------------------------------------------------------------------------------------------------------------------------\n")
            
            for key2,val2 in val.iteritems():
                outFd.write( "        " + val2[0].ljust(25) + val2[1].ljust(25) + key2.ljust(45)  + val2[2].ljust(35) +"\n")
                outFd.write('\n')
            outFd.write('\n')
            numOfScript+=1

    #----------------------------------------------------------
    def PrintSummaryTable(self):
    	print "+----------------------------"
        print "Exception List Statistics"
        print
        print "      ---------------------------------------------------------------------------------------------------------"
        print "      Number Of Scripts   |   occurrences      |               File Name              |     Text Message       "
        print "      ---------------------------------------------------------------------------------------------------------"
        print
                
        #Create Dict with key=File Name+line
        outputDict =dict()
        for key,val in self.m_tableDict.iteritems():
            for key2,val2 in val.iteritems():
                if outputDict.has_key(key2) == False:
                    outputDict[key2]  =list()     #Createa a new list
                    outputDict[key2].append(0)    #set the scripts counter to 0
                    asertOccurences = self.CountAssertsOccurrences(key2) #Count the assert occurnces
                    outputDict[key2].append(asertOccurences)
                    
                outputDict[key2][0] += 1         #Find another assert from the same type
                outputDict[key2].append(val2[2]) # Add the description to the output table
            
        outputList = list()
        for key,val in outputDict.iteritems():
            outputList.append("      " + str(val[0]).ljust(22) +  str(val[1]).ljust(22)+ key.ljust(40) + val[2].ljust(45))

        #Sort the lines of the output ascending
        outputList.sort()

        #reverse the list to be descending order
        outputList.reverse()
        
        for line in outputList:
            print line
            
    #----------------------------------------------------------
    def CountAssertsOccurrences(self,fileName):
        counter = 0
        for tmpList in self.m_dataList:
            exceptionKey = tmpList[1]+ ":" + tmpList[2]
            if ( exceptionKey == fileName):
                counter+=1
        return counter

#----------------------------------------------------------
if __name__ == '__main__':
    if len(sys.argv) != 3:
        sys.exit("Usage: CreateExceptionFile FolderPath FullReportFileName")

    parser =ParseException(sys.argv[1],'test_run.txt',sys.argv[2]) 
    parser.ReadAllExceptions()
    parser.BuildDataTable()
    parser.CreateOutputFile()
    parser.PrintSummaryTable()
