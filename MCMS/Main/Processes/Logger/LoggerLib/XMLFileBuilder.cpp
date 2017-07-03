#include "XMLFileBuilder.h"
#include <iostream>
#include <fstream>
#include "ObjString.h"
#include "Trace.h"

using namespace std;

XMLFileBuilder::XMLFileBuilder(void)
{
	m_bSwitch = false;
}

void XMLFileBuilder::Init(const string& udpLoggerName, const string& sysLogLoggerName)
{
	m_udpLoggerName = udpLoggerName;
	m_sysLogLoggerName = sysLogLoggerName;
}

XMLFileBuilder::~XMLFileBuilder(void)
{
}


vector<udp_appender>&	XMLFileBuilder::GetUDPAppendersVec()
{
	return m_vecUdpAppenders;
}

bool	XMLFileBuilder::GetSwitchState() const
{
	return m_bSwitch;
}

void    XMLFileBuilder::SetSwitchState(bool bSwitch)
{
	m_bSwitch = bSwitch;
}

void    XMLFileBuilder::AddUDPAppender(string name,string ip, string port,string filter)
{
	if (ValidateIPAndPort(ip,port) == false)
	{
		PASSERTMSG(1,"XMLFileBuilder::AddUDPAppender - invalid ip address");
		return;
	}
	
	udp_appender tmp_appender;
	tmp_appender.name = name;
	tmp_appender.ip = ip;
	tmp_appender.port = port;
	tmp_appender.filter_str = filter;
	m_vecUdpAppenders.push_back(tmp_appender);
}

void    XMLFileBuilder::AddSysLogAppender(string name,string ip, string port,string filter)
{
	if (ValidateIPAndPort(ip,port) == false)
	{
		PASSERTMSG(1,"XMLFileBuilder::AddSysLogAppender - invalid ip address");
		return;
	}
	
	udp_appender tmp_appender;
	tmp_appender.name = name;
	tmp_appender.ip = ip;
	tmp_appender.port = port;
	tmp_appender.filter_str = filter;
	m_vecSysLogAppenders.push_back(tmp_appender);
}

void	XMLFileBuilder::ClearAllUDPAppenders()
{	
	m_vecUdpAppenders.clear();
}

void	XMLFileBuilder::ClearAllSysLogAppenders()
{
	m_vecSysLogAppenders.clear();
}

void    XMLFileBuilder::DelUDPAppender(string name)
{
}

void    XMLFileBuilder::GenerateSysLogFile(string full_path_name)
{
		/*string syslog_appender = GetSyslogAppenders();
		vector<udp_appender> * appenderVector;
		appenderVector = &m_vecSysLogAppenders;
		string async_header = GetAsyncHeader(appenderVector);
		GenerateFile(full_path_name, syslog_appender,async_header);*/
}

void    XMLFileBuilder::GenerateRemoteViewerFile(string full_path_name)
{
		/*string udpAppender = GetUDPAppenders();
		vector<udp_appender> * appenderVector;
		appenderVector = &m_vecUdpAppenders;
		string async_header = GetAsyncHeader(appenderVector);
		GenerateFile(full_path_name, udpAppender,async_header);*/
}

void    XMLFileBuilder::GenerateFile(string full_path_name)
{
        ofstream output (full_path_name.c_str());

        const string enter_str = "\n";
        const string xml_header1 = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>";
        const string xml_header2 = "<!DOCTYPE log4j:configuration SYSTEM \"log4j.dtd\">";
        const string xml_header3 = "<log4j:configuration xmlns:log4j=\"http://jakarta.apache.org/log4j/\" debug=\"false\">";
        const string ending = "</log4j:configuration>";

        //string async_header = GetAsyncHeader();
        string rolling_file_appender = GetRollingFileAppender();
        string udp_appenders = GetUDPAppenders();
        string sys_appenders = GetSyslogAppenders();
        string udp_category = GetCategoriesHeader(m_vecUdpAppenders, m_udpLoggerName);
        string syslog_category = GetCategoriesHeader(m_vecSysLogAppenders, m_sysLogLoggerName);

        output.write(xml_header1.c_str(),xml_header1.length());
        output.write(enter_str.c_str(),enter_str.length());

        output.write(xml_header2.c_str(),xml_header2.length());
        output.write(enter_str.c_str(),enter_str.length());

        output.write(xml_header3.c_str(),xml_header3.length());
        output.write(enter_str.c_str(),enter_str.length());

        // output.write(async_header.c_str(),async_header.length());
        output.write(udp_appenders.c_str(),udp_appenders.length());
        output.write(sys_appenders.c_str(),sys_appenders.length());

        output.write(syslog_category.c_str(),syslog_category.length());
        output.write(udp_category.c_str(),udp_category.length());


        output.write(rolling_file_appender.c_str(),rolling_file_appender.length());

        //string root = GetRootHeader();
        // output.write(root.c_str(),root.length());

        output.write(enter_str.c_str(),enter_str.length());
        output.write(ending.c_str(),ending.length());
        output.close();
}

