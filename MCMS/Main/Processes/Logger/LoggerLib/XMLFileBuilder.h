#pragma once

#include <string>
#include <vector>
#include "PObject.h"
using namespace std;

struct udp_appender
{
	string  name;
	string  port;
	string  ip;
	string  filter_str;
};

class XMLFileBuilder : public CPObject
{
	CLASS_TYPE_1(XMLFileBuilder, CPObject)
		
public:
        XMLFileBuilder(void);
        ~XMLFileBuilder(void);
	virtual const char*    NameOf() const               { return "XMLFileBuilder";}
        void 										Init(const string& udpLoggerName, const string& sysLogLoggerName);
        void                                    	AddUDPAppender(string name,string ip, string port,string filter);
        void                                    	AddSysLogAppender(string name,string ip, string port,string filter);
        void                                    	DelUDPAppender(string name);
        void                                    	GenerateFile(string full_path_name);
        void										GenerateSysLogFile(string full_path_name);
        void										GenerateRemoteViewerFile(string full_path_name);
        void                                    	LoadFromFile(string path);
        string 										m_udpLoggerName;
        string 										m_sysLogLoggerName;
		vector<udp_appender>&						GetUDPAppendersVec();
		bool										GetSwitchState() const;
		void										SetSwitchState(bool bSwitch);
		void										ClearAllUDPAppenders();
		void 										ClearAllSysLogAppenders();
private:
        vector<udp_appender>            				m_vecUdpAppenders;
        vector<udp_appender>                    		m_vecSysLogAppenders;
		bool											m_bSwitch;
        void                                            ReadPort(string line,string& port);
        void                                            ReadIPAddr(string line,string& ip);
        void                                            ReadFilterString(string line,string& filter_str);
        void                                            ReadAppenderName(string line,string& name);
        string                                          GetValue(string line,string token);
        string                                          GetRollingFileAppender() const;
        string                                          GetAsyncHeader() const;
        string                                          GetUDPAppenders() const;
        string                                          GetSyslogAppenders() const;
        string                                          GetRootHeader() const;
		bool 											ValidateIPAndPort(const string& ip, const string& port);
        string 											GetCategoriesHeader(vector<udp_appender>  & appender, string categoryName) const;

};
