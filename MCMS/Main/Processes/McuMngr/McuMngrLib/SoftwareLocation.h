// SoftwareLocation.h: interface for the CSoftwareLocation class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _SoftwareLocation_H_
#define _SoftwareLocation_H_


#include "PObject.h"
#include "DataTypes.h"
#include "CardsStructs.h"
#include "McuMngrStructs.h"



class CSoftwareLocation : public CPObject
{

CLASS_TYPE_1(CSoftwareLocation, CPObject)

public:
	CSoftwareLocation ();
	virtual const char* NameOf() const { return "CSoftwareLocation";}
	virtual ~CSoftwareLocation ();
	virtual void Dump(std::ostream& msg) const;

	CSoftwareLocation& operator = (const CSoftwareLocation &rOther);

	BYTE*     GetHostName ();
	void      SetHostName (const BYTE *name);

	DWORD     GetHostIp ();
	void      SetHostIp (const DWORD ip);

	BYTE*     GetLocation ();
	void      SetLocation (const BYTE *location);

	eUrlType  GetUrlType ();
	void      SetUrlType (const eUrlType type);

	BYTE*     GetUserName ();
	void      SetUserName (const BYTE *name);

	BYTE*     GetPassword ();
	void      SetPassword (const BYTE *pwd);

	DWORD     GetVLanId ();
	void      SetVLanId (const DWORD id);

	void      SetData(const char *data);

    void      ValidateStrings();

protected:
	MPL_SW_LOCATION_S m_swLocationStruct;
};

#endif // _SoftwareLocation_H_