void    XMLFileBuilder::LoadFromFile(string path)
{
        ifstream fs_stream;
        fs_stream.open(path.c_str());
        //if (fs_streams
        string line;
        do
        {
                getline(fs_stream,line,'>');

                if ((line.size() <= 0) || (line.find("</log4j:configuration") != string::npos))
                        break;


                if ((line.find("<appender") !=string::npos ) && (line.find("UDPAppender") != string::npos) )
                {
                        udp_appender    _appender;
                        ReadAppenderName(line,_appender.name);
                        do
                        {
                                getline(fs_stream,line,'>');

                                if ((line.size() <= 0) || (line.find("/appender") !=string::npos))
                                        break;

                                ReadPort(line,_appender.port);
                                ReadIPAddr(line,_appender.ip);
                                ReadFilterString(line,_appender.filter_str);

                        }while(true);
                        //if (_appender.ip.length() > 0 && _appender.
                        m_vecUdpAppenders.push_back(_appender);
                }


        }while (true);
}

void    XMLFileBuilder::ReadPort(string line,string& port)
{


        if ((line.find("Port") != string::npos) ||
                (line.find("port") != string::npos) ||
                (line.find("PORT") != string::npos)
                )
        {

                // = line.find
                string token = "value=\"";
                port = GetValue(line,token);

                //printf("%s \n",port.c_str());

        }

}

void    XMLFileBuilder::ReadIPAddr(string line,string& ip)
{
        if ((line.find("RemoteHost") != string::npos) ||
                (line.find("remoteHost") != string::npos) ||
                (line.find("REMOTEHOST") != string::npos)
                )
        {
                string token = "value=\"";
                ip = GetValue(line,token);
                //printf("%s \n",ip.c_str());

        }
}

void    XMLFileBuilder::ReadFilterString(string line,string& filter_str)
{

        if ((line.find("STRINGTOMATCH") != string::npos))
        {
                string token = "value=\"";
                filter_str = GetValue(line,token);
                //printf("%s \n",filter_str.c_str());
        }


}

void    XMLFileBuilder::ReadAppenderName(string line,string& name)
{
        if ((line.find("<appender") !=string::npos ) && (line.find("UDPAppender")!=string::npos) )
        {
                string token = "name=\"";
                name = GetValue(line,token);
                //printf("%s \n",name.c_str());
        }
}

string  XMLFileBuilder::GetValue(string line,string token)
{
        string value;
        size_t nPosStart;
        size_t nPosEnd;
        //string token = "value=\"";
        nPosStart = line.find(token);
        nPosEnd = line.find("\"",nPosStart+token.length());
        value = line.substr(token.length() + nPosStart,nPosEnd-(token.length() + nPosStart));

        return value;
}

string XMLFileBuilder::GetRollingFileAppender() const
{
        string output;

        return output;
}

string XMLFileBuilder::GetAsyncHeader() const
{
        string output;
        const string enter_str = "\n";
        const string async_header = "<appender name=\"ASYNC\" class=\"org.apache.log4j.AsyncAppender\">";
        const string ending = "</appender>";
        //const string rolling_name = "<appender-ref ref=\"ROLLER\"/>";
        string element_name;
        for (unsigned int i =0 ; i < m_vecUdpAppenders.size(); i++)
        {
                element_name += "<appender-ref ref=\"";
                element_name += m_vecUdpAppenders[i].name + "\"/>" + enter_str;
        }
        for (unsigned int i =0 ; i < m_vecSysLogAppenders.size(); i++)
	    {
			   element_name += "<appender-ref ref=\"";
			   element_name += m_vecSysLogAppenders[i].name + "\"/>" + enter_str;
	    }

        output = async_header + enter_str + element_name + ending + enter_str;

        return output;
}

string XMLFileBuilder::GetCategoriesHeader(vector<udp_appender>  & appender, string categoryName) const
{
        string output;
        const string enter_str = "\n";
        const string category_header = "<category name=\""+categoryName+"\">";
        const string ending = "</category>";
        //const string rolling_name = "<appender-ref ref=\"ROLLER\"/>";
        string element_name;
        for (unsigned int i =0 ; i < (appender).size(); i++)
        {
                element_name += "<appender-ref ref=\"";
                element_name += (appender)[i].name + "\"/>" + enter_str;
        }


        output = category_header + enter_str + element_name + ending + enter_str;

        return output;
}

string XMLFileBuilder::GetRootHeader() const
{
        string output;
        output = "<root>\n";
        output += "<priority value=\"all\"/>\n";
        output += "<appender-ref ref=\"ASYNC\"/>\n";
        output += "</root>\n";

        return output;

}

