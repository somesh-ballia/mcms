// Authentication.h

#ifndef AUTHETICATION_H
#define AUTHETICATION_H

#include "PObject.h"
#include "CardsStructs.h"
#include "McuMngrStructs.h"
#include "ProductType.h"

class CAuthentication : public CPObject
{
CLASS_TYPE_1(CAuthentication, CPObject)
 public:
					            CAuthentication ();
	virtual const char* NameOf() const { return "CAuthentication";}
	virtual void        Dump(std::ostream& msg) const;

	char*          GetSerialNumber ();
	void           SetSerialNumber (const char* theNum);

	ePlatformType  GetPlatformType ();
	void           SetPlatformType (const ePlatformType type);

	VERSION_S      GetMcuVersionFromMpl ();
	void           SetMcuVersionFromMpl (const VERSION_S mcuVer);
	
	VERSION_S 	   GetMcuChassisVersionFromMpl ();
	void 		   SetMcuChassisVersionFromMpl (const VERSION_S chassisVer);

	const char*    Get_X_KeyCode () const;
	void           Set_X_KeyCode (const char* keyCode);

	const char*    Get_U_KeyCode () const;
	void           Set_U_KeyCode (const char* keyCode);

	void           SetData(const char *data);

	VERSION_S 	   GetkeyCodeVersionFromMpl ();

	APIU8          GetIsNewCtrlGeneration ();

 protected:
	MPL_AUTHENTICATION_S m_authenticationStruct;
};

#endif
