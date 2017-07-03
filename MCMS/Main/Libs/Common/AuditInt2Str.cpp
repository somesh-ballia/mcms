#include "AuditInt2Str.h"
#include "AuditDefines.h"


const char* AuditEventTypeToStr(eAuditEventType type)
{
    static const char *names [] =
        {
            "API",
            "HTTP",
            "Internal"
        };
    const char *name = ((unsigned)type < sizeof(names) / sizeof(names[0])
                        ?
                        names[type] : "none");
    return name;
}



const char* AuditEventStatusToStr(eAuditEventStatus status)
{
    static const char *names [] =
        {
            "Yes",
            "No"
        };
    const char *name = ((unsigned)status < sizeof(names) / sizeof(names[0])
                        ?
                        names[status] : "none");
    return name;
}