string XMLFileBuilder::GetSyslogAppenders() const
{
        string output;
        const string enter_str = "\n";
        const string appender_name = "<appender name=\"";
        const string class_name ="\" class=\"org.apache.log4j.SyslogAppender\">";
        const string port = "<param name=\"Port\" value=\"";
        const string ip = "<param name=\"SysLogHost\" value=\"";
        const string buffer = "<param name=\"BufferSize\" value=\"1000\"/>";
        const string patternLayout = "<layout class=\"org.apache.log4j.PatternLayout\">";
        const string pattern_format = "<param name=\"ConversionPattern\" value=\"%date  - %message%newline\"/>";
        const string ending_layout = "</layout>";
        const string ending = "</appender>";
        string pattern = patternLayout + enter_str + pattern_format + enter_str + ending_layout;

        const string filter_name = "<filter  class=\"org.apache.log4j.varia.StringMatchFilter\">";
        const string string_match = "<param name=\"STRINGTOMATCH\" value=\"";
        const string accept_match = "<param name=\"ACCEPTONMATCH\" value=\"false\"/>";
		const string end_filter = "</filter>";

        for (unsigned int i =0 ; i < m_vecSysLogAppenders.size(); i++)
        {
                string _appender_name = appender_name + m_vecSysLogAppenders[i].name + class_name;
                string _port = port + m_vecSysLogAppenders[i].port + "\"/>";
                string _ip = ip + m_vecSysLogAppenders[i].ip + "\"/>";

                output += _appender_name + enter_str;
                output += _port + enter_str;
                output += _ip + enter_str;
                output += buffer + enter_str;
                output += pattern + enter_str;
                if (m_vecSysLogAppenders[i].filter_str.length() > 0)
                {
                        string _string_match = string_match + m_vecSysLogAppenders[i].filter_str + "\"/>";
                        string _filter_part = filter_name + enter_str;
                        _filter_part += _string_match + enter_str;
                        _filter_part += accept_match + enter_str;
                        _filter_part += end_filter + enter_str;;
                        output += _filter_part;
                }
                output += ending + enter_str;

        }

        return output;
}

string XMLFileBuilder::GetUDPAppenders() const
{
        string output;
        const string enter_str = "\n";
        const string appender_name = "<appender name=\"";
        const string class_name ="\" class=\"org.apache.log4j.UDPAppender\">";
        const string port = "<param name=\"Port\" value=\"";
        const string ip = "<param name=\"RemoteHost\" value=\"";
        const string buffer = "<param name=\"BufferSize\" value=\"1000\"/>";
        const string patternLayout = "<layout class=\"org.apache.log4j.PatternLayout\">";
        const string pattern_format = "<param name=\"ConversionPattern\" value=\"%date  - %message%newline\"/>";
        const string ending_layout = "</layout>";
        const string ending = "</appender>";
        string pattern = patternLayout + enter_str + pattern_format + enter_str + ending_layout;

        const string filter_name = "<filter  class=\"org.apache.log4j.varia.StringMatchFilter\">";
        const string string_match = "<param name=\"STRINGTOMATCH\" value=\"";
        const string accept_match = "<param name=\"ACCEPTONMATCH\" value=\"false\"/>";
		const string end_filter = "</filter>";

        for (unsigned int i =0 ; i < m_vecUdpAppenders.size(); i++)
        {
                string _appender_name = appender_name + m_vecUdpAppenders[i].name + class_name;
                string _port = port + m_vecUdpAppenders[i].port + "\"/>";
                string _ip = ip + m_vecUdpAppenders[i].ip + "\"/>";

                output += _appender_name + enter_str;
                output += _port + enter_str;
                output += _ip + enter_str;
                output += buffer + enter_str;
                output += pattern + enter_str;
                if (m_vecUdpAppenders[i].filter_str.length() > 0)
                {
                        string _string_match = string_match + m_vecUdpAppenders[i].filter_str + "\"/>";
                        string _filter_part = filter_name + enter_str;
                        _filter_part += _string_match + enter_str;
                        _filter_part += accept_match + enter_str;
                        _filter_part += end_filter + enter_str;;
                        output += _filter_part;
                }
                output += ending + enter_str;

        }

        return output;
}

bool XMLFileBuilder::ValidateIPAndPort(const string& ip, const string& port)
{
	const unsigned int MAX_IPV4_LEN = 15;
	if (ip.size()>MAX_IPV4_LEN)
		return false;

	char cpy[MAX_IPV4_LEN+1];
	strncpy(cpy, ip.c_str(),MAX_IPV4_LEN);
	cpy[MAX_IPV4_LEN] = '\0';
	
	char * pch = strtok (cpy,".");
	while (pch != NULL)
	{
		if (TRUE == CObjString::IsNumeric(pch))
		{
			if (atoi(pch) < 1 || atoi(pch) > 255)
				return FALSE;
		}
		pch = strtok (NULL,".");
	}

	DWORD iport = atoi(port.c_str());
	if (iport < 1 || iport > 4095)
			return false;

	return true;

}

