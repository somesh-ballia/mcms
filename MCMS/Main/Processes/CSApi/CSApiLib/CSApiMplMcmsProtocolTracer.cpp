#include "CSApiMplMcmsProtocolTracer.h"
#include "ObjString.h"




CCSApiMplMcmsProtocolTracer::CCSApiMplMcmsProtocolTracer()
{
	
}

CCSApiMplMcmsProtocolTracer::CCSApiMplMcmsProtocolTracer(CMplMcmsProtocol &mplMcmsProt)
:CMplMcmsProtocolTracer(mplMcmsProt)
{
}

CCSApiMplMcmsProtocolTracer::~CCSApiMplMcmsProtocolTracer()
{
}
/* //BRIDGE-18254
void CCSApiMplMcmsProtocolTracer::TraceContent(CObjString* pContentStr, eProcessType processType)
{
	*pContentStr << "\nCONTENT: CSApi does not print content";
}
*/
