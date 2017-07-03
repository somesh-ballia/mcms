#!/mcms/python/bin/python


from McmsConnection import *


class User:
    m_Name = ""
    m_Password = ""
    m_Permissions = ""
    

    def __init__(self, name, password, permissions):
        self.m_Name = name
        self.m_Password = password
        self.m_Permissions = permissions




class UsersUtilities ( McmsConnection ):
    
#------------------------------------------------------------------------------
    def AddNewUserNew(self,user,expected_status="Status OK"):
        self.AddNewUser(user.m_Name, user.m_Password, user.m_Permissions, expected_status)
        return(0)
#------------------------------------------------------------------------------    
#------------------------------------------------------------------------------
    def AddNewUser(self,user_name,user_password,permission,expected_status="Status OK"):
        print "Send Add user to : "+ user_name + " user_password: "  + user_password + " permission: " + permission + " to users list"
        self.LoadXmlFile('Scripts/AddNewOperator.xml') 
        self.ModifyXml("NEW_OPERATOR","USER_NAME",user_name)
        self.ModifyXml("NEW_OPERATOR","PASSWORD",user_password)
        self.ModifyXml("NEW_OPERATOR","AUTHORIZATION_GROUP",permission)
        self.Send(expected_status)
        return(0)
#------------------------------------------------------------------------------
#------------------------------------------------------------------------------
    def DelUser(self,user_name,expected_status="Status OK"):
        print "Send Del user to : "+ user_name + " from the users list"
        self.LoadXmlFile('Scripts/RemoveNewOperator.xml') 
        self.ModifyXml("DELETE_OPERATOR","USER_NAME",user_name)
        self.Send(expected_status)
        return(0)
#------------------------------------------------------------------------------
   
    def CheckIfUserExistInTheUsersList(self,user_name,user_password,permission):
        foundUser =0
        self.SendXmlFile("Scripts/GetUserListReq.xml")
        
        #fault_elm_list = self.xmlResponse.getElementsByTagName("OPER_LIST")
        #print self.xmlResponse.toprettyxml()
        fault_elm_list = self.xmlResponse.getElementsByTagName("OPERATOR")
        num_of_faults = len(fault_elm_list)
        print "GetUsersList ..."      
        for index in range(len(fault_elm_list)):
            user_from_list = fault_elm_list[index].getElementsByTagName("USER_NAME")[0].firstChild.data
            print user_from_list
            if user_name == user_from_list:
               permission_from_list=fault_elm_list[index].getElementsByTagName("AUTHORIZATION_GROUP")[0].firstChild.data
               print permission_from_list
               print user_name+"= " + user_name+" " + permission+" found in users list"
               if permission_from_list == permission:
                  foundUser = foundUser+1
        return (foundUser)
#------------------------------------------------------------------------------
   
   # def CheckIfUserExistInTheUsersList(self,user_name,user_password,permission):
   #     foundUser =0
   #     self.SendXmlFile("Scripts/GetUserListReq.xml")
        
   #     fault_elm_list = self.xmlResponse.getElementsByTagName("CONNECTIONS_LIST")
        
   #     fault_elm_list = self.xmlResponse.getElementsByTagName("OPERATOR")
   #     num_of_faults = len(fault_elm_list)
   #     print "GetConnectionList ..."      
   #     for index in range(len(fault_elm_list)):
   #         user_from_list = fault_elm_list[index].getElementsByTagName("USER_NAME")[0].firstChild.data
   #         print user_from_list
   #         if user_name == user_from_list:
   #            foundUser = foundUser+1
   #            print user_name+"= " + user_name + " found in users list"
       
   #     return (foundUser)

 #------------------------------------------------------------------------------
 #------------------------------------------------------------------------------
   
    def CheckIfUserExistInTheConnectionList(self,user_name,user_password,permission):
        foundUser =0
        self.SendXmlFile("Scripts/GetConnectionListReq.xml")
        #fault_elm_list = self.xmlResponse.getElementsByTagName("CONNECTIONS_LIST")
        
        fault_elm_list = self.xmlResponse.getElementsByTagName("OPERATOR_CONNECTION")
        num_of_faults = len(fault_elm_list)
        print "GetConnectionList ..."      
        for index in range(len(fault_elm_list)):
            user_from_list = fault_elm_list[index].getElementsByTagName("USER_NAME")[0].firstChild.data
            print len(fault_elm_list)
            print user_from_list
            if user_name == user_from_list:
              # permission_from_list=fault_elm_list[index].getElementsByTagName("AUTHORIZATION_GROUP")[0].firstChild.data
              # print permission_from_list
                print user_name+"= " + user_name+" " + permission+" found in users list"
              # if permission_from_list == permission:
                foundUser = foundUser+1
        return (foundUser)
        
