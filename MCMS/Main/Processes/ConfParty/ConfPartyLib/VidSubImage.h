#ifndef _CVidSubImage_H_
#define _CVidSubImage_H_

#include "PObject.h"
#include "Image.h"
#include "ConfPartyDefines.h"

class CImage;
class CTaskApp;

////////////////////////////////////////////////////////////////////////////
//                        CVidSubImage
////////////////////////////////////////////////////////////////////////////
class CVidSubImage : public CPObject
{
	CLASS_TYPE_1(CVidSubImage, CPObject)

public:
	                      CVidSubImage(void);
	                      CVidSubImage(const CVidSubImage&);
	virtual              ~CVidSubImage(void);

	virtual const char*   NameOf() const { return "CVidSubImage";}

	WORD                  GetStartX(void) const                { return m_start_x; }
	void                  SetStartX(WORD start_x)              { m_start_x = start_x; }

	WORD                  GetStartY(void) const                { return m_start_y; }
	void                  SetStartY(WORD start_y)              { m_start_y = start_y; }
	void                  SetStart(WORD start_x, WORD start_y) { m_start_x = start_x; m_start_y = start_y; }

	WORD                  GetSizeX(void) const                 { return m_sizeX; }
	void                  SetSizeX(WORD size_x)                { m_sizeX = size_x; }

	WORD                  GetSizeY(void) const                 { return m_sizeY; }
	void                  SetSizeY(WORD size_y)                { m_sizeY = size_y; }

	BYTE                  noImgSet(void) const;
	void                  SetImageId(DWORD imageId)            { m_imageId = imageId; }
	DWORD                 GetImageId() const                   { return m_imageId; }

	BYTE                  CanBeSetTo(const RequestPriority who_ask, const VideoActivities toDo, const CTaskApp* pSenderParty = NIL(CTaskApp)) const;
	void                  SetLayoutTypeAndSizes(LayoutType newType, WORD place = AUTO);

	void                  SetPartyForceName(const char* pPartyName);
	void 				  SetForcedNotFoundCell(bool val);
	bool 				  GetForcedNotFoundCell() const {return m_forcedNotFoundCell;}
	const char*           GetPartyForce() const                { return m_pForcedToSee; }
	RequestPriority       GetRequestPriority(void) const       { return m_who_it_asked; }
	VideoActivities       GetVideoActivities(void) const       { return m_what_was_done; }
	void                  SetForceAttributes(const RequestPriority rp, const VideoActivities va);
	void                  RemovePartyForceName(void);
	BYTE                  isForcedInConfLevel(void) const;
	BYTE                  isForcedInPartLevel(void) const;
	BYTE                  isBlanked(void) const;
	void                  SetBlankedCell() { m_what_was_done = BLANK_PARTY_Activ; }
	BYTE                  isForced(void) const;
	void                  ClearForce(RequestPriority who_asked = AUTO_Prior);
	void                  CleanUp(void);
	void                  Dump(std::ostream& msg) const;
	void                  DumpForces(std::ostream& msg) const;

	BYTE                  IsAutoScan(void) const;
	BYTE                  IsEqualOrLargeSize(const CVidSubImage& im)const;

protected:
	BYTE                  RelationTable(const RequestPriority who_asked, const VideoActivities what_done) const;

	DWORD                 m_imageId;
	char*                 m_pForcedToSee;
	RequestPriority       m_who_it_asked;
	VideoActivities       m_what_was_done;
	WORD                  m_sizeX;
	WORD                  m_sizeY;
	WORD                  m_start_x;
	WORD                  m_start_y;
	bool 				  m_forcedNotFoundCell;

private:
	BYTE                  operator==(const CImage& otherImage) const { return (m_imageId == otherImage.GetArtPartyId()); }
	BYTE                  operator!=(const CImage& otherImage) const { return !(*this == otherImage); }
	friend BYTE           operator==(const CVidSubImage& first, const CVidSubImage& second);
};

#endif // _CVidSubImage_H_


