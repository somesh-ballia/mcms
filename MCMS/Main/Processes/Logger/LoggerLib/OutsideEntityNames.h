#ifndef OUTSIDEENTITYNAMES_H_
#define OUTSIDEENTITYNAMES_H_

#include "Macros.h"



static const char *EmaProcessNames[] = 
{
	"EmaProcess",  // eTheOneTheOnlyEmaProcess	
};
static const char *GetEmaProcessName(eEmaProcesses type)
{
	const char *name = (eTheOneTheOnlyEmaProcess <= type && type < NumOfEmaProcesses
						?
						EmaProcessNames[type] : "InvalideProcess");
	return name;
}





static const char *CSProcessNames[] = 
{
	"NA",  // eTheOneTheOnlyCSModuleProcess
};
static const char *GetCSProcessName(eCSProcesses type)
{
	const char *name = (eTheOneTheOnlyCSProcess <= type && type < NumOfCSProcesses
						?
						CSProcessNames[type] : "InvalideProcess");
	return name;
}





static const char *MplProcessNames[] = 
{
	"NA"						,  // eTheOneTheOnlyMplProcess	
	"MfaCardManager"			,
	"SwitchCardManager"			,
	"VideoDsp"					,
	"ArtDsp"					,
	"EmbeddedApacheModule"		,
	"IceManagerProcess"			,
	"MfaLauncher"               ,
	"AMP"						,
	"VMP"						,
	"MPProxy"
};
static const char *GetMplProcessName(eMplProcesses type)
{
	const char *name = (type >= 0 && (unsigned int)type < ARRAYSIZE(MplProcessNames)
						?
						MplProcessNames[type] : "InvalideProcess");
	return name;
}


#endif /*OUTSIDEENTITYNAMES_H_*/