#------------------------------------------------------------------------------   
 #       def CheckIfUserExistInTheConnectionList(self,user_name,user_password,permission):
 #       foundUser =0
 #       self.SendXmlFile("Scripts/GetConnectionListReq.xml")
        
        #fault_elm_list = self.xmlResponse.getElementsByTagName("GET_OPER_LIST")
        #RESPONSE_TRANS_OPER_LIST
 #       fault_elm_list = self.xmlResponse.getElementsByTagName("OPERATOR")
 #       num_of_faults = len(fault_elm_list)
 #       print "GetUsersList ..."      
 #       for index in range(len(fault_elm_list)):
 #           user_from_list = fault_elm_list[index].getElementsByTagName("USER_NAME")[0].firstChild.data
 #           print user_from_list
 #           if user_name == user_from_list:
 #              foundUser = foundUser+1
 #              print user_name+"= " + user_name + " found in users list"
 #       return (foundUser)          
 #------------------------------------------------------------------------------
    def UpdatePwd(self,user_name,old_password,new_password,expected_status="Status OK"):
        print "Send update user password to : "+ user_name + " old password: "  + old_password + " new password: " + new_password + " to users list"
        self.LoadXmlFile('Scripts/UpdateUser.xml') 
        self.ModifyXml("CHANGE_PASSWORD","USER_NAME",user_name)
        self.ModifyXml("CHANGE_PASSWORD","OLD_PASSWORD",old_password)
        self.ModifyXml("CHANGE_PASSWORD","NEW_PASSWORD",new_password)
        self.Send(expected_status)
        return(0)
    
  #------------------------------------------------------------------------------
    def UpdateUserFirstFailureTimeInOperatorDB(self, name, days, hours, minutes):
        os.system("Scripts/Destroy.sh")
        
        t = datetime.utcnow( ) 
        deltat = timedelta(days, 0, 0, 0, minutes, hours, 0)
        t = t - deltat
        newTime = t.strftime("%Y-%m-%dT%I:%M:%S") 
        
        loadedXml = parse("Cfg/EncOperatorDB.xml")
        elm_list = loadedXml.getElementsByTagName("OPERATOR")   
        list_size = len(elm_list)
        for index in range(list_size):
            if name == elm_list[index].getElementsByTagName("USER_NAME")[0].firstChild.data:
                elm_list[index].getElementsByTagName("FIRST_LOGIN_PWD_FAILURE")[0].firstChild.data = newTime    
                break
                          
        xml = loadedXml.toxml()[22:]
        
        out = open("Cfg/EncOperatorDB.xml", "w")
        out.write(xml)
        out.close()
        
        os.system("Scripts/StartupWithFederal.sh")  
        
