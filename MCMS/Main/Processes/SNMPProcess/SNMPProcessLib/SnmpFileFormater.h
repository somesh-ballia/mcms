#if !defined(_SNMP_FILE_FORMATER_)
#define _SNMP_FILE_FORMATER_

#include <string>
using namespace std;

#include "PObject.h"
#include "NStream.h"


class CSnmpFileFormater : CPObject
{
CLASS_TYPE_1(CScmlFileFormater, CPObject)
public:
    virtual void Dump(std::ostream&) const{};

    
    static bool UpdateNsmpMibFile(const string & fileNameSource, const string & fileNameDest, std::ostream & answer);
    
    
private:
    // disabled
    CSnmpFileFormater();
    CSnmpFileFormater(const CSnmpFileFormater&);
    CSnmpFileFormater&operator=(const CSnmpFileFormater&);



    static bool ReadFromMibFile(const string & fileNameSource, char *& buffer, std::ostream& answer);
    static bool WriteToMibFile(const string & fileNameDest, COstrStream & ostr, std::ostream & answer);
    static void DumpAlertMap(std::ostream & ostr);
    static void RemoveEndPattern(char *buffer, const char *strEndPattern);
    static void CreateMIBAlertName(char * strErrorCodeBuffer);
    static void DumpSigleAlert(const char *mibName,
                               const char *mibNameExtension,
                               WORD errorCode,
                               const char *strAlarmDescription,
                               std::ostream & ostr);
};





#endif // !defined(_SNMP_FILE_FORMATER_)

