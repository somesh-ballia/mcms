#!/mcms/python/bin/python

#############################################################################
#Script which tests the repeated reservations feature
#it will also run under valgrind
#############################################################################

# For list of profiles look at RunTest.sh
#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_4
#-LONG_SCRIPT_TYPE

import os
from ResourceUtilities import *

c = ResourceUtilities()
c.Connect()

profId = c.AddProfile("profile_for_recurrence")
res_name = "test"
repeated_id = 0

gmt_offset = timedelta(0,0,0,0,0,2,0)
#------------------------------------------------------------------------------
def CheckStartTime(resName, t):
    #t in local time
    t = t - gmt_offset 
    c.SendXmlFile('Scripts/AddRemoveReservation/TransResList.xml',"Status OK")
    res_list = c.xmlResponse.getElementsByTagName("RES_SUMMARY")
    for index in range(len(res_list)):  
        if(resName == res_list[index].getElementsByTagName("NAME")[0].firstChild.data):
           startTime = res_list[index].getElementsByTagName("START_TIME")[0].firstChild.data
           iso_time = t.strftime("%Y-%m-%dT%H:%M:%S")
           if(iso_time == startTime):
               print "Start time of occurence: " + resName + " is indeed " + startTime
               return 1
           else: 
               c.DelRepeatedReservation(repeated_id)
               c.DelProfile(profId)   
               c.Disconnect()                
               sys.exit("Start time of occurence: " + resName + " is not as expected! Actual start time: " + startTime + " Expected start time: " + iso_time) 
    c.Disconnect()                
    sys.exit("CheckStartTime - Reservation not found " + resName) 
#------------------------------------------------------------------------------
def SetBasicParamsAndSend(confName, ProfId, t):
    #t in local time
    t = t - gmt_offset
    c.ModifyXml("RESERVATION","NAME",confName)
    c.ModifyXml("RESERVATION","DISPLAY_NAME",confName)
    c.ModifyXml("RESERVATION","AD_HOC_PROFILE_ID",str(ProfId))
    iso_time = t.strftime("%Y-%m-%dT%H:%M:%S")
    print "Adding a repeated reservation - " + confName + " with start time " + iso_time
    c.ModifyXml("RESERVATION","START_TIME",iso_time)
    #when adding repeated reservations, it might take a long time
	#especially if there are a lot of reservations
    c.Send("Status OK", 60)        
    repeated_id = c.GetTextUnder("RESERVATION","REPEATED_ID")
    print "Repeated reservation, named: " + confName + ", repeated_id = " + repeated_id + ", is added"
    sleep(3)
    return repeated_id 
#------------------------------------------------------------------------------
def SetOccurNumber(occur_num):
    print "Setting number of occurences: " + str(occur_num)
    c.ModifyXml("REPEATED_EX","OCCUR_NUM",str(occur_num))
#------------------------------------------------------------------------------
def SetLimitTime(t):
    #t in local time
    t = t - gmt_offset
    iso_time = t.strftime("%Y-%m-%dT%H:%M:%S")
    print "Setting limit of time: " + iso_time
    c.ModifyXml("REPEATED_EX","END_TIME",iso_time)
    c.ModifyXml("REPEATED_EX","LIMIT","true")
#------------------------------------------------------------------------------
def LoadDailyRepeatedXML(sunday, monday, tuesday, wednesday, thursday, friday, saturday):
    #sunday, monday, etc - "true" if we want that day "false" if we don't want it
    fileName="Scripts/AddRemoveReservation/AddRepeatedRes.xml"
    c.LoadXmlFile(fileName)
    c.ModifyXml("DAILY","SUNDAY",sunday)
    c.ModifyXml("DAILY","MONDAY",monday)
    c.ModifyXml("DAILY","TUESDAY",tuesday)
    c.ModifyXml("DAILY","WEDNESDAY",wednesday)
    c.ModifyXml("DAILY","THURSDAY",thursday)
    c.ModifyXml("DAILY","FRIDAY",friday)
    c.ModifyXml("DAILY","SATURDAY",saturday)    
#------------------------------------------------------------------------------
def LoadWeeklyRepeatedXML(sunday, monday, tuesday, wednesday, thursday, friday, saturday, interval=1):
    #sunday, monday, etc - "true" if we want that day "false" if we don't want it
    fileName="Scripts/AddRemoveReservation/AddWeeklyRepeated.xml"
    c.LoadXmlFile(fileName)
    c.ModifyXml("WEEKLY","SUNDAY",sunday)
    c.ModifyXml("WEEKLY","MONDAY",monday)
    c.ModifyXml("WEEKLY","TUESDAY",tuesday)
    c.ModifyXml("WEEKLY","WEDNESDAY",wednesday)
    c.ModifyXml("WEEKLY","THURSDAY",thursday)
    c.ModifyXml("WEEKLY","FRIDAY",friday)
    c.ModifyXml("WEEKLY","SATURDAY",saturday)     
    c.ModifyXml("WEEKLY","TIME_INTERVAL",str(interval))         
#------------------------------------------------------------------------------
def LoadMonthlyPerDateRepeatedXML(date, interval=1):
    #date - date of the month to be recurred
    fileName="Scripts/AddRemoveReservation/AddMonthlyRepeated.xml"
    c.LoadXmlFile(fileName)
    c.ModifyXml("MONTHLY","DAY_OF_MONTH",str(date))
    c.ModifyXml("MONTHLY","TIME_INTERVAL",str(interval))         
#------------------------------------------------------------------------------
def LoadMonthlyPerDayRepeatedXML(instance, day, interval=1):
    #instance - first, second, third, fourth, last
    #day - 1- sunday, 2 - monday etc
    fileName="Scripts/AddRemoveReservation/AddMonthlyRepeated.xml"
    c.LoadXmlFile(fileName)
    c.ModifyXml("MONTHLY","MONTHLY_PATTERN","by_day")   
    c.ModifyXml("MONTHLY","INSTANCE",instance)     
    c.ModifyXml("MONTHLY","DAY_OF_MONTH",str(day))
    c.ModifyXml("MONTHLY","TIME_INTERVAL",str(interval))         