#------------------------------------------------------------------------------
    def UpdateUserLastPwdChangedInOperatorDB(self, name, days, hours, minutes):
        os.system("Scripts/Destroy.sh")
        
        t = datetime.utcnow( ) 
        deltat = timedelta(days, 0, 0, 0, minutes, hours, 0)
        t = t - deltat
        newTime = t.strftime("%Y-%m-%dT%I:%M:%S") 
        
        loadedXml = parse("Cfg/EncOperatorDB.xml")
        elm_list = loadedXml.getElementsByTagName("OPERATOR")   
        list_size = len(elm_list)
        for index in range(list_size):
            if name == elm_list[index].getElementsByTagName("USER_NAME")[0].firstChild.data:
                elm_list[index].getElementsByTagName("LAST_PWD_CHANGE")[0].firstChild.data = newTime
                break
                            
        xml = loadedXml.toxml()[22:]
        
        out = open("Cfg/EncOperatorDB.xml", "w")
        out.write(xml)
        out.close()
        
        os.system("Scripts/StartupWithFederal.sh")
    
#------------------------------------------------------------------------------
    def UpdateUserLastLoginTimeInOperatorDB(self, name, days, hours, minutes):
        os.system("Scripts/Destroy.sh")
        
        t = datetime.utcnow( ) 
        deltat = timedelta(days, 0, 0, 0, minutes, hours, 0)
        t = t - deltat
        newTime = t.strftime("%Y-%m-%dT%I:%M:%S") 
        
        loadedXml = parse("Cfg/EncOperatorDB.xml")
        elm_list = loadedXml.getElementsByTagName("OPERATOR")   
        list_size = len(elm_list)
        for index in range(list_size):
            if name == elm_list[index].getElementsByTagName("USER_NAME")[0].firstChild.data:
                elm_list[index].getElementsByTagName("LAST_LOGIN")[0].getElementsByTagName("DATE")[0].firstChild.data = newTime
                break
                       
        xml = loadedXml.toxml()[22:]
        
        out = open("Cfg/EncOperatorDB.xml", "w")
        out.write(xml)
        out.close()
        
        os.system("Scripts/StartupWithFederal.sh")
        

#------------------------------------------------------------------------------
    def UpdateAccountDisabledTimeInOperatorDB(self, name, days, hours, minutes):
        os.system("Scripts/Destroy.sh")
        
        t = datetime.utcnow( ) 
        deltat = timedelta(days, 0, 0, 0, minutes, hours, 0)
        t = t - deltat
        newTime = t.strftime("%Y-%m-%dT%I:%M:%S") 
        
        loadedXml = parse("Cfg/EncOperatorDB.xml")
        elm_list = loadedXml.getElementsByTagName("OPERATOR")   
        list_size = len(elm_list)
        for index in range(list_size):
            if name == elm_list[index].getElementsByTagName("USER_NAME")[0].firstChild.data:
                elm_list[index].getElementsByTagName("DISABLED_AT")[0].firstChild.data = newTime
                break
                       
        xml = loadedXml.toxml()[22:]
        
        out = open("Cfg/EncOperatorDB.xml", "w")
        out.write(xml)
        out.close()
        
        os.system("Scripts/StartupWithFederal.sh")
        
#------------------------------------------------------------------------------
    def UpdateAccountLockedTimeInOperatorDB(self, name, days, hours, minutes):
        os.system("Scripts/Destroy.sh")
        
        t = datetime.utcnow( ) 
        deltat = timedelta(days, 0, 0, 0, minutes, hours, 0)
        t = t - deltat
        newTime = t.strftime("%Y-%m-%dT%I:%M:%S") 
        
        loadedXml = parse("Cfg/EncOperatorDB.xml")
        elm_list = loadedXml.getElementsByTagName("OPERATOR")   
        list_size = len(elm_list)
        for index in range(list_size):
            if name == elm_list[index].getElementsByTagName("USER_NAME")[0].firstChild.data:
                elm_list[index].getElementsByTagName("LOCKED_AT")[0].firstChild.data = newTime
                break
                       
        xml = loadedXml.toxml()[22:]
        
        out = open("Cfg/EncOperatorDB.xml", "w")
        out.write(xml)
        out.close()
        
        os.system("Scripts/StartupWithFederal.sh")
#------------------------------------------------------------------------------
  
#------------------------------------------------------------------------------           