#------------------------------------------------------------------------------
def CheckEverything(hour, minutes):
	#minutes: at least 1!!!!
		
	#the following line says that repeated_id is shared with the global code
	global repeated_id
	
	print "***********************************************************"
	print "***********************************************************"
	print "Checking all recurrence patterns with start hour: " + str(hour) + " and start minutes: " + str(minutes)
	print "***********************************************************"
	print "***********************************************************"
	
	print "-----------------------------------------------------------"
	print "-----------------------------------------------------------"
	print "           Daily test - with occurrence number"
	print "-----------------------------------------------------------"
	print "-----------------------------------------------------------"
	print "Check daily - 5 days a week, with 10 occurrences"
	year = 2025
	LoadDailyRepeatedXML("false","true","true","true","true","true","false")
	SetOccurNumber(10)
	repeated_id = SetBasicParamsAndSend(res_name, profId, datetime(year, 7, 1, hour, minutes))
	c.CheckNumOfReservationsInList(10)
	CheckStartTime("test_00001",datetime(year, 7, 1, hour, minutes))
	CheckStartTime("test_00002",datetime(year, 7, 2, hour, minutes))
	CheckStartTime("test_00003",datetime(year, 7, 3, hour, minutes))
	CheckStartTime("test_00004",datetime(year, 7, 4, hour, minutes))
	CheckStartTime("test_00005",datetime(year, 7, 7, hour, minutes))
	CheckStartTime("test_00006",datetime(year, 7, 8, hour, minutes))
	CheckStartTime("test_00007",datetime(year, 7, 9, hour, minutes))
	CheckStartTime("test_00008",datetime(year, 7, 10, hour, minutes))
	CheckStartTime("test_00009",datetime(year, 7, 11, hour, minutes))
	CheckStartTime("test_00010",datetime(year, 7, 14, hour, minutes))
	c.DelRepeatedReservation(repeated_id)
	c.CheckNumOfReservationsInList(0)
	print "-----------------------------------------------------------"
	print "Check daily - 7 days a week, with 9 occurrences. In February in a leap year"
	year = 2028
	LoadDailyRepeatedXML("true","true","true","true","true","true","true")
	SetOccurNumber(9)
	repeated_id = SetBasicParamsAndSend(res_name, profId, datetime(year, 2, 27, hour, minutes))
	c.CheckNumOfReservationsInList(9)
	CheckStartTime("test_00001",datetime(year, 2, 27, hour, minutes))
	CheckStartTime("test_00002",datetime(year, 2, 28, hour, minutes))
	CheckStartTime("test_00003",datetime(year, 2, 29, hour, minutes))
	CheckStartTime("test_00004",datetime(year, 3, 1, hour, minutes))
	CheckStartTime("test_00005",datetime(year, 3, 2, hour, minutes))
	CheckStartTime("test_00006",datetime(year, 3, 3, hour, minutes))
	CheckStartTime("test_00007",datetime(year, 3, 4, hour, minutes))
	CheckStartTime("test_00008",datetime(year, 3, 5, hour, minutes))
	CheckStartTime("test_00009",datetime(year, 3, 6, hour, minutes))
	c.DelRepeatedReservation(repeated_id)
	c.CheckNumOfReservationsInList(0)
	print "-----------------------------------------------------------"
	print "Check daily - 1 days a week, with 12 occurrences"
	year = 2028
	LoadDailyRepeatedXML("true","false","false","false","false","false","false")
	SetOccurNumber(12)
	repeated_id = SetBasicParamsAndSend(res_name, profId, datetime(year, 1, 1, hour, minutes))
	c.CheckNumOfReservationsInList(12)
	CheckStartTime("test_00001",datetime(year, 1, 2, hour, minutes))
	CheckStartTime("test_00002",datetime(year, 1, 9, hour, minutes))
	CheckStartTime("test_00003",datetime(year, 1, 16, hour, minutes))
	CheckStartTime("test_00004",datetime(year, 1, 23, hour, minutes))
	CheckStartTime("test_00005",datetime(year, 1, 30, hour, minutes))
	CheckStartTime("test_00006",datetime(year, 2, 6, hour, minutes))
	CheckStartTime("test_00007",datetime(year, 2, 13, hour, minutes))
	CheckStartTime("test_00008",datetime(year, 2, 20, hour, minutes))
	CheckStartTime("test_00009",datetime(year, 2, 27, hour, minutes))
	CheckStartTime("test_00010",datetime(year, 3, 5, hour, minutes))
	CheckStartTime("test_00011",datetime(year, 3, 12, hour, minutes))
	CheckStartTime("test_00012",datetime(year, 3, 19, hour, minutes))
	c.DelRepeatedReservation(repeated_id)
	c.CheckNumOfReservationsInList(0)
	
	print "-----------------------------------------------------------"
	print "-----------------------------------------------------------"
	print "           Daily test - with limit of time"
	print "-----------------------------------------------------------"
	print "-----------------------------------------------------------"
	print "Check daily - 5 days a week, with limit of time"
	LoadDailyRepeatedXML("false","true","true","true","true","true","false")
	year = 2025
	SetLimitTime(datetime(year, 7, 14, hour, minutes))
	repeated_id = SetBasicParamsAndSend(res_name, profId, datetime(year, 7, 1, hour, minutes))
	c.CheckNumOfReservationsInList(10)
	CheckStartTime("test_00001",datetime(year, 7, 1, hour, minutes))
	CheckStartTime("test_00002",datetime(year, 7, 2, hour, minutes))
	CheckStartTime("test_00003",datetime(year, 7, 3, hour, minutes))
	CheckStartTime("test_00004",datetime(year, 7, 4, hour, minutes))
	CheckStartTime("test_00005",datetime(year, 7, 7, hour, minutes))
	CheckStartTime("test_00006",datetime(year, 7, 8, hour, minutes))
	CheckStartTime("test_00007",datetime(year, 7, 9, hour, minutes))
	CheckStartTime("test_00008",datetime(year, 7, 10, hour, minutes))
	CheckStartTime("test_00009",datetime(year, 7, 11, hour, minutes))
	CheckStartTime("test_00010",datetime(year, 7, 14, hour, minutes))
	c.DelRepeatedReservation(repeated_id)
	c.CheckNumOfReservationsInList(0)
	print "-----------------------------------------------------------"
	print "Check daily - 5 days a week, with other limit of time"
	year = 2025
	LoadDailyRepeatedXML("false","true","true","true","true","true","false")
	SetLimitTime(datetime(year, 7, 15, hour, minutes))
	repeated_id = SetBasicParamsAndSend(res_name, profId, datetime(year, 7, 1, hour, minutes))
	c.CheckNumOfReservationsInList(11)
	c.DelRepeatedReservation(repeated_id)
	c.CheckNumOfReservationsInList(0)
	print "-----------------------------------------------------------"
	print "Check daily - 5 days a week, with yet another limit of time"
	year = 2025
	LoadDailyRepeatedXML("false","true","true","true","true","true","false")
	SetLimitTime(datetime(year, 7, 15, 0, 0))
	repeated_id = SetBasicParamsAndSend(res_name, profId, datetime(year, 7, 1, hour, minutes))
	c.CheckNumOfReservationsInList(10)
	c.DelRepeatedReservation(repeated_id)
	c.CheckNumOfReservationsInList(0)
	print "-----------------------------------------------------------"
	print "Check daily - 7 days a week, with limit of time, In February in a not leap year"
	year = 2027
	LoadDailyRepeatedXML("true","true","true","true","true","true","true")
	SetLimitTime(datetime(year, 3, 8, hour, minutes))
	repeated_id = SetBasicParamsAndSend(res_name, profId, datetime(year, 2, 27, hour, minutes))
	c.CheckNumOfReservationsInList(10)
	CheckStartTime("test_00001",datetime(year, 2, 27, hour, minutes))
	CheckStartTime("test_00002",datetime(year, 2, 28, hour, minutes))
	CheckStartTime("test_00003",datetime(year, 3, 1, hour, minutes))
	CheckStartTime("test_00004",datetime(year, 3, 2, hour, minutes))
	CheckStartTime("test_00005",datetime(year, 3, 3, hour, minutes))
	CheckStartTime("test_00006",datetime(year, 3, 4, hour, minutes))
	CheckStartTime("test_00007",datetime(year, 3, 5, hour, minutes))
	CheckStartTime("test_00008",datetime(year, 3, 6, hour, minutes))
	CheckStartTime("test_00009",datetime(year, 3, 7, hour, minutes))
	CheckStartTime("test_00010",datetime(year, 3, 8, hour, minutes))
	c.DelRepeatedReservation(repeated_id)
	c.CheckNumOfReservationsInList(0)
	print "-----------------------------------------------------------"
	print "Check daily - 1 days a week, with limit of time"
	year = 2028
	LoadDailyRepeatedXML("true","false","false","false","false","false","false")
	SetLimitTime(datetime(year, 3, 8, hour, minutes-1))
	repeated_id = SetBasicParamsAndSend(res_name, profId, datetime(year, 1, 2, hour, minutes))
	c.CheckNumOfReservationsInList(10)
	CheckStartTime("test_00001",datetime(year, 1, 2, hour, minutes))
	CheckStartTime("test_00002",datetime(year, 1, 9, hour, minutes))
	CheckStartTime("test_00003",datetime(year, 1, 16, hour, minutes))
	CheckStartTime("test_00004",datetime(year, 1, 23, hour, minutes))
	CheckStartTime("test_00005",datetime(year, 1, 30, hour, minutes))
	CheckStartTime("test_00006",datetime(year, 2, 6, hour, minutes))
	CheckStartTime("test_00007",datetime(year, 2, 13, hour, minutes))
	CheckStartTime("test_00008",datetime(year, 2, 20, hour, minutes))
	CheckStartTime("test_00009",datetime(year, 2, 27, hour, minutes))
	CheckStartTime("test_00010",datetime(year, 3, 5, hour, minutes))
	c.DelRepeatedReservation(repeated_id)
	c.CheckNumOfReservationsInList(0)
	
	print "-----------------------------------------------------------"
	print "-----------------------------------------------------------"
	print "           Weekly test - with occurrence number"
	print "-----------------------------------------------------------"
	print "-----------------------------------------------------------"
	print "Check weekly - 5 days a week, with 15 occurrences"
	year = 2025
	LoadWeeklyRepeatedXML("false","true","true","true","true","true","false")
	SetOccurNumber(15)
	repeated_id = SetBasicParamsAndSend(res_name, profId, datetime(year, 7, 1, hour, minutes))
	c.CheckNumOfReservationsInList(15)
	CheckStartTime("test_00001",datetime(year, 7, 1, hour, minutes))
	CheckStartTime("test_00002",datetime(year, 7, 2, hour, minutes))
	CheckStartTime("test_00003",datetime(year, 7, 3, hour, minutes))
	CheckStartTime("test_00004",datetime(year, 7, 4, hour, minutes))
	CheckStartTime("test_00005",datetime(year, 7, 7, hour, minutes))
	CheckStartTime("test_00006",datetime(year, 7, 8, hour, minutes))
	CheckStartTime("test_00007",datetime(year, 7, 9, hour, minutes))
	CheckStartTime("test_00008",datetime(year, 7, 10, hour, minutes))
	CheckStartTime("test_00009",datetime(year, 7, 11, hour, minutes))
	CheckStartTime("test_00010",datetime(year, 7, 14, hour, minutes))
	CheckStartTime("test_00011",datetime(year, 7, 15, hour, minutes))
	CheckStartTime("test_00012",datetime(year, 7, 16, hour, minutes))
	CheckStartTime("test_00013",datetime(year, 7, 17, hour, minutes))
	CheckStartTime("test_00014",datetime(year, 7, 18, hour, minutes))
	CheckStartTime("test_00015",datetime(year, 7, 21, hour, minutes))
	c.DelRepeatedReservation(repeated_id)
	c.CheckNumOfReservationsInList(0)
	print "-----------------------------------------------------------"
	print "Check weekly - 7 days a week, with 14 occurrences. In February in a leap year"
	year = 2028
	LoadWeeklyRepeatedXML("true","true","true","true","true","true","true")
	SetOccurNumber(14)
	repeated_id = SetBasicParamsAndSend(res_name, profId, datetime(year, 2, 27, hour, minutes))
	c.CheckNumOfReservationsInList(14)
	CheckStartTime("test_00001",datetime(year, 2, 27, hour, minutes))
	CheckStartTime("test_00002",datetime(year, 2, 28, hour, minutes))
	CheckStartTime("test_00003",datetime(year, 2, 29, hour, minutes))
	CheckStartTime("test_00004",datetime(year, 3, 1, hour, minutes))
	CheckStartTime("test_00005",datetime(year, 3, 2, hour, minutes))
	CheckStartTime("test_00006",datetime(year, 3, 3, hour, minutes))
	CheckStartTime("test_00007",datetime(year, 3, 4, hour, minutes))
	CheckStartTime("test_00008",datetime(year, 3, 5, hour, minutes))
	CheckStartTime("test_00009",datetime(year, 3, 6, hour, minutes))
	CheckStartTime("test_00010",datetime(year, 3, 7, hour, minutes))
	CheckStartTime("test_00011",datetime(year, 3, 8, hour, minutes))
	CheckStartTime("test_00012",datetime(year, 3, 9, hour, minutes))
	CheckStartTime("test_00013",datetime(year, 3, 10, hour, minutes))
	CheckStartTime("test_00014",datetime(year, 3, 11, hour, minutes))
	c.DelRepeatedReservation(repeated_id)
	c.CheckNumOfReservationsInList(0)
	print "-----------------------------------------------------------"
	print "Check weekly - 1 days a week, with 12 occurrences"
	year = 2028
	LoadWeeklyRepeatedXML("true","false","false","false","false","false","false")
	SetOccurNumber(12)
	repeated_id = SetBasicParamsAndSend(res_name, profId, datetime(year, 1, 1, hour, minutes))
	c.CheckNumOfReservationsInList(12)
	CheckStartTime("test_00001",datetime(year, 1, 2, hour, minutes))
	CheckStartTime("test_00002",datetime(year, 1, 9, hour, minutes))
	CheckStartTime("test_00003",datetime(year, 1, 16, hour, minutes))
	CheckStartTime("test_00004",datetime(year, 1, 23, hour, minutes))
	CheckStartTime("test_00005",datetime(year, 1, 30, hour, minutes))
	CheckStartTime("test_00006",datetime(year, 2, 6, hour, minutes))
	CheckStartTime("test_00007",datetime(year, 2, 13, hour, minutes))
	CheckStartTime("test_00008",datetime(year, 2, 20, hour, minutes))
	CheckStartTime("test_00009",datetime(year, 2, 27, hour, minutes))
	CheckStartTime("test_00010",datetime(year, 3, 5, hour, minutes))
	CheckStartTime("test_00011",datetime(year, 3, 12, hour, minutes))
	CheckStartTime("test_00012",datetime(year, 3, 19, hour, minutes))
	c.DelRepeatedReservation(repeated_id)
	c.CheckNumOfReservationsInList(0)
	print "-----------------------------------------------------------"
	print "Check weekly - 5 days a week, every 2 weeks, with 20 occurrences"
	year = 2025
	LoadWeeklyRepeatedXML("false","true","true","true","true","true","false",2)
	SetOccurNumber(20)
	repeated_id = SetBasicParamsAndSend(res_name, profId, datetime(year, 2, 3, hour, minutes))
	c.CheckNumOfReservationsInList(20)
	CheckStartTime("test_00001",datetime(year, 2, 3, hour, minutes))
	CheckStartTime("test_00002",datetime(year, 2, 4, hour, minutes))
	CheckStartTime("test_00003",datetime(year, 2, 5, hour, minutes))
	CheckStartTime("test_00004",datetime(year, 2, 6, hour, minutes))
	CheckStartTime("test_00005",datetime(year, 2, 7, hour, minutes))
	CheckStartTime("test_00006",datetime(year, 2, 17, hour, minutes))
	CheckStartTime("test_00007",datetime(year, 2, 18, hour, minutes))
	CheckStartTime("test_00008",datetime(year, 2, 19, hour, minutes))
	CheckStartTime("test_00009",datetime(year, 2, 20, hour, minutes))
	CheckStartTime("test_00010",datetime(year, 2, 21, hour, minutes))
	CheckStartTime("test_00011",datetime(year, 3, 3, hour, minutes))
	CheckStartTime("test_00012",datetime(year, 3, 4, hour, minutes))
	CheckStartTime("test_00013",datetime(year, 3, 5, hour, minutes))
	CheckStartTime("test_00014",datetime(year, 3, 6, hour, minutes))
	CheckStartTime("test_00015",datetime(year, 3, 7, hour, minutes))
	CheckStartTime("test_00016",datetime(year, 3, 17, hour, minutes))
	CheckStartTime("test_00017",datetime(year, 3, 18, hour, minutes))
	CheckStartTime("test_00018",datetime(year, 3, 19, hour, minutes))
	CheckStartTime("test_00019",datetime(year, 3, 20, hour, minutes))
	CheckStartTime("test_00020",datetime(year, 3, 21, hour, minutes))
	c.DelRepeatedReservation(repeated_id)
	c.CheckNumOfReservationsInList(0)
	print "-----------------------------------------------------------"
	print "Check weekly - 7 days a week, every 3 weeks, with 14 occurrences. In February in a leap year"
	year = 2028
	LoadWeeklyRepeatedXML("true","true","true","true","true","true","true",3)
	SetOccurNumber(14)
	repeated_id = SetBasicParamsAndSend(res_name, profId, datetime(year, 2, 27, hour, minutes))
	c.CheckNumOfReservationsInList(14)
	CheckStartTime("test_00001",datetime(year, 2, 27, hour, minutes))
	CheckStartTime("test_00002",datetime(year, 2, 28, hour, minutes))
	CheckStartTime("test_00003",datetime(year, 2, 29, hour, minutes))
	CheckStartTime("test_00004",datetime(year, 3, 1, hour, minutes))
	CheckStartTime("test_00005",datetime(year, 3, 2, hour, minutes))
	CheckStartTime("test_00006",datetime(year, 3, 3, hour, minutes))
	CheckStartTime("test_00007",datetime(year, 3, 4, hour, minutes))
	CheckStartTime("test_00008",datetime(year, 3, 19, hour, minutes))
	CheckStartTime("test_00009",datetime(year, 3, 20, hour, minutes))
	CheckStartTime("test_00010",datetime(year, 3, 21, hour, minutes))
	CheckStartTime("test_00011",datetime(year, 3, 22, hour, minutes))
	CheckStartTime("test_00012",datetime(year, 3, 23, hour, minutes))
	CheckStartTime("test_00013",datetime(year, 3, 24, hour, minutes))
	CheckStartTime("test_00014",datetime(year, 3, 25, hour, minutes))
	c.DelRepeatedReservation(repeated_id)
	c.CheckNumOfReservationsInList(0)
	print "-----------------------------------------------------------"
	print "Check weekly - 1 days a week, every 4 weeks, with 10 occurrences"
	year = 2028
	LoadWeeklyRepeatedXML("true","false","false","false","false","false","false",4)
	SetOccurNumber(10)
	repeated_id = SetBasicParamsAndSend(res_name, profId, datetime(year, 7, 2, hour, minutes))
	c.CheckNumOfReservationsInList(10)
	CheckStartTime("test_00001",datetime(year, 7, 2, hour, minutes))
	CheckStartTime("test_00002",datetime(year, 7, 30, hour, minutes))
	CheckStartTime("test_00003",datetime(year, 8, 27, hour, minutes))
	CheckStartTime("test_00004",datetime(year, 9, 24, hour, minutes))
	CheckStartTime("test_00005",datetime(year, 10, 22, hour, minutes))
	CheckStartTime("test_00006",datetime(year, 11, 19, hour, minutes))
	CheckStartTime("test_00007",datetime(year, 12, 17, hour, minutes))
	CheckStartTime("test_00008",datetime(year+1, 1, 14, hour, minutes))
	CheckStartTime("test_00009",datetime(year+1, 2, 11, hour, minutes))
	CheckStartTime("test_00010",datetime(year+1, 3, 11, hour, minutes))
	c.DelRepeatedReservation(repeated_id)
	c.CheckNumOfReservationsInList(0)
	
	print "-----------------------------------------------------------"
	print "-----------------------------------------------------------"
	print "           Weekly test - with limit of time"
	print "-----------------------------------------------------------"
	print "-----------------------------------------------------------"
	print "Check weekly - 5 days a week, with limit of time"
	LoadWeeklyRepeatedXML("false","true","true","true","true","true","false")
	year = 2025
	SetLimitTime(datetime(year, 7, 14, hour, minutes))
	repeated_id = SetBasicParamsAndSend(res_name, profId, datetime(year, 7, 1, hour, minutes))
	c.CheckNumOfReservationsInList(10)
	CheckStartTime("test_00001",datetime(year, 7, 1, hour, minutes))
	CheckStartTime("test_00002",datetime(year, 7, 2, hour, minutes))
	CheckStartTime("test_00003",datetime(year, 7, 3, hour, minutes))
	CheckStartTime("test_00004",datetime(year, 7, 4, hour, minutes))
	CheckStartTime("test_00005",datetime(year, 7, 7, hour, minutes))
	CheckStartTime("test_00006",datetime(year, 7, 8, hour, minutes))
	CheckStartTime("test_00007",datetime(year, 7, 9, hour, minutes))
	CheckStartTime("test_00008",datetime(year, 7, 10, hour, minutes))
	CheckStartTime("test_00009",datetime(year, 7, 11, hour, minutes))
	CheckStartTime("test_00010",datetime(year, 7, 14, hour, minutes))
	c.DelRepeatedReservation(repeated_id)
	c.CheckNumOfReservationsInList(0)
	print "-----------------------------------------------------------"
	print "Check weekly - 5 days a week, with other limit of time"
	year = 2025
	LoadWeeklyRepeatedXML("false","true","true","true","true","true","false")
	SetLimitTime(datetime(year, 7, 15, hour, minutes))
	repeated_id = SetBasicParamsAndSend(res_name, profId, datetime(year, 7, 1, hour, minutes))
	c.CheckNumOfReservationsInList(11)
	c.DelRepeatedReservation(repeated_id)
	c.CheckNumOfReservationsInList(0)
	print "-----------------------------------------------------------"
	print "Check weekly - 5 days a week, with yet another limit of time"
	year = 2025
	LoadWeeklyRepeatedXML("false","true","true","true","true","true","false")
	SetLimitTime(datetime(year, 7, 15, 0, 0))
	repeated_id = SetBasicParamsAndSend(res_name, profId, datetime(year, 7, 1, hour, minutes))
	c.CheckNumOfReservationsInList(10)
	c.DelRepeatedReservation(repeated_id)
	c.CheckNumOfReservationsInList(0)
	print "-----------------------------------------------------------"
	print "Check weekly - 7 days a week, with limit of time, In February in a not leap year"
	year = 2027
	LoadWeeklyRepeatedXML("true","true","true","true","true","true","true")
	SetLimitTime(datetime(year, 3, 8, hour, minutes))
	repeated_id = SetBasicParamsAndSend(res_name, profId, datetime(year, 2, 27, hour, minutes))
	c.CheckNumOfReservationsInList(10)
	CheckStartTime("test_00001",datetime(year, 2, 27, hour, minutes))
	CheckStartTime("test_00002",datetime(year, 2, 28, hour, minutes))
	CheckStartTime("test_00003",datetime(year, 3, 1, hour, minutes))
	CheckStartTime("test_00004",datetime(year, 3, 2, hour, minutes))
	CheckStartTime("test_00005",datetime(year, 3, 3, hour, minutes))
	CheckStartTime("test_00006",datetime(year, 3, 4, hour, minutes))
	CheckStartTime("test_00007",datetime(year, 3, 5, hour, minutes))
	CheckStartTime("test_00008",datetime(year, 3, 6, hour, minutes))
	CheckStartTime("test_00009",datetime(year, 3, 7, hour, minutes))
	CheckStartTime("test_00010",datetime(year, 3, 8, hour, minutes))
	c.DelRepeatedReservation(repeated_id)
	c.CheckNumOfReservationsInList(0)
	print "-----------------------------------------------------------"
	print "Check weekly - 1 days a week, with limit of time"
	year = 2028
	LoadWeeklyRepeatedXML("true","false","false","false","false","false","false")
	SetLimitTime(datetime(year, 3, 8, hour, minutes-1))
	repeated_id = SetBasicParamsAndSend(res_name, profId, datetime(year, 1, 2, hour, minutes))
	c.CheckNumOfReservationsInList(10)
	CheckStartTime("test_00001",datetime(year, 1, 2, hour, minutes))
	CheckStartTime("test_00002",datetime(year, 1, 9, hour, minutes))
	CheckStartTime("test_00003",datetime(year, 1, 16, hour, minutes))
	CheckStartTime("test_00004",datetime(year, 1, 23, hour, minutes))
	CheckStartTime("test_00005",datetime(year, 1, 30, hour, minutes))
	CheckStartTime("test_00006",datetime(year, 2, 6, hour, minutes))
	CheckStartTime("test_00007",datetime(year, 2, 13, hour, minutes))
	CheckStartTime("test_00008",datetime(year, 2, 20, hour, minutes))
	CheckStartTime("test_00009",datetime(year, 2, 27, hour, minutes))
	CheckStartTime("test_00010",datetime(year, 3, 5, hour, minutes))
	c.DelRepeatedReservation(repeated_id)
	c.CheckNumOfReservationsInList(0)
	print "-----------------------------------------------------------"
	print "Check weekly - 5 days a week, every 2 weeks, with limit of time"
	LoadWeeklyRepeatedXML("false","true","true","true","true","true","false", 2)
	year = 2022
	SetLimitTime(datetime(year+1, 1, 29, hour, minutes))
	repeated_id = SetBasicParamsAndSend(res_name, profId, datetime(year, 12, 26, hour, minutes))
	c.CheckNumOfReservationsInList(15)
	CheckStartTime("test_00001",datetime(year, 12, 26, hour, minutes))
	CheckStartTime("test_00002",datetime(year, 12, 27, hour, minutes))
	CheckStartTime("test_00003",datetime(year, 12, 28, hour, minutes))
	CheckStartTime("test_00004",datetime(year, 12, 29, hour, minutes))
	CheckStartTime("test_00005",datetime(year, 12, 30, hour, minutes))
	CheckStartTime("test_00006",datetime(year+1, 1, 9, hour, minutes))
	CheckStartTime("test_00007",datetime(year+1, 1, 10, hour, minutes))
	CheckStartTime("test_00008",datetime(year+1, 1, 11, hour, minutes))
	CheckStartTime("test_00009",datetime(year+1, 1, 12, hour, minutes))
	CheckStartTime("test_00010",datetime(year+1, 1, 13, hour, minutes))
	CheckStartTime("test_00011",datetime(year+1, 1, 23, hour, minutes))
	CheckStartTime("test_00012",datetime(year+1, 1, 24, hour, minutes))
	CheckStartTime("test_00013",datetime(year+1, 1, 25, hour, minutes))
	CheckStartTime("test_00014",datetime(year+1, 1, 26, hour, minutes))
	CheckStartTime("test_00015",datetime(year+1, 1, 27, hour, minutes))
	c.DelRepeatedReservation(repeated_id)
	c.CheckNumOfReservationsInList(0)
	print "-----------------------------------------------------------"
	print "Check weekly - 7 days a week, every 3 weeks, with limit of time, In February in a not leap year"
	year = 2023
	LoadWeeklyRepeatedXML("true","true","true","true","true","true","true", 3)
	SetLimitTime(datetime(year, 3, 23, hour, minutes-1))
	repeated_id = SetBasicParamsAndSend(res_name, profId, datetime(year, 2, 27, hour, minutes))
	c.CheckNumOfReservationsInList(10)
	CheckStartTime("test_00001",datetime(year, 2, 27, hour, minutes))
	CheckStartTime("test_00002",datetime(year, 2, 28, hour, minutes))
	CheckStartTime("test_00003",datetime(year, 3, 1, hour, minutes))
	CheckStartTime("test_00004",datetime(year, 3, 2, hour, minutes))
	CheckStartTime("test_00005",datetime(year, 3, 3, hour, minutes))
	CheckStartTime("test_00006",datetime(year, 3, 4, hour, minutes))
	CheckStartTime("test_00007",datetime(year, 3, 19, hour, minutes))
	CheckStartTime("test_00008",datetime(year, 3, 20, hour, minutes))
	CheckStartTime("test_00009",datetime(year, 3, 21, hour, minutes))
	CheckStartTime("test_00010",datetime(year, 3, 22, hour, minutes))
	c.DelRepeatedReservation(repeated_id)
	c.CheckNumOfReservationsInList(0)
	print "-----------------------------------------------------------"
	print "Check weekly - 1 days a week, every 4 weeks, with limit of time"
	year = 2028
	LoadWeeklyRepeatedXML("true","false","false","false","false","false","false",4)
	SetLimitTime(datetime(year+1, 1, 1, hour, minutes))
	repeated_id = SetBasicParamsAndSend(res_name, profId, datetime(year, 1, 2, hour, minutes))
	c.CheckNumOfReservationsInList(14)
	CheckStartTime("test_00001",datetime(year, 1, 2, hour, minutes))
	CheckStartTime("test_00002",datetime(year, 1, 30, hour, minutes))
	CheckStartTime("test_00003",datetime(year, 2, 27, hour, minutes))
	CheckStartTime("test_00004",datetime(year, 3, 26, hour, minutes))
	CheckStartTime("test_00005",datetime(year, 4, 23, hour, minutes))
	CheckStartTime("test_00006",datetime(year, 5, 21, hour, minutes))
	CheckStartTime("test_00007",datetime(year, 6, 18, hour, minutes))
	CheckStartTime("test_00008",datetime(year, 7, 16, hour, minutes))
	CheckStartTime("test_00009",datetime(year, 8, 13, hour, minutes))
	CheckStartTime("test_00010",datetime(year, 9, 10, hour, minutes))
	CheckStartTime("test_00011",datetime(year, 10, 8, hour, minutes))
	CheckStartTime("test_00012",datetime(year, 11, 5, hour, minutes))
	CheckStartTime("test_00013",datetime(year, 12, 3, hour, minutes))
	CheckStartTime("test_00014",datetime(year, 12, 31, hour, minutes))
	c.DelRepeatedReservation(repeated_id)
	c.CheckNumOfReservationsInList(0)

	print "-----------------------------------------------------------"
	print "-----------------------------------------------------------"
	print "           Monthly test per date - with occurrence number"
	print "-----------------------------------------------------------"
	print "-----------------------------------------------------------"
	print "Check Monthly - every 3 of every 1 month, with 24 0ccurences"  
	year = 2025
	LoadMonthlyPerDateRepeatedXML(3)
	SetOccurNumber(24)
	repeated_id = SetBasicParamsAndSend(res_name, profId, datetime(year, 1, 1, hour, minutes))
	c.CheckNumOfReservationsInList(24)
	CheckStartTime("test_00001",datetime(year, 1, 3, hour, minutes))
	CheckStartTime("test_00002",datetime(year, 2, 3, hour, minutes))
	CheckStartTime("test_00003",datetime(year, 3, 3, hour, minutes))
	CheckStartTime("test_00004",datetime(year, 4, 3, hour, minutes))
	CheckStartTime("test_00005",datetime(year, 5, 3, hour, minutes))
	CheckStartTime("test_00006",datetime(year, 6, 3, hour, minutes))
	CheckStartTime("test_00007",datetime(year, 7, 3, hour, minutes))
	CheckStartTime("test_00008",datetime(year, 8, 3, hour, minutes))
	CheckStartTime("test_00009",datetime(year, 9, 3, hour, minutes))
	CheckStartTime("test_00010",datetime(year, 10, 3, hour, minutes))
	CheckStartTime("test_00011",datetime(year, 11, 3, hour, minutes))
	CheckStartTime("test_00012",datetime(year, 12, 3, hour, minutes))
	CheckStartTime("test_00013",datetime(year+1, 1, 3, hour, minutes))
	CheckStartTime("test_00014",datetime(year+1, 2, 3, hour, minutes))
	CheckStartTime("test_00015",datetime(year+1, 3, 3, hour, minutes))
	CheckStartTime("test_00016",datetime(year+1, 4, 3, hour, minutes))
	CheckStartTime("test_00017",datetime(year+1, 5, 3, hour, minutes))
	CheckStartTime("test_00018",datetime(year+1, 6, 3, hour, minutes))
	CheckStartTime("test_00019",datetime(year+1, 7, 3, hour, minutes))
	CheckStartTime("test_00020",datetime(year+1, 8, 3, hour, minutes))
	CheckStartTime("test_00021",datetime(year+1, 9, 3, hour, minutes))
	CheckStartTime("test_00022",datetime(year+1, 10, 3, hour, minutes))
	CheckStartTime("test_00023",datetime(year+1, 11, 3, hour, minutes))
	CheckStartTime("test_00024",datetime(year+1, 12, 3, hour, minutes))
	c.DelRepeatedReservation(repeated_id)
	c.CheckNumOfReservationsInList(0)
	print "-----------------------------------------------------------"
	print "Check Monthly - every 31 of every 1 month, with 24 0ccurences"  
	year = 2027
	LoadMonthlyPerDateRepeatedXML(31)
	SetOccurNumber(24)
	repeated_id = SetBasicParamsAndSend(res_name, profId, datetime(year, 1, 1, hour, minutes))
	c.CheckNumOfReservationsInList(24)
	CheckStartTime("test_00001",datetime(year, 1, 31, hour, minutes))
	CheckStartTime("test_00002",datetime(year, 2, 28, hour, minutes))
	CheckStartTime("test_00003",datetime(year, 3, 31, hour, minutes))
	CheckStartTime("test_00004",datetime(year, 4, 30, hour, minutes))
	CheckStartTime("test_00005",datetime(year, 5, 31, hour, minutes))
	CheckStartTime("test_00006",datetime(year, 6, 30, hour, minutes))
	CheckStartTime("test_00007",datetime(year, 7, 31, hour, minutes))
	CheckStartTime("test_00008",datetime(year, 8, 31, hour, minutes))
	CheckStartTime("test_00009",datetime(year, 9, 30, hour, minutes))
	CheckStartTime("test_00010",datetime(year, 10, 31, hour, minutes))
	CheckStartTime("test_00011",datetime(year, 11, 30, hour, minutes))
	CheckStartTime("test_00012",datetime(year, 12, 31, hour, minutes))
	CheckStartTime("test_00013",datetime(year+1, 1, 31, hour, minutes))
	CheckStartTime("test_00014",datetime(year+1, 2, 29, hour, minutes))
	CheckStartTime("test_00015",datetime(year+1, 3, 31, hour, minutes))
	CheckStartTime("test_00016",datetime(year+1, 4, 30, hour, minutes))
	CheckStartTime("test_00017",datetime(year+1, 5, 31, hour, minutes))
	CheckStartTime("test_00018",datetime(year+1, 6, 30, hour, minutes))
	CheckStartTime("test_00019",datetime(year+1, 7, 31, hour, minutes))
	CheckStartTime("test_00020",datetime(year+1, 8, 31, hour, minutes))
	CheckStartTime("test_00021",datetime(year+1, 9, 30, hour, minutes))
	CheckStartTime("test_00022",datetime(year+1, 10, 31, hour, minutes))
	CheckStartTime("test_00023",datetime(year+1, 11, 30, hour, minutes))
	CheckStartTime("test_00024",datetime(year+1, 12, 31, hour, minutes))
	c.DelRepeatedReservation(repeated_id)
	c.CheckNumOfReservationsInList(0)
	print "-----------------------------------------------------------"
	print "Check Monthly - every 1 of every 1 month, with 12 0ccurences"  
	year = 2027
	LoadMonthlyPerDateRepeatedXML(1)
	SetOccurNumber(12)
	repeated_id = SetBasicParamsAndSend(res_name, profId, datetime(year, 1, 1, hour, minutes))
	c.CheckNumOfReservationsInList(12)
	CheckStartTime("test_00001",datetime(year, 1, 1, hour, minutes))
	CheckStartTime("test_00002",datetime(year, 2, 1, hour, minutes))
	CheckStartTime("test_00003",datetime(year, 3, 1, hour, minutes))
	CheckStartTime("test_00004",datetime(year, 4, 1, hour, minutes))
	CheckStartTime("test_00005",datetime(year, 5, 1, hour, minutes))
	CheckStartTime("test_00006",datetime(year, 6, 1, hour, minutes))
	CheckStartTime("test_00007",datetime(year, 7, 1, hour, minutes))
	CheckStartTime("test_00008",datetime(year, 8, 1, hour, minutes))
	CheckStartTime("test_00009",datetime(year, 9, 1, hour, minutes))
	CheckStartTime("test_00010",datetime(year, 10, 1, hour, minutes))
	CheckStartTime("test_00011",datetime(year, 11, 1, hour, minutes))
	CheckStartTime("test_00012",datetime(year, 12, 1, hour, minutes))
	c.DelRepeatedReservation(repeated_id)
	c.CheckNumOfReservationsInList(0)
	print "-----------------------------------------------------------"
	print "Check Monthly - every 20 of every 3 months, with 12 0ccurences"  
	year = 2025
	LoadMonthlyPerDateRepeatedXML(20, 3)
	SetOccurNumber(12)
	repeated_id = SetBasicParamsAndSend(res_name, profId, datetime(year, 1, 1, hour, minutes))
	c.CheckNumOfReservationsInList(12)
	CheckStartTime("test_00001",datetime(year, 1, 20, hour, minutes))
	CheckStartTime("test_00002",datetime(year, 4, 20, hour, minutes))
	CheckStartTime("test_00003",datetime(year, 7, 20, hour, minutes))
	CheckStartTime("test_00004",datetime(year, 10, 20, hour, minutes))
	CheckStartTime("test_00005",datetime(year+1, 1, 20, hour, minutes))
	CheckStartTime("test_00006",datetime(year+1, 4, 20, hour, minutes))
	CheckStartTime("test_00007",datetime(year+1, 7, 20, hour, minutes))
	CheckStartTime("test_00008",datetime(year+1, 10, 20, hour, minutes))
	CheckStartTime("test_00009",datetime(year+2, 1, 20, hour, minutes))
	CheckStartTime("test_00010",datetime(year+2, 4, 20, hour, minutes))
	CheckStartTime("test_00011",datetime(year+2, 7, 20, hour, minutes))
	CheckStartTime("test_00012",datetime(year+2, 10, 20, hour, minutes))
	c.DelRepeatedReservation(repeated_id)
	c.CheckNumOfReservationsInList(0)
	print "-----------------------------------------------------------"
	print "Check Monthly - every 1 of every 6 month, with 8 0ccurences"  
	year = 2027
	LoadMonthlyPerDateRepeatedXML(1,6)
	SetOccurNumber(8)
	repeated_id = SetBasicParamsAndSend(res_name, profId, datetime(year, 1, 1, hour, minutes))
	c.CheckNumOfReservationsInList(8)
	CheckStartTime("test_00001",datetime(year, 1, 1, hour, minutes))
	CheckStartTime("test_00002",datetime(year, 7, 1, hour, minutes))
	CheckStartTime("test_00003",datetime(year+1, 1, 1, hour, minutes))
	CheckStartTime("test_00004",datetime(year+1, 7, 1, hour, minutes))
	CheckStartTime("test_00005",datetime(year+2, 1, 1, hour, minutes))
	CheckStartTime("test_00006",datetime(year+2, 7, 1, hour, minutes))
	CheckStartTime("test_00007",datetime(year+3, 1, 1, hour, minutes))
	CheckStartTime("test_00008",datetime(year+3, 7, 1, hour, minutes))
	c.DelRepeatedReservation(repeated_id)
	c.CheckNumOfReservationsInList(0)

	print "-----------------------------------------------------------"
	print "-----------------------------------------------------------"
	print "           Monthly test per date - with limit time"
	print "-----------------------------------------------------------"
	print "-----------------------------------------------------------"
	print "Check Monthly - every 3 of every 1 month, with limit time"  
	year = 2025
	LoadMonthlyPerDateRepeatedXML(3)
	SetLimitTime(datetime(year+1, 12, 3, hour, minutes))
	repeated_id = SetBasicParamsAndSend(res_name, profId, datetime(year, 1, 1, hour, minutes))
	c.CheckNumOfReservationsInList(24)
	CheckStartTime("test_00001",datetime(year, 1, 3, hour, minutes))
	CheckStartTime("test_00002",datetime(year, 2, 3, hour, minutes))
	CheckStartTime("test_00003",datetime(year, 3, 3, hour, minutes))
	CheckStartTime("test_00004",datetime(year, 4, 3, hour, minutes))
	CheckStartTime("test_00005",datetime(year, 5, 3, hour, minutes))
	CheckStartTime("test_00006",datetime(year, 6, 3, hour, minutes))
	CheckStartTime("test_00007",datetime(year, 7, 3, hour, minutes))
	CheckStartTime("test_00008",datetime(year, 8, 3, hour, minutes))
	CheckStartTime("test_00009",datetime(year, 9, 3, hour, minutes))
	CheckStartTime("test_00010",datetime(year, 10, 3, hour, minutes))
	CheckStartTime("test_00011",datetime(year, 11, 3, hour, minutes))
	CheckStartTime("test_00012",datetime(year, 12, 3, hour, minutes))
	CheckStartTime("test_00013",datetime(year+1, 1, 3, hour, minutes))
	CheckStartTime("test_00014",datetime(year+1, 2, 3, hour, minutes))
	CheckStartTime("test_00015",datetime(year+1, 3, 3, hour, minutes))
	CheckStartTime("test_00016",datetime(year+1, 4, 3, hour, minutes))
	CheckStartTime("test_00017",datetime(year+1, 5, 3, hour, minutes))
	CheckStartTime("test_00018",datetime(year+1, 6, 3, hour, minutes))
	CheckStartTime("test_00019",datetime(year+1, 7, 3, hour, minutes))
	CheckStartTime("test_00020",datetime(year+1, 8, 3, hour, minutes))
	CheckStartTime("test_00021",datetime(year+1, 9, 3, hour, minutes))
	CheckStartTime("test_00022",datetime(year+1, 10, 3, hour, minutes))
	CheckStartTime("test_00023",datetime(year+1, 11, 3, hour, minutes))
	CheckStartTime("test_00024",datetime(year+1, 12, 3, hour, minutes))
	c.DelRepeatedReservation(repeated_id)
	c.CheckNumOfReservationsInList(0)
	print "-----------------------------------------------------------"
	print "Check Monthly - every 31 of every 1 month, with limit time"  
	year = 2027
	LoadMonthlyPerDateRepeatedXML(31)
	SetLimitTime(datetime(year+2, 1, 31, hour, minutes-1))
	repeated_id = SetBasicParamsAndSend(res_name, profId, datetime(year, 1, 1, hour, minutes))
	c.CheckNumOfReservationsInList(24)
	CheckStartTime("test_00001",datetime(year, 1, 31, hour, minutes))
	CheckStartTime("test_00002",datetime(year, 2, 28, hour, minutes))
	CheckStartTime("test_00003",datetime(year, 3, 31, hour, minutes))
	CheckStartTime("test_00004",datetime(year, 4, 30, hour, minutes))
	CheckStartTime("test_00005",datetime(year, 5, 31, hour, minutes))
	CheckStartTime("test_00006",datetime(year, 6, 30, hour, minutes))
	CheckStartTime("test_00007",datetime(year, 7, 31, hour, minutes))
	CheckStartTime("test_00008",datetime(year, 8, 31, hour, minutes))
	CheckStartTime("test_00009",datetime(year, 9, 30, hour, minutes))
	CheckStartTime("test_00010",datetime(year, 10, 31, hour, minutes))
	CheckStartTime("test_00011",datetime(year, 11, 30, hour, minutes))
	CheckStartTime("test_00012",datetime(year, 12, 31, hour, minutes))
	CheckStartTime("test_00013",datetime(year+1, 1, 31, hour, minutes))
	CheckStartTime("test_00014",datetime(year+1, 2, 29, hour, minutes))
	CheckStartTime("test_00015",datetime(year+1, 3, 31, hour, minutes))
	CheckStartTime("test_00016",datetime(year+1, 4, 30, hour, minutes))
	CheckStartTime("test_00017",datetime(year+1, 5, 31, hour, minutes))
	CheckStartTime("test_00018",datetime(year+1, 6, 30, hour, minutes))
	CheckStartTime("test_00019",datetime(year+1, 7, 31, hour, minutes))
	CheckStartTime("test_00020",datetime(year+1, 8, 31, hour, minutes))
	CheckStartTime("test_00021",datetime(year+1, 9, 30, hour, minutes))
	CheckStartTime("test_00022",datetime(year+1, 10, 31, hour, minutes))
	CheckStartTime("test_00023",datetime(year+1, 11, 30, hour, minutes))
	CheckStartTime("test_00024",datetime(year+1, 12, 31, hour, minutes))
	c.DelRepeatedReservation(repeated_id)
	c.CheckNumOfReservationsInList(0)
	print "-----------------------------------------------------------"
	print "Check Monthly - every 1 of every 1 month, with limit time"  
	year = 2027
	LoadMonthlyPerDateRepeatedXML(1)
	SetLimitTime(datetime(year+1, 1, 1, hour, minutes-1))
	repeated_id = SetBasicParamsAndSend(res_name, profId, datetime(year, 1, 1, hour, minutes))
	c.CheckNumOfReservationsInList(12)
	CheckStartTime("test_00001",datetime(year, 1, 1, hour, minutes))
	CheckStartTime("test_00002",datetime(year, 2, 1, hour, minutes))
	CheckStartTime("test_00003",datetime(year, 3, 1, hour, minutes))
	CheckStartTime("test_00004",datetime(year, 4, 1, hour, minutes))
	CheckStartTime("test_00005",datetime(year, 5, 1, hour, minutes))
	CheckStartTime("test_00006",datetime(year, 6, 1, hour, minutes))
	CheckStartTime("test_00007",datetime(year, 7, 1, hour, minutes))
	CheckStartTime("test_00008",datetime(year, 8, 1, hour, minutes))
	CheckStartTime("test_00009",datetime(year, 9, 1, hour, minutes))
	CheckStartTime("test_00010",datetime(year, 10, 1, hour, minutes))
	CheckStartTime("test_00011",datetime(year, 11, 1, hour, minutes))
	CheckStartTime("test_00012",datetime(year, 12, 1, hour, minutes))
	c.DelRepeatedReservation(repeated_id)
	c.CheckNumOfReservationsInList(0)
	print "-----------------------------------------------------------"
	print "Check Monthly - every 20 of every 3 months, with limit time"  
	year = 2025
	LoadMonthlyPerDateRepeatedXML(20, 3)
	SetLimitTime(datetime(year+2, 10, 20, hour, minutes))
	repeated_id = SetBasicParamsAndSend(res_name, profId, datetime(year, 1, 1, hour, minutes))
	c.CheckNumOfReservationsInList(12)
	CheckStartTime("test_00001",datetime(year, 1, 20, hour, minutes))
	CheckStartTime("test_00002",datetime(year, 4, 20, hour, minutes))
	CheckStartTime("test_00003",datetime(year, 7, 20, hour, minutes))
	CheckStartTime("test_00004",datetime(year, 10, 20, hour, minutes))
	CheckStartTime("test_00005",datetime(year+1, 1, 20, hour, minutes))
	CheckStartTime("test_00006",datetime(year+1, 4, 20, hour, minutes))
	CheckStartTime("test_00007",datetime(year+1, 7, 20, hour, minutes))
	CheckStartTime("test_00008",datetime(year+1, 10, 20, hour, minutes))
	CheckStartTime("test_00009",datetime(year+2, 1, 20, hour, minutes))
	CheckStartTime("test_00010",datetime(year+2, 4, 20, hour, minutes))
	CheckStartTime("test_00011",datetime(year+2, 7, 20, hour, minutes))
	CheckStartTime("test_00012",datetime(year+2, 10, 20, hour, minutes))
	c.DelRepeatedReservation(repeated_id)
	c.CheckNumOfReservationsInList(0)
	print "-----------------------------------------------------------"
	print "Check Monthly - every 1 of every 6 month, with limit time"  
	year = 2027
	LoadMonthlyPerDateRepeatedXML(1,6)
	SetLimitTime(datetime(year+4, 1, 1, hour, minutes-1))
	repeated_id = SetBasicParamsAndSend(res_name, profId, datetime(year, 1, 1, hour, minutes))
	c.CheckNumOfReservationsInList(8)
	CheckStartTime("test_00001",datetime(year, 1, 1, hour, minutes))
	CheckStartTime("test_00002",datetime(year, 7, 1, hour, minutes))
	CheckStartTime("test_00003",datetime(year+1, 1, 1, hour, minutes))
	CheckStartTime("test_00004",datetime(year+1, 7, 1, hour, minutes))
	CheckStartTime("test_00005",datetime(year+2, 1, 1, hour, minutes))
	CheckStartTime("test_00006",datetime(year+2, 7, 1, hour, minutes))
	CheckStartTime("test_00007",datetime(year+3, 1, 1, hour, minutes))
	CheckStartTime("test_00008",datetime(year+3, 7, 1, hour, minutes))
	c.DelRepeatedReservation(repeated_id)
	c.CheckNumOfReservationsInList(0)
	
	print "-----------------------------------------------------------"
	print "-----------------------------------------------------------"
	print "           Monthly test per day - with occurrence number"
	print "-----------------------------------------------------------"
	print "-----------------------------------------------------------"
	print "Check Monthly - first sunday of every month, with 24 occurrences"  
	year = 2027
	LoadMonthlyPerDayRepeatedXML("first",1)
	SetOccurNumber(24)
	repeated_id = SetBasicParamsAndSend(res_name, profId, datetime(year, 1, 1, hour, minutes))
	c.CheckNumOfReservationsInList(24)
	CheckStartTime("test_00001",datetime(year, 1, 3, hour, minutes))
	CheckStartTime("test_00002",datetime(year, 2, 7, hour, minutes))
	CheckStartTime("test_00003",datetime(year, 3, 7, hour, minutes))
	CheckStartTime("test_00004",datetime(year, 4, 4, hour, minutes))
	CheckStartTime("test_00005",datetime(year, 5, 2, hour, minutes))
	CheckStartTime("test_00006",datetime(year, 6, 6, hour, minutes))
	CheckStartTime("test_00007",datetime(year, 7, 4, hour, minutes))
	CheckStartTime("test_00008",datetime(year, 8, 1, hour, minutes))
	CheckStartTime("test_00009",datetime(year, 9, 5, hour, minutes))
	CheckStartTime("test_00010",datetime(year, 10, 3, hour, minutes))
	CheckStartTime("test_00011",datetime(year, 11, 7, hour, minutes))
	CheckStartTime("test_00012",datetime(year, 12, 5, hour, minutes))
	CheckStartTime("test_00013",datetime(year+1, 1, 2, hour, minutes))
	CheckStartTime("test_00014",datetime(year+1, 2, 6, hour, minutes))
	CheckStartTime("test_00015",datetime(year+1, 3, 5, hour, minutes))
	CheckStartTime("test_00016",datetime(year+1, 4, 2, hour, minutes))
	CheckStartTime("test_00017",datetime(year+1, 5, 7, hour, minutes))
	CheckStartTime("test_00018",datetime(year+1, 6, 4, hour, minutes))
	CheckStartTime("test_00019",datetime(year+1, 7, 2, hour, minutes))
	CheckStartTime("test_00020",datetime(year+1, 8, 6, hour, minutes))
	CheckStartTime("test_00021",datetime(year+1, 9, 3, hour, minutes))
	CheckStartTime("test_00022",datetime(year+1, 10, 1, hour, minutes))
	CheckStartTime("test_00023",datetime(year+1, 11, 5, hour, minutes))
	CheckStartTime("test_00024",datetime(year+1, 12, 3, hour, minutes))
	c.DelRepeatedReservation(repeated_id)
	c.CheckNumOfReservationsInList(0)
	print "-----------------------------------------------------------"
	print "Check Monthly - second monday of every month, with 12 occurrences"  
	year = 2027
	LoadMonthlyPerDayRepeatedXML("second",2)
	SetOccurNumber(12)
	repeated_id = SetBasicParamsAndSend(res_name, profId, datetime(year, 1, 1, hour, minutes))
	c.CheckNumOfReservationsInList(12)
	CheckStartTime("test_00001",datetime(year, 1, 11, hour, minutes))
	CheckStartTime("test_00002",datetime(year, 2, 8, hour, minutes))
	CheckStartTime("test_00003",datetime(year, 3, 8, hour, minutes))
	CheckStartTime("test_00004",datetime(year, 4, 12, hour, minutes))
	CheckStartTime("test_00005",datetime(year, 5, 10, hour, minutes))
	CheckStartTime("test_00006",datetime(year, 6, 14, hour, minutes))
	CheckStartTime("test_00007",datetime(year, 7, 12, hour, minutes))
	CheckStartTime("test_00008",datetime(year, 8, 9, hour, minutes))
	CheckStartTime("test_00009",datetime(year, 9, 13, hour, minutes))
	CheckStartTime("test_00010",datetime(year, 10, 11, hour, minutes))
	CheckStartTime("test_00011",datetime(year, 11, 8, hour, minutes))
	CheckStartTime("test_00012",datetime(year, 12, 13, hour, minutes))
	c.DelRepeatedReservation(repeated_id)
	c.CheckNumOfReservationsInList(0)
	print "-----------------------------------------------------------"
	print "Check Monthly - third thuesday of every month, with 6 occurrences"  
	year = 2027
	LoadMonthlyPerDayRepeatedXML("third",3)
	SetOccurNumber(6)
	repeated_id = SetBasicParamsAndSend(res_name, profId, datetime(year, 1, 1, hour, minutes))
	c.CheckNumOfReservationsInList(6)
	CheckStartTime("test_00001",datetime(year, 1, 19, hour, minutes))
	CheckStartTime("test_00002",datetime(year, 2, 16, hour, minutes))
	CheckStartTime("test_00003",datetime(year, 3, 16, hour, minutes))
	CheckStartTime("test_00004",datetime(year, 4, 20, hour, minutes))
	CheckStartTime("test_00005",datetime(year, 5, 18, hour, minutes))
	CheckStartTime("test_00006",datetime(year, 6, 15, hour, minutes))
	c.DelRepeatedReservation(repeated_id)
	c.CheckNumOfReservationsInList(0)
	print "-----------------------------------------------------------"
	print "Check Monthly - forth saturday of every month, with 24 occurrences"  
	year = 2027
	LoadMonthlyPerDayRepeatedXML("fourth",7)
	SetOccurNumber(24)
	repeated_id = SetBasicParamsAndSend(res_name, profId, datetime(year, 1, 1, hour, minutes))
	c.CheckNumOfReservationsInList(24)
	CheckStartTime("test_00001",datetime(year, 1, 23, hour, minutes))
	CheckStartTime("test_00002",datetime(year, 2, 27, hour, minutes))
	CheckStartTime("test_00003",datetime(year, 3, 27, hour, minutes))
	CheckStartTime("test_00004",datetime(year, 4, 24, hour, minutes))
	CheckStartTime("test_00005",datetime(year, 5, 22, hour, minutes))
	CheckStartTime("test_00006",datetime(year, 6, 26, hour, minutes))
	CheckStartTime("test_00007",datetime(year, 7, 24, hour, minutes))
	CheckStartTime("test_00008",datetime(year, 8, 28, hour, minutes))
	CheckStartTime("test_00009",datetime(year, 9, 25, hour, minutes))
	CheckStartTime("test_00010",datetime(year, 10, 23, hour, minutes))
	CheckStartTime("test_00011",datetime(year, 11, 27, hour, minutes))
	CheckStartTime("test_00012",datetime(year, 12, 25, hour, minutes))
	CheckStartTime("test_00013",datetime(year+1, 1, 22, hour, minutes))
	CheckStartTime("test_00014",datetime(year+1, 2, 26, hour, minutes))
	CheckStartTime("test_00015",datetime(year+1, 3, 25, hour, minutes))
	CheckStartTime("test_00016",datetime(year+1, 4, 22, hour, minutes))
	CheckStartTime("test_00017",datetime(year+1, 5, 27, hour, minutes))
	CheckStartTime("test_00018",datetime(year+1, 6, 24, hour, minutes))
	CheckStartTime("test_00019",datetime(year+1, 7, 22, hour, minutes))
	CheckStartTime("test_00020",datetime(year+1, 8, 26, hour, minutes))
	CheckStartTime("test_00021",datetime(year+1, 9, 23, hour, minutes))
	CheckStartTime("test_00022",datetime(year+1, 10, 28, hour, minutes))
	CheckStartTime("test_00023",datetime(year+1, 11, 25, hour, minutes))
	CheckStartTime("test_00024",datetime(year+1, 12, 23, hour, minutes))
	c.DelRepeatedReservation(repeated_id)
	c.CheckNumOfReservationsInList(0)
	print "-----------------------------------------------------------"
	print "Check Monthly - last wednesday of every month, with 12 occurrences"  
	year = 2025
	LoadMonthlyPerDayRepeatedXML("last",4)
	SetOccurNumber(12)
	repeated_id = SetBasicParamsAndSend(res_name, profId, datetime(year, 1, 1, hour, minutes))
	c.CheckNumOfReservationsInList(12)
	CheckStartTime("test_00001",datetime(year, 1, 29, hour, minutes))
	CheckStartTime("test_00002",datetime(year, 2, 26, hour, minutes))
	CheckStartTime("test_00003",datetime(year, 3, 26, hour, minutes))
	CheckStartTime("test_00004",datetime(year, 4, 30, hour, minutes))
	CheckStartTime("test_00005",datetime(year, 5, 28, hour, minutes))
	CheckStartTime("test_00006",datetime(year, 6, 25, hour, minutes))
	CheckStartTime("test_00007",datetime(year, 7, 30, hour, minutes))
	CheckStartTime("test_00008",datetime(year, 8, 27, hour, minutes))
	CheckStartTime("test_00009",datetime(year, 9, 24, hour, minutes))
	CheckStartTime("test_00010",datetime(year, 10, 29, hour, minutes))
	CheckStartTime("test_00011",datetime(year, 11, 26, hour, minutes))
	CheckStartTime("test_00012",datetime(year, 12, 31, hour, minutes))
	c.DelRepeatedReservation(repeated_id)
	c.CheckNumOfReservationsInList(0)
	print "-----------------------------------------------------------"
	print "Check Monthly - first saturday of every 3 months, with 8 occurrences"  
	year = 2027
	LoadMonthlyPerDayRepeatedXML("first",7,3)
	SetOccurNumber(8)
	repeated_id = SetBasicParamsAndSend(res_name, profId, datetime(year, 1, 1, hour, minutes))
	c.CheckNumOfReservationsInList(8)
	CheckStartTime("test_00001",datetime(year, 1, 2, hour, minutes))
	CheckStartTime("test_00002",datetime(year, 4, 3, hour, minutes))
	CheckStartTime("test_00003",datetime(year, 7, 3, hour, minutes))
	CheckStartTime("test_00004",datetime(year, 10, 2, hour, minutes))
	CheckStartTime("test_00005",datetime(year+1, 1, 1, hour, minutes))
	CheckStartTime("test_00006",datetime(year+1, 4, 1, hour, minutes))
	CheckStartTime("test_00007",datetime(year+1, 7, 1, hour, minutes))
	CheckStartTime("test_00008",datetime(year+1, 10, 7, hour, minutes))
	c.DelRepeatedReservation(repeated_id)
	c.CheckNumOfReservationsInList(0)
	print "-----------------------------------------------------------"
	print "Check Monthly - last sunday of every 2 month, with 6 occurrences"  
	year = 2025
	LoadMonthlyPerDayRepeatedXML("last",1,2)
	SetOccurNumber(6)
	repeated_id = SetBasicParamsAndSend(res_name, profId, datetime(year, 1, 1, hour, minutes))
	c.CheckNumOfReservationsInList(6)
	CheckStartTime("test_00001",datetime(year, 1, 26, hour, minutes))
	CheckStartTime("test_00002",datetime(year, 3, 30, hour, minutes))
	CheckStartTime("test_00003",datetime(year, 5, 25, hour, minutes))
	CheckStartTime("test_00004",datetime(year, 7, 27, hour, minutes))
	CheckStartTime("test_00005",datetime(year, 9, 28, hour, minutes))
	CheckStartTime("test_00006",datetime(year, 11, 30, hour, minutes))
	c.DelRepeatedReservation(repeated_id)
	c.CheckNumOfReservationsInList(0)
	print "-----------------------------------------------------------"
	print "Check Monthly - second friday of every 6 month, with 12 occurrences"  
	year = 2020
	LoadMonthlyPerDayRepeatedXML("second",6,6)
	SetOccurNumber(12)
	repeated_id = SetBasicParamsAndSend(res_name, profId, datetime(year, 1, 1, hour, minutes))
	c.CheckNumOfReservationsInList(12)
	CheckStartTime("test_00001",datetime(year, 1, 10, hour, minutes))
	CheckStartTime("test_00002",datetime(year, 7, 10, hour, minutes))
	CheckStartTime("test_00003",datetime(year+1, 1, 8, hour, minutes))
	CheckStartTime("test_00004",datetime(year+1, 7, 9, hour, minutes))
	CheckStartTime("test_00005",datetime(year+2, 1, 14, hour, minutes))
	CheckStartTime("test_00006",datetime(year+2, 7, 8, hour, minutes))
	CheckStartTime("test_00007",datetime(year+3, 1, 13, hour, minutes))
	CheckStartTime("test_00008",datetime(year+3, 7, 14, hour, minutes))
	CheckStartTime("test_00009",datetime(year+4, 1, 12, hour, minutes))
	CheckStartTime("test_00010",datetime(year+4, 7, 12, hour, minutes))
	CheckStartTime("test_00011",datetime(year+5, 1, 10, hour, minutes))
	CheckStartTime("test_00012",datetime(year+5, 7, 11, hour, minutes))
	c.DelRepeatedReservation(repeated_id)
	c.CheckNumOfReservationsInList(0)
	
	print "-----------------------------------------------------------"
	print "-----------------------------------------------------------"
	print "           Monthly test per day - with limit of time"
	print "-----------------------------------------------------------"
	print "-----------------------------------------------------------"
	print "Check Monthly - first sunday of every month, with limit of time"  
	year = 2027
	LoadMonthlyPerDayRepeatedXML("first",1)
	SetLimitTime(datetime(year+1, 12, 31, hour, minutes))
	repeated_id = SetBasicParamsAndSend(res_name, profId, datetime(year, 1, 1, hour, minutes))
	c.CheckNumOfReservationsInList(24)
	CheckStartTime("test_00001",datetime(year, 1, 3, hour, minutes))
	CheckStartTime("test_00002",datetime(year, 2, 7, hour, minutes))
	CheckStartTime("test_00003",datetime(year, 3, 7, hour, minutes))
	CheckStartTime("test_00004",datetime(year, 4, 4, hour, minutes))
	CheckStartTime("test_00005",datetime(year, 5, 2, hour, minutes))
	CheckStartTime("test_00006",datetime(year, 6, 6, hour, minutes))
	CheckStartTime("test_00007",datetime(year, 7, 4, hour, minutes))
	CheckStartTime("test_00008",datetime(year, 8, 1, hour, minutes))
	CheckStartTime("test_00009",datetime(year, 9, 5, hour, minutes))
	CheckStartTime("test_00010",datetime(year, 10, 3, hour, minutes))
	CheckStartTime("test_00011",datetime(year, 11, 7, hour, minutes))
	CheckStartTime("test_00012",datetime(year, 12, 5, hour, minutes))
	CheckStartTime("test_00013",datetime(year+1, 1, 2, hour, minutes))
	CheckStartTime("test_00014",datetime(year+1, 2, 6, hour, minutes))
	CheckStartTime("test_00015",datetime(year+1, 3, 5, hour, minutes))
	CheckStartTime("test_00016",datetime(year+1, 4, 2, hour, minutes))
	CheckStartTime("test_00017",datetime(year+1, 5, 7, hour, minutes))
	CheckStartTime("test_00018",datetime(year+1, 6, 4, hour, minutes))
	CheckStartTime("test_00019",datetime(year+1, 7, 2, hour, minutes))
	CheckStartTime("test_00020",datetime(year+1, 8, 6, hour, minutes))
	CheckStartTime("test_00021",datetime(year+1, 9, 3, hour, minutes))
	CheckStartTime("test_00022",datetime(year+1, 10, 1, hour, minutes))
	CheckStartTime("test_00023",datetime(year+1, 11, 5, hour, minutes))
	CheckStartTime("test_00024",datetime(year+1, 12, 3, hour, minutes))
	c.DelRepeatedReservation(repeated_id)
	c.CheckNumOfReservationsInList(0)
	print "-----------------------------------------------------------"
	print "Check Monthly - second monday of every month, with limit of time"  
	year = 2027
	LoadMonthlyPerDayRepeatedXML("second",2)
	SetLimitTime(datetime(year+1, 1, 9, hour, minutes-1))
	repeated_id = SetBasicParamsAndSend(res_name, profId, datetime(year, 1, 1, hour, minutes))
	c.CheckNumOfReservationsInList(12)
	CheckStartTime("test_00001",datetime(year, 1, 11, hour, minutes))
	CheckStartTime("test_00002",datetime(year, 2, 8, hour, minutes))
	CheckStartTime("test_00003",datetime(year, 3, 8, hour, minutes))
	CheckStartTime("test_00004",datetime(year, 4, 12, hour, minutes))
	CheckStartTime("test_00005",datetime(year, 5, 10, hour, minutes))
	CheckStartTime("test_00006",datetime(year, 6, 14, hour, minutes))
	CheckStartTime("test_00007",datetime(year, 7, 12, hour, minutes))
	CheckStartTime("test_00008",datetime(year, 8, 9, hour, minutes))
	CheckStartTime("test_00009",datetime(year, 9, 13, hour, minutes))
	CheckStartTime("test_00010",datetime(year, 10, 11, hour, minutes))
	CheckStartTime("test_00011",datetime(year, 11, 8, hour, minutes))
	CheckStartTime("test_00012",datetime(year, 12, 13, hour, minutes))
	c.DelRepeatedReservation(repeated_id)
	c.CheckNumOfReservationsInList(0)
	print "-----------------------------------------------------------"
	print "Check Monthly - third thuesday of every month, with limit of time"  
	year = 2027
	LoadMonthlyPerDayRepeatedXML("third",3)
	SetLimitTime(datetime(year, 7, 20, hour, minutes-1))
	repeated_id = SetBasicParamsAndSend(res_name, profId, datetime(year, 1, 1, hour, minutes))
	c.CheckNumOfReservationsInList(6)
	CheckStartTime("test_00001",datetime(year, 1, 19, hour, minutes))
	CheckStartTime("test_00002",datetime(year, 2, 16, hour, minutes))
	CheckStartTime("test_00003",datetime(year, 3, 16, hour, minutes))
	CheckStartTime("test_00004",datetime(year, 4, 20, hour, minutes))
	CheckStartTime("test_00005",datetime(year, 5, 18, hour, minutes))
	CheckStartTime("test_00006",datetime(year, 6, 15, hour, minutes))
	c.DelRepeatedReservation(repeated_id)
	c.CheckNumOfReservationsInList(0)
	print "-----------------------------------------------------------"
	print "Check Monthly - forth saturday of every month, with limit of time"  
	year = 2027
	LoadMonthlyPerDayRepeatedXML("fourth",7)
	SetLimitTime(datetime(year+1, 12, 23, hour, minutes))
	repeated_id = SetBasicParamsAndSend(res_name, profId, datetime(year, 1, 1, hour, minutes))
	c.CheckNumOfReservationsInList(24)
	CheckStartTime("test_00001",datetime(year, 1, 23, hour, minutes))
	CheckStartTime("test_00002",datetime(year, 2, 27, hour, minutes))
	CheckStartTime("test_00003",datetime(year, 3, 27, hour, minutes))
	CheckStartTime("test_00004",datetime(year, 4, 24, hour, minutes))
	CheckStartTime("test_00005",datetime(year, 5, 22, hour, minutes))
	CheckStartTime("test_00006",datetime(year, 6, 26, hour, minutes))
	CheckStartTime("test_00007",datetime(year, 7, 24, hour, minutes))
	CheckStartTime("test_00008",datetime(year, 8, 28, hour, minutes))
	CheckStartTime("test_00009",datetime(year, 9, 25, hour, minutes))
	CheckStartTime("test_00010",datetime(year, 10, 23, hour, minutes))
	CheckStartTime("test_00011",datetime(year, 11, 27, hour, minutes))
	CheckStartTime("test_00012",datetime(year, 12, 25, hour, minutes))
	CheckStartTime("test_00013",datetime(year+1, 1, 22, hour, minutes))
	CheckStartTime("test_00014",datetime(year+1, 2, 26, hour, minutes))
	CheckStartTime("test_00015",datetime(year+1, 3, 25, hour, minutes))
	CheckStartTime("test_00016",datetime(year+1, 4, 22, hour, minutes))
	CheckStartTime("test_00017",datetime(year+1, 5, 27, hour, minutes))
	CheckStartTime("test_00018",datetime(year+1, 6, 24, hour, minutes))
	CheckStartTime("test_00019",datetime(year+1, 7, 22, hour, minutes))
	CheckStartTime("test_00020",datetime(year+1, 8, 26, hour, minutes))
	CheckStartTime("test_00021",datetime(year+1, 9, 23, hour, minutes))
	CheckStartTime("test_00022",datetime(year+1, 10, 28, hour, minutes))
	CheckStartTime("test_00023",datetime(year+1, 11, 25, hour, minutes))
	CheckStartTime("test_00024",datetime(year+1, 12, 23, hour, minutes))
	c.DelRepeatedReservation(repeated_id)
	c.CheckNumOfReservationsInList(0)
	print "-----------------------------------------------------------"
	print "Check Monthly - last wednesday of every month, with limit of time"  
	year = 2025
	LoadMonthlyPerDayRepeatedXML("last",4)
	SetLimitTime(datetime(year+1, 1, 28, hour, minutes-1))
	repeated_id = SetBasicParamsAndSend(res_name, profId, datetime(year, 1, 1, hour, minutes))
	c.CheckNumOfReservationsInList(12)
	CheckStartTime("test_00001",datetime(year, 1, 29, hour, minutes))
	CheckStartTime("test_00002",datetime(year, 2, 26, hour, minutes))
	CheckStartTime("test_00003",datetime(year, 3, 26, hour, minutes))
	CheckStartTime("test_00004",datetime(year, 4, 30, hour, minutes))
	CheckStartTime("test_00005",datetime(year, 5, 28, hour, minutes))
	CheckStartTime("test_00006",datetime(year, 6, 25, hour, minutes))
	CheckStartTime("test_00007",datetime(year, 7, 30, hour, minutes))
	CheckStartTime("test_00008",datetime(year, 8, 27, hour, minutes))
	CheckStartTime("test_00009",datetime(year, 9, 24, hour, minutes))
	CheckStartTime("test_00010",datetime(year, 10, 29, hour, minutes))
	CheckStartTime("test_00011",datetime(year, 11, 26, hour, minutes))
	CheckStartTime("test_00012",datetime(year, 12, 31, hour, minutes))
	c.DelRepeatedReservation(repeated_id)
	c.CheckNumOfReservationsInList(0)
	print "-----------------------------------------------------------"
	print "Check Monthly - first saturday of every 3 months, with limit of time"  
	year = 2027
	LoadMonthlyPerDayRepeatedXML("first",7,3)
	SetLimitTime(datetime(year+1, 10, 7, hour, minutes))
	repeated_id = SetBasicParamsAndSend(res_name, profId, datetime(year, 1, 1, hour, minutes))
	c.CheckNumOfReservationsInList(8)
	CheckStartTime("test_00001",datetime(year, 1, 2, hour, minutes))
	CheckStartTime("test_00002",datetime(year, 4, 3, hour, minutes))
	CheckStartTime("test_00003",datetime(year, 7, 3, hour, minutes))
	CheckStartTime("test_00004",datetime(year, 10, 2, hour, minutes))
	CheckStartTime("test_00005",datetime(year+1, 1, 1, hour, minutes))
	CheckStartTime("test_00006",datetime(year+1, 4, 1, hour, minutes))
	CheckStartTime("test_00007",datetime(year+1, 7, 1, hour, minutes))
	CheckStartTime("test_00008",datetime(year+1, 10, 7, hour, minutes))
	c.DelRepeatedReservation(repeated_id)
	c.CheckNumOfReservationsInList(0)
	print "-----------------------------------------------------------"
	print "Check Monthly - last sunday of every 2 month, with limit of time"  
	year = 2025
	LoadMonthlyPerDayRepeatedXML("last",1,2)
	SetLimitTime(datetime(year, 11, 30, hour, minutes))
	repeated_id = SetBasicParamsAndSend(res_name, profId, datetime(year, 1, 1, hour, minutes))
	c.CheckNumOfReservationsInList(6)
	CheckStartTime("test_00001",datetime(year, 1, 26, hour, minutes))
	CheckStartTime("test_00002",datetime(year, 3, 30, hour, minutes))
	CheckStartTime("test_00003",datetime(year, 5, 25, hour, minutes))
	CheckStartTime("test_00004",datetime(year, 7, 27, hour, minutes))
	CheckStartTime("test_00005",datetime(year, 9, 28, hour, minutes))
	CheckStartTime("test_00006",datetime(year, 11, 30, hour, minutes))
	c.DelRepeatedReservation(repeated_id)
	c.CheckNumOfReservationsInList(0)
	print "-----------------------------------------------------------"
	print "Check Monthly - second friday of every 6 month, with limit of time"  
	year = 2020
	LoadMonthlyPerDayRepeatedXML("second",6,6)
	SetLimitTime(datetime(year+6, 1, 8, hour, minutes))
	repeated_id = SetBasicParamsAndSend(res_name, profId, datetime(year, 1, 1, hour, minutes))
	c.CheckNumOfReservationsInList(12)
	CheckStartTime("test_00001",datetime(year, 1, 10, hour, minutes))
	CheckStartTime("test_00002",datetime(year, 7, 10, hour, minutes))
	CheckStartTime("test_00003",datetime(year+1, 1, 8, hour, minutes))
	CheckStartTime("test_00004",datetime(year+1, 7, 9, hour, minutes))
	CheckStartTime("test_00005",datetime(year+2, 1, 14, hour, minutes))
	CheckStartTime("test_00006",datetime(year+2, 7, 8, hour, minutes))
	CheckStartTime("test_00007",datetime(year+3, 1, 13, hour, minutes))
	CheckStartTime("test_00008",datetime(year+3, 7, 14, hour, minutes))
	CheckStartTime("test_00009",datetime(year+4, 1, 12, hour, minutes))
	CheckStartTime("test_00010",datetime(year+4, 7, 12, hour, minutes))
	CheckStartTime("test_00011",datetime(year+5, 1, 10, hour, minutes))
	CheckStartTime("test_00012",datetime(year+5, 7, 11, hour, minutes))
	c.DelRepeatedReservation(repeated_id)
	c.CheckNumOfReservationsInList(0)

#------------------------------------------------------------------------------

#------------------------------------------------------------------------------
#---------------------TEST ITSELF----------------------------------------------
#------------------------------------------------------------------------------

CheckEverything(0,5)
CheckEverything(10,1)
CheckEverything(21,30)
CheckEverything(23,55)
print "-----------------------------------------------------------"
c.DelProfile(profId)
c.Disconnect()

