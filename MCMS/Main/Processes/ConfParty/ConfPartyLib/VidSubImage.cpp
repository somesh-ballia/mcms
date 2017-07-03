#include "VidSubImage.h"
#include "ObjString.h"
#include "Party.h"
#include "ProcessBase.h"
#include "SysConfig.h"
#include "ProcessBase.h"

class TaskApp;

////////////////////////////////////////////////////////////////////////////
//                        CVidSubImage
////////////////////////////////////////////////////////////////////////////
CVidSubImage::CVidSubImage()
{
	m_imageId       = 0;
	m_pForcedToSee  = NULL;
	m_start_x       = AUTO;
	m_start_y       = AUTO;
	m_sizeX         = AUTO;
	m_sizeY         = AUTO;
	m_who_it_asked  = AUTO_Prior;
	m_what_was_done = DEFAULT_Activ;
	m_forcedNotFoundCell   = false;
}

//--------------------------------------------------------------------------
CVidSubImage::~CVidSubImage()
{
	PDELETEA(m_pForcedToSee);
}

//--------------------------------------------------------------------------
CVidSubImage::CVidSubImage(const CVidSubImage& rhs)
	: CPObject(rhs)
{
	m_imageId       = rhs.GetImageId();
	m_start_x       = rhs.GetStartX();
	m_start_y       = rhs.GetStartY();
	m_sizeX         = rhs.GetSizeX();
	m_sizeY         = rhs.GetSizeY();
	m_who_it_asked  = rhs.GetRequestPriority();
	m_what_was_done = rhs.GetVideoActivities();
	m_pForcedToSee  = NIL(char);
	m_forcedNotFoundCell  = rhs.GetForcedNotFoundCell();

	SetPartyForceName(rhs.GetPartyForce());
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//--------------------------------------------------------------------------
void CVidSubImage::SetPartyForceName(const char* pParty)
{
	PDELETEA(m_pForcedToSee);

	if (pParty == NULL)
		return;

	m_pForcedToSee = new char[H243_NAME_LEN];
	strcpy_safe(m_pForcedToSee, H243_NAME_LEN, pParty);
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//--------------------------------------------------------------------------
void CVidSubImage::DumpForces(std::ostream& msg) const
{
	if (isForced())
	{
		const RequestPriority rp = GetRequestPriority();
		if (rp == PARTY_Prior)
		{
			msg << " Forced by remote ";
		}
		else if (rp == CHAIRMAN_Prior)
		{
			msg << " Forced by chairman ";
		}
		else if (rp == OPERATOR_Prior)
		{
			msg << " Forced by operator ";
		}
		else
		{
			msg << " Error happened in RequestPriority - found:";
			switch (rp)
			{
				case AUTO_Prior     : { msg << "AUTO_Prior"; break;}
				case PARTY_Prior    : { msg << "PARTY_Prior"; break;}
				case CHAIRMAN_Prior : { msg << "CHAIRMAN_Prior"; break;}
				case MCMS_Prior     : { msg << "MCMS_Prior"; break;}
				case SYNC_LOST_Prior: { msg << "SYNC_LOST_Prior"; break;}
				case OPERATOR_Prior : { msg << "OPERATOR_Prior"; break;}
				default             : { msg << "Unknown type";}
			} // switch
		}

		const VideoActivities va = GetVideoActivities();
		if (va == FORCE_PARTY_Activ || va == FORCE_PRIVATE_PARTY_Active)  // Private layout
		{
			msg << " in party level ";
		}
		else if (va == FORCE_CONF_Activ || va == FORCE_MVC_CONF_Activ)
		{
			msg << " in bridge level ";
		}
		else if (va == FORCE_CONTENT_Activ)
		{
			msg << " in bridge level (Content) ";
		}
		else
		{
			msg << "Error happened in VideoActivities! - found: ";
			switch (va)
			{
				case DEFAULT_Activ             : { msg << "DEFAULT_Activ"; break;}
				case AUTO_PARTY_Activ          : { msg << "AUTO_PARTY_Activ"; break;}
				case FORCE_PARTY_Activ         : { msg << "FORCE_PARTY_Activ"; break;}
				case BLANK_PARTY_Activ         : { msg << "BLANK_PARTY_Activ"; break;}
				case AUTO_CONF_Activ           : { msg << "AUTO_CONF_Activ"; break;}
				case FORCE_CONF_Activ          : { msg << "FORCE_CONF_Activ"; break;}
				case FORCE_MVC_CONF_Activ      : { msg << "FORCE_MVC_CONF_Activ"; break;}
				case FORCE_CONTENT_Activ       : { msg << "FORCE_CONTENT_Activ"; break;}
				case BLANK_CONF_Activ          : { msg << "BLANK_CONF_Activ"; break;}
				case FORCE_PRIVATE_PARTY_Active: { msg << "FORCE_PRIVATE_PARTY_Active"; break;} // Private layout
				default                        : { msg << "Unknown activity"; }
			} // switch
		}

		if (GetPartyForce() == NIL(char))
		{
			msg << "Error in forced name " << "\n";
		}
		else
		{
			ALLOCBUFFER(pBuf, H243_NAME_LEN);
			strncpy(pBuf, GetPartyForce(), H243_NAME_LEN-1);
			pBuf[H243_NAME_LEN-1] = '\0';
			msg << " forced to " << pBuf << "\n";
			DEALLOCBUFFER(pBuf);
		}
	}
	else if (isBlanked())
	{
		const RequestPriority rp = GetRequestPriority();
		if (rp == PARTY_Prior)
		{
			msg << " Blanked by remote ";
		}
		else if (rp == CHAIRMAN_Prior)
		{
			msg << " Blanked by chairman ";
		}
		else if (rp == OPERATOR_Prior)
		{
			msg << " Blanked by operator ";
		}
		else
		{
			msg << " Error happened in RequestPriority - found:";
			switch (rp)
			{
				case AUTO_Prior     : { msg << "AUTO_Prior"; break;}
				case PARTY_Prior    : { msg << "PARTY_Prior"; break;}
				case CHAIRMAN_Prior : { msg << "CHAIRMAN_Prior"; break;}
				case SYNC_LOST_Prior: { msg << "SYNC_LOST_Prior"; break;}
				case MCMS_Prior     : { msg << "MCMS_Prior"; break; }
				case OPERATOR_Prior : { msg << "OPERATOR_Prior"; break;}
				default             : { msg << "Unknown type";}
			} // switch
		}

		const VideoActivities va = GetVideoActivities();
		if (va == BLANK_PARTY_Activ)
		{
			msg << " in party level";
		}
		else if (va == BLANK_CONF_Activ)
		{
			msg << " in bridge level";
		}
		else
		{
			msg << " Error happened in VideoActivities - found:";
			switch (va)
			{
				case DEFAULT_Activ             : { msg << "DEFAULT_Activ"; break;}
				case AUTO_PARTY_Activ          : { msg << "AUTO_PARTY_Activ"; break;}
				case FORCE_PARTY_Activ         : { msg << "FORCE_PARTY_Activ"; break;}
				case BLANK_PARTY_Activ         : { msg << "BLANK_PARTY_Activ"; break;}
				case AUTO_CONF_Activ           : { msg << "AUTO_CONF_Activ"; break;}
				case FORCE_CONF_Activ          : { msg << "FORCE_CONF_Activ"; break;}
				case FORCE_MVC_CONF_Activ      : { msg << "FORCE_MVC_CONF_Activ"; break;}
				case FORCE_CONTENT_Activ       : { msg << "FORCE_CONTENT_Activ"; break;}
				case BLANK_CONF_Activ          : { msg << "BLANK_CONF_Activ"; break;}
				case BLANK_PRIVATE_PARTY_Active: { msg << "BLANK_PRIVATE_PARTY_Active"; break;}
				default                        : { msg << "Unknown activity"; }
			} // switch
		}

		if (GetPartyForce() != NIL(char))
		{
			msg << " Error in blanked name - still forced to somebody";
		}
	}
	else
	{
		if (GetRequestPriority() == AUTO_Prior && (GetVideoActivities() == DEFAULT_Activ || GetVideoActivities() == AUTO_SCAN_Active))
		{

		}
		else
		{
			msg << " Error in recognition - found:";
			switch (GetVideoActivities())
			{
				case DEFAULT_Activ       : { msg << "DEFAULT_Activ"; break;}
				case AUTO_PARTY_Activ    : { msg << "AUTO_PARTY_Activ"; break;}
				case FORCE_PARTY_Activ   : { msg << "FORCE_PARTY_Activ"; break;}
				case BLANK_PARTY_Activ   : { msg << "BLANK_PARTY_Activ"; break;}
				case AUTO_CONF_Activ     : { msg << "AUTO_CONF_Activ"; break;}
				case FORCE_CONF_Activ    : { msg << "FORCE_CONF_Activ"; break;}
				case FORCE_MVC_CONF_Activ: { msg << "FORCE_MVC_CONF_Activ"; break;}
				case FORCE_CONTENT_Activ : { msg << "FORCE_CONTENT_Activ"; break;}
				case BLANK_CONF_Activ    : { msg << "BLANK_CONF_Activ"; break;}
				default                  : { msg << "Unknown activity"; }
			} // switch

			switch (GetRequestPriority())
			{
				case AUTO_Prior     : { msg << "AUTO_Prior"; break;}
				case PARTY_Prior    : { msg << "PARTY_Prior"; break;}
				case CHAIRMAN_Prior : { msg << "CHAIRMAN_Prior"; break;}
				case SYNC_LOST_Prior: { msg << "SYNC_LOST_Prior"; break;}
				case MCMS_Prior     : { msg << "MCMS_Prior"; break; }
				case OPERATOR_Prior : { msg << "OPERATOR_Prior"; break;}
				default             : { msg << "Unknown type";}
			} // switch
		}
	}
}

//--------------------------------------------------------------------------
void CVidSubImage::Dump(std::ostream& msg) const
{
	msg << "sizeX:"<< m_sizeX << ", sizeY:" << m_sizeY << ", startX:" << m_start_x << ", startY:" << m_start_y << ", partyId:" << GetImageId();
	msg << "\n  Forces for the subImage:";
	DumpForces(msg);
}

//--------------------------------------------------------------------------
BYTE CVidSubImage::RelationTable(const RequestPriority who_asked, const VideoActivities what_done_par) const
{
	if (m_who_it_asked == AUTO_Prior &&
	    m_what_was_done == DEFAULT_Activ)
		return YES;

	if (m_who_it_asked == AUTO_Prior &&
	    m_what_was_done == AUTO_SCAN_Active)
		return YES;

	// the BLANK ACTIV is same as FORCE
	VideoActivities what_done;
	if (what_done_par == BLANK_PARTY_Activ)
		what_done = FORCE_PARTY_Activ;
	else if (what_done_par == BLANK_CONF_Activ)
		what_done = FORCE_CONF_Activ;
	else if (what_done_par == FORCE_MVC_CONF_Activ)
		what_done = FORCE_CONF_Activ;
	else if (what_done_par == BLANK_PRIVATE_PARTY_Active)
		what_done = FORCE_PRIVATE_PARTY_Active;
	else
		what_done = what_done_par;

	VideoActivities memb_what_was_done_temp;
	if (m_what_was_done == BLANK_PARTY_Activ)
		memb_what_was_done_temp = FORCE_PARTY_Activ;
	else if (m_what_was_done == BLANK_CONF_Activ)
		memb_what_was_done_temp = FORCE_CONF_Activ;
	else if (m_what_was_done == FORCE_MVC_CONF_Activ)
		memb_what_was_done_temp = FORCE_CONF_Activ;
	else if (m_what_was_done == BLANK_PRIVATE_PARTY_Active)
		memb_what_was_done_temp = FORCE_PRIVATE_PARTY_Active;
	else
		memb_what_was_done_temp = m_what_was_done;

// Begin of the table:

// 1. =================================================
	if (who_asked == OPERATOR_Prior &&
	    what_done == FORCE_CONF_Activ &&
	    // ---------------------------------
	    m_who_it_asked == OPERATOR_Prior &&
	    memb_what_was_done_temp == FORCE_CONF_Activ)
		return YES;
	else
// 2. =================================================
	if (who_asked == CHAIRMAN_Prior &&
	    what_done == FORCE_CONF_Activ &&
	    // ---------------------------------
	    m_who_it_asked == OPERATOR_Prior &&
	    memb_what_was_done_temp == FORCE_CONF_Activ)
		return YES;
	else
// 3. =================================================
	if (who_asked == PARTY_Prior &&
	    what_done == FORCE_CONF_Activ &&
	    // ---------------------------------
	    m_who_it_asked == OPERATOR_Prior &&
	    memb_what_was_done_temp == FORCE_CONF_Activ)
		return YES;
	else
// 4. =================================================
	if (who_asked == OPERATOR_Prior &&
	    what_done == FORCE_PARTY_Activ &&
	    // ---------------------------------
	    m_who_it_asked == OPERATOR_Prior &&
	    memb_what_was_done_temp == FORCE_CONF_Activ)
		return NO;
	else
// 5. =================================================
	if (who_asked == CHAIRMAN_Prior &&
	    what_done == FORCE_PARTY_Activ &&
	    // ---------------------------------
	    m_who_it_asked == OPERATOR_Prior &&
	    memb_what_was_done_temp == FORCE_CONF_Activ)
		return YES;
	else
// 6. =================================================
	if (who_asked == PARTY_Prior &&
	    what_done == FORCE_PARTY_Activ &&
	    // ---------------------------------
	    m_who_it_asked == OPERATOR_Prior &&
	    memb_what_was_done_temp == FORCE_CONF_Activ)
		return NO;
	else
// 7. =================================================
	if (who_asked == OPERATOR_Prior &&
	    what_done == FORCE_CONF_Activ &&
	    // ---------------------------------
	    m_who_it_asked == CHAIRMAN_Prior &&
	    memb_what_was_done_temp == FORCE_CONF_Activ)
		return YES;
	else
// 8. =================================================
	if (who_asked == CHAIRMAN_Prior &&
	    what_done == FORCE_CONF_Activ &&
	    // ---------------------------------
	    m_who_it_asked == CHAIRMAN_Prior &&
	    memb_what_was_done_temp == FORCE_CONF_Activ)
		return YES;
	else
// 9. =================================================
	if (who_asked == PARTY_Prior &&
	    what_done == FORCE_CONF_Activ &&
	    // ---------------------------------
	    m_who_it_asked == CHAIRMAN_Prior &&
	    memb_what_was_done_temp == FORCE_CONF_Activ)
		return NO;
	else
// 10. =================================================
	if (who_asked == OPERATOR_Prior &&
	    what_done == FORCE_PARTY_Activ &&
	    // ---------------------------------
	    m_who_it_asked == CHAIRMAN_Prior &&
	    memb_what_was_done_temp == FORCE_CONF_Activ)
		return NO;
	else
// 11. =================================================
	if (who_asked == CHAIRMAN_Prior &&
	    what_done == FORCE_PARTY_Activ &&
	    // ---------------------------------
	    m_who_it_asked == CHAIRMAN_Prior &&
	    memb_what_was_done_temp == FORCE_CONF_Activ)
		return YES;
	else
// 12. =================================================
	if (who_asked == PARTY_Prior &&
	    what_done == FORCE_PARTY_Activ &&
	    // ---------------------------------
	    m_who_it_asked == CHAIRMAN_Prior &&
	    memb_what_was_done_temp == FORCE_CONF_Activ)
		return NO;
	else
// 13. =================================================
	if (who_asked == OPERATOR_Prior &&
	    what_done == FORCE_CONF_Activ &&
	    // ---------------------------------
	    m_who_it_asked == PARTY_Prior &&
	    memb_what_was_done_temp == FORCE_CONF_Activ)
		return YES;
	else
// 14. =================================================
	if (who_asked == CHAIRMAN_Prior &&
	    what_done == FORCE_CONF_Activ &&
	    // ---------------------------------
	    m_who_it_asked == PARTY_Prior &&
	    memb_what_was_done_temp == FORCE_CONF_Activ)
		return YES;
	else
// 15. =================================================
	if (who_asked == PARTY_Prior &&
	    what_done == FORCE_CONF_Activ &&
	    // ---------------------------------
	    m_who_it_asked == PARTY_Prior &&
	    memb_what_was_done_temp == FORCE_CONF_Activ)
		return NO;
	else
// 16. =================================================
	if (who_asked == OPERATOR_Prior &&
	    what_done == FORCE_PARTY_Activ &&
	    // ---------------------------------
	    m_who_it_asked == PARTY_Prior &&
	    memb_what_was_done_temp == FORCE_CONF_Activ)
		return NO;
	else
// 17. =================================================
	if (who_asked == CHAIRMAN_Prior &&
	    what_done == FORCE_PARTY_Activ &&
	    // ---------------------------------
	    m_who_it_asked == PARTY_Prior &&
	    memb_what_was_done_temp == FORCE_CONF_Activ)
		return YES;
	else
// 18. =================================================
	if (who_asked == PARTY_Prior &&
	    what_done == FORCE_PARTY_Activ &&
	    // ---------------------------------
	    m_who_it_asked == PARTY_Prior &&
	    memb_what_was_done_temp == FORCE_CONF_Activ)
		return NO;
	else
// 19. =================================================
	if (who_asked == OPERATOR_Prior &&
	    what_done == FORCE_CONF_Activ &&
	    // ---------------------------------
	    m_who_it_asked == OPERATOR_Prior &&
	    memb_what_was_done_temp == FORCE_PARTY_Activ)
		return YES;
	else
// 20. =================================================
	if (who_asked == CHAIRMAN_Prior &&
	    what_done == FORCE_CONF_Activ &&
	    // ---------------------------------
	    m_who_it_asked == OPERATOR_Prior &&
	    memb_what_was_done_temp == FORCE_PARTY_Activ)
		return YES;
	else
// 21. =================================================
	if (who_asked == PARTY_Prior &&
	    what_done == FORCE_CONF_Activ &&
	    // ---------------------------------
	    m_who_it_asked == OPERATOR_Prior &&
	    memb_what_was_done_temp == FORCE_PARTY_Activ)
		return YES;
	else
// 22. =================================================
	if (who_asked == OPERATOR_Prior &&
	    what_done == FORCE_PARTY_Activ &&
	    // ---------------------------------
	    m_who_it_asked == OPERATOR_Prior &&
	    memb_what_was_done_temp == FORCE_PARTY_Activ)
		return YES;
	else
// 23. =================================================
	if (who_asked == CHAIRMAN_Prior &&
	    what_done == FORCE_PARTY_Activ &&
	    // ---------------------------------
	    m_who_it_asked == OPERATOR_Prior &&
	    memb_what_was_done_temp == FORCE_PARTY_Activ)
		return YES;
	else
// 24. =================================================
	if (who_asked == PARTY_Prior &&
	    what_done == FORCE_PARTY_Activ &&
	    // ---------------------------------
	    m_who_it_asked == OPERATOR_Prior &&
	    memb_what_was_done_temp == FORCE_PARTY_Activ)
		return YES;
	else
// 25. =================================================
	if (who_asked == OPERATOR_Prior &&
	    what_done == FORCE_CONF_Activ &&
	    // ---------------------------------
	    m_who_it_asked == CHAIRMAN_Prior &&
	    memb_what_was_done_temp == FORCE_PARTY_Activ)
		return YES;
	else
// 26. =================================================
	if (who_asked == CHAIRMAN_Prior &&
	    what_done == FORCE_CONF_Activ &&
	    // ---------------------------------
	    m_who_it_asked == CHAIRMAN_Prior &&
	    memb_what_was_done_temp == FORCE_PARTY_Activ)
		return NO;
	else
// 27. =================================================
	if (who_asked == PARTY_Prior &&
	    what_done == FORCE_CONF_Activ &&
	    // ---------------------------------
	    m_who_it_asked == CHAIRMAN_Prior &&
	    memb_what_was_done_temp == FORCE_PARTY_Activ)
		return NO;
	else
// 28. =================================================
	if (who_asked == OPERATOR_Prior &&
	    what_done == FORCE_PARTY_Activ &&
	    // ---------------------------------
	    m_who_it_asked == CHAIRMAN_Prior &&
	    memb_what_was_done_temp == FORCE_PARTY_Activ)
		return YES;
	else
// 29. =================================================
	if (who_asked == CHAIRMAN_Prior &&
	    what_done == FORCE_PARTY_Activ &&
	    // ---------------------------------
	    m_who_it_asked == CHAIRMAN_Prior &&
	    memb_what_was_done_temp == FORCE_PARTY_Activ)
		return YES;
	else
// 30. =================================================
	if (who_asked == PARTY_Prior &&
	    what_done == FORCE_PARTY_Activ &&
	    // ---------------------------------
	    m_who_it_asked == CHAIRMAN_Prior &&
	    memb_what_was_done_temp == FORCE_PARTY_Activ)
		return YES;
	else
// 31. =================================================
	if (who_asked == OPERATOR_Prior &&
	    what_done == FORCE_CONF_Activ &&
	    // ---------------------------------
	    m_who_it_asked == PARTY_Prior &&
	    memb_what_was_done_temp == FORCE_PARTY_Activ)
		return YES;
	else
// 32. =================================================
	if (who_asked == CHAIRMAN_Prior &&
	    what_done == FORCE_CONF_Activ &&
	    // ---------------------------------
	    m_who_it_asked == PARTY_Prior &&
	    memb_what_was_done_temp == FORCE_PARTY_Activ)
		return YES;
	else
// 33. =================================================
	if (who_asked == PARTY_Prior &&
	    what_done == FORCE_CONF_Activ &&
	    // ---------------------------------
	    m_who_it_asked == PARTY_Prior &&
	    memb_what_was_done_temp == FORCE_PARTY_Activ)
		return YES;
	else
// 34. =================================================
	if (who_asked == OPERATOR_Prior &&
	    what_done == FORCE_PARTY_Activ &&
	    // ---------------------------------
	    m_who_it_asked == PARTY_Prior &&
	    memb_what_was_done_temp == FORCE_PARTY_Activ)
		return YES;
	else
// 35. =================================================
	if (who_asked == CHAIRMAN_Prior &&
	    what_done == FORCE_PARTY_Activ &&
	    // ---------------------------------
	    m_who_it_asked == PARTY_Prior &&
	    memb_what_was_done_temp == FORCE_PARTY_Activ)
		return YES;
	else
// 36. =================================================
	if (who_asked == PARTY_Prior &&
	    what_done == FORCE_PARTY_Activ &&
	    // ---------------------------------
	    m_who_it_asked == PARTY_Prior &&
	    memb_what_was_done_temp == FORCE_PARTY_Activ)
		return YES;
	else
// 1-a. =================================================
	if (who_asked == OPERATOR_Prior &&
	    what_done == AUTO_CONF_Activ &&
	    // ---------------------------------
	    m_who_it_asked == OPERATOR_Prior &&
	    memb_what_was_done_temp == FORCE_CONF_Activ)
		return YES;
	else
// 7-a. =================================================
	if (who_asked == OPERATOR_Prior &&
	    what_done == AUTO_CONF_Activ &&
	    // ---------------------------------
	    m_who_it_asked == CHAIRMAN_Prior &&
	    memb_what_was_done_temp == FORCE_CONF_Activ)
		return YES;
	else
// 8-a. =================================================
	if (who_asked == CHAIRMAN_Prior &&
	    what_done == AUTO_CONF_Activ &&
	    // ---------------------------------
	    m_who_it_asked == CHAIRMAN_Prior &&
	    memb_what_was_done_temp == FORCE_CONF_Activ)
		return YES;
	else
// 13-a. =================================================
	if (who_asked == OPERATOR_Prior &&
	    what_done == AUTO_CONF_Activ &&
	    // ---------------------------------
	    m_who_it_asked == PARTY_Prior &&
	    memb_what_was_done_temp == FORCE_CONF_Activ)
		return YES;
	else
// 15-a. =================================================
	if (who_asked == PARTY_Prior &&
	    what_done == AUTO_CONF_Activ &&
	    // ---------------------------------
	    m_who_it_asked == PARTY_Prior &&
	    memb_what_was_done_temp == FORCE_CONF_Activ)
		return YES;
	else
// 22-a. =================================================
	if (who_asked == OPERATOR_Prior &&
	    what_done == AUTO_PARTY_Activ &&
	    // ---------------------------------
	    m_who_it_asked == OPERATOR_Prior &&
	    memb_what_was_done_temp == FORCE_PARTY_Activ)
		return YES;
	else
// 28-a. =================================================
	if (who_asked == OPERATOR_Prior &&
	    what_done == AUTO_PARTY_Activ &&
	    // ---------------------------------
	    m_who_it_asked == CHAIRMAN_Prior &&
	    memb_what_was_done_temp == FORCE_PARTY_Activ)
		return YES;
	else
// 29-a. =================================================
	if (who_asked == CHAIRMAN_Prior &&
	    what_done == AUTO_PARTY_Activ &&
	    // ---------------------------------
	    m_who_it_asked == CHAIRMAN_Prior &&
	    memb_what_was_done_temp == FORCE_PARTY_Activ)
		return YES;
	else
// 34-a. =================================================
	if (who_asked == OPERATOR_Prior &&
	    what_done == AUTO_PARTY_Activ &&
	    // ---------------------------------
	    m_who_it_asked == PARTY_Prior &&
	    memb_what_was_done_temp == FORCE_PARTY_Activ)
		return YES;
	else
// 35-a. =================================================
	if (who_asked == CHAIRMAN_Prior &&
	    what_done == AUTO_PARTY_Activ &&
	    // ---------------------------------
	    m_who_it_asked == PARTY_Prior &&
	    memb_what_was_done_temp == FORCE_PARTY_Activ)
		return YES;
	else
// 36-a. =================================================
	if (who_asked == PARTY_Prior &&
	    what_done == AUTO_PARTY_Activ &&
	    // ---------------------------------
	    m_who_it_asked == PARTY_Prior &&
	    memb_what_was_done_temp == FORCE_PARTY_Activ)
		return YES;

// =======================================================
// This section for CONTENT force :
// forces  :     37 -- 49
// release :     38-a, 40-a
// =======================================================

// 37 ====================================================
	if (who_asked == PARTY_Prior &&
	    what_done == FORCE_CONTENT_Activ &&
	    // ---------------------------------
	    m_who_it_asked == PARTY_Prior &&
	    memb_what_was_done_temp == FORCE_CONTENT_Activ)
		return YES;

// 38 ====================================================
	if (who_asked == OPERATOR_Prior &&
	    what_done == FORCE_CONF_Activ &&
	    // ---------------------------------
	    m_who_it_asked == PARTY_Prior &&
	    memb_what_was_done_temp == FORCE_CONTENT_Activ)
		return YES;

// 39 ====================================================
	if (who_asked == CHAIRMAN_Prior &&
	    what_done == FORCE_CONF_Activ &&
	    // ---------------------------------
	    m_who_it_asked == PARTY_Prior &&
	    memb_what_was_done_temp == FORCE_CONTENT_Activ)
		return NO;

// 40 ====================================================
	if (who_asked == PARTY_Prior &&
	    what_done == FORCE_CONF_Activ &&
	    // ---------------------------------
	    m_who_it_asked == PARTY_Prior &&
	    memb_what_was_done_temp == FORCE_CONTENT_Activ)
		return NO;

// 41 ====================================================
	if (who_asked == OPERATOR_Prior &&
	    what_done == FORCE_PARTY_Activ &&
	    // ---------------------------------
	    m_who_it_asked == PARTY_Prior &&
	    memb_what_was_done_temp == FORCE_CONTENT_Activ)
		return NO;

// 42 ====================================================
	if (who_asked == CHAIRMAN_Prior &&
	    what_done == FORCE_PARTY_Activ &&
	    // ---------------------------------
	    m_who_it_asked == PARTY_Prior &&
	    memb_what_was_done_temp == FORCE_CONTENT_Activ)
		return NO;

// 43 ====================================================
	if (who_asked == PARTY_Prior &&
	    what_done == FORCE_PARTY_Activ &&
	    // ---------------------------------
	    m_who_it_asked == PARTY_Prior &&
	    memb_what_was_done_temp == FORCE_CONTENT_Activ)
		return NO;

// 44 ====================================================
	if (who_asked == PARTY_Prior &&
	    what_done == FORCE_CONTENT_Activ &&
	    // ---------------------------------
	    m_who_it_asked == OPERATOR_Prior &&
	    memb_what_was_done_temp == FORCE_CONF_Activ)
		return YES;

// 45 ====================================================
	if (who_asked == PARTY_Prior &&
	    what_done == FORCE_CONTENT_Activ &&
	    // ---------------------------------
	    m_who_it_asked == CHAIRMAN_Prior &&
	    memb_what_was_done_temp == FORCE_CONF_Activ)
		return YES;

// 46 ====================================================
	if (who_asked == PARTY_Prior &&
	    what_done == FORCE_CONTENT_Activ &&
	    // ---------------------------------
	    m_who_it_asked == PARTY_Prior &&
	    memb_what_was_done_temp == FORCE_CONF_Activ)
		return YES;

// 47 ====================================================
	if (who_asked == PARTY_Prior &&
	    what_done == FORCE_CONTENT_Activ &&
	    // ---------------------------------
	    m_who_it_asked == OPERATOR_Prior &&
	    memb_what_was_done_temp == FORCE_PARTY_Activ)
		return YES;

// 48 ====================================================
	if (who_asked == PARTY_Prior &&
	    what_done == FORCE_CONTENT_Activ &&
	    // ---------------------------------
	    m_who_it_asked == CHAIRMAN_Prior &&
	    memb_what_was_done_temp == FORCE_PARTY_Activ)
		return YES;

// 49 ====================================================
	if (who_asked == PARTY_Prior &&
	    what_done == FORCE_CONTENT_Activ &&
	    // ---------------------------------
	    m_who_it_asked == PARTY_Prior &&
	    memb_what_was_done_temp == FORCE_PARTY_Activ)
		return YES;

// 38-a. =================================================
	if (who_asked == OPERATOR_Prior &&
	    what_done == AUTO_CONF_Activ &&
	    // ---------------------------------
	    m_who_it_asked == PARTY_Prior &&
	    memb_what_was_done_temp == FORCE_CONTENT_Activ)
		return YES;

// 40-a. =================================================
	if (who_asked == PARTY_Prior &&
	    what_done == AUTO_CONF_Activ &&
	    // ---------------------------------
	    m_who_it_asked == PARTY_Prior &&
	    memb_what_was_done_temp == FORCE_CONTENT_Activ)
		return YES;

// =======================================================
// This section for Private layout force :
// forces  : 51 - 76
// release : 60a - d
// =======================================================

// 51 ====================================================
	if (who_asked == PARTY_Prior &&
	    what_done == FORCE_PRIVATE_PARTY_Active &&
	    // ---------------------------------
	    m_who_it_asked == OPERATOR_Prior &&
	    memb_what_was_done_temp == FORCE_CONF_Activ)
		return YES;

// 52 ====================================================
	if (who_asked == PARTY_Prior &&
	    what_done == FORCE_PRIVATE_PARTY_Active &&
	    // ---------------------------------
	    m_who_it_asked == CHAIRMAN_Prior &&
	    memb_what_was_done_temp == FORCE_CONF_Activ)
		return NO;

// 53 ====================================================
	if (who_asked == PARTY_Prior &&
	    what_done == FORCE_PRIVATE_PARTY_Active &&
	    // ---------------------------------
	    m_who_it_asked == PARTY_Prior &&
	    memb_what_was_done_temp == FORCE_CONF_Activ)
		return NO;

// 54 ====================================================
	if (who_asked == PARTY_Prior &&
	    what_done == FORCE_PRIVATE_PARTY_Active &&
	    // ---------------------------------
	    m_who_it_asked == OPERATOR_Prior &&
	    memb_what_was_done_temp == FORCE_PARTY_Activ)
		return YES;

// 55 ====================================================
	if (who_asked == PARTY_Prior &&
	    what_done == FORCE_PRIVATE_PARTY_Active &&
	    // ---------------------------------
	    m_who_it_asked == CHAIRMAN_Prior &&
	    memb_what_was_done_temp == FORCE_PARTY_Activ)
		return NO;

// 56 ====================================================
	if (who_asked == PARTY_Prior &&
	    what_done == FORCE_PRIVATE_PARTY_Active &&
	    // ---------------------------------
	    m_who_it_asked == PARTY_Prior &&
	    memb_what_was_done_temp == FORCE_PARTY_Activ)
		return NO;

// 57 ====================================================
	if (who_asked == PARTY_Prior &&
	    what_done == FORCE_PRIVATE_PARTY_Active &&
	    // ---------------------------------
	    m_who_it_asked == PARTY_Prior &&
	    memb_what_was_done_temp == FORCE_PRIVATE_PARTY_Active)
		return YES;

// 58 ====================================================
	if (who_asked == OPERATOR_Prior &&
	    what_done == FORCE_CONF_Activ &&
	    // ---------------------------------
	    m_who_it_asked == PARTY_Prior &&
	    memb_what_was_done_temp == FORCE_PRIVATE_PARTY_Active)
		return NO;

// 59 ====================================================
	if (who_asked == CHAIRMAN_Prior &&
	    what_done == FORCE_CONF_Activ &&
	    // ---------------------------------
	    m_who_it_asked == PARTY_Prior &&
	    memb_what_was_done_temp == FORCE_PRIVATE_PARTY_Active)
		return YES;

// 60 ====================================================
	if (who_asked == PARTY_Prior &&
	    what_done == FORCE_CONF_Activ &&
	    // ---------------------------------
	    m_who_it_asked == PARTY_Prior &&
	    memb_what_was_done_temp == FORCE_PRIVATE_PARTY_Active)
		return YES;

// 61 ====================================================
	if (who_asked == CHAIRMAN_Prior &&
	    what_done == FORCE_PARTY_Activ &&
	    // ---------------------------------
	    m_who_it_asked == PARTY_Prior &&
	    memb_what_was_done_temp == FORCE_PRIVATE_PARTY_Active)
		return YES;

// 62 ====================================================
	if (who_asked == PARTY_Prior &&
	    what_done == FORCE_PARTY_Activ &&
	    // ---------------------------------
	    m_who_it_asked == PARTY_Prior &&
	    memb_what_was_done_temp == FORCE_PRIVATE_PARTY_Active)
		return YES;

// 63 ====================================================
	if (who_asked == OPERATOR_Prior &&
	    what_done == FORCE_PARTY_Activ &&
	    // ---------------------------------
	    m_who_it_asked == PARTY_Prior &&
	    memb_what_was_done_temp == FORCE_PRIVATE_PARTY_Active)
		return NO;

// 64 ====================================================
	if (who_asked == OPERATOR_Prior &&
	    what_done == FORCE_PRIVATE_PARTY_Active &&
	    // ---------------------------------
	    m_who_it_asked == OPERATOR_Prior &&
	    memb_what_was_done_temp == FORCE_CONF_Activ)
		return YES;

// 65 ====================================================
	if (who_asked == OPERATOR_Prior &&
	    what_done == FORCE_PRIVATE_PARTY_Active &&
	    // ---------------------------------
	    m_who_it_asked == CHAIRMAN_Prior &&
	    memb_what_was_done_temp == FORCE_CONF_Activ)
		return NO;

// 66 ====================================================
	if (who_asked == OPERATOR_Prior &&
	    what_done == FORCE_PRIVATE_PARTY_Active &&
	    // ---------------------------------
	    m_who_it_asked == PARTY_Prior &&
	    memb_what_was_done_temp == FORCE_CONF_Activ)
		return NO;

// 67 ====================================================
	if (who_asked == OPERATOR_Prior &&
	    what_done == FORCE_PRIVATE_PARTY_Active &&
	    // ---------------------------------
	    m_who_it_asked == OPERATOR_Prior &&
	    memb_what_was_done_temp == FORCE_PARTY_Activ)
		return YES;

// 68 ====================================================
	if (who_asked == OPERATOR_Prior &&
	    what_done == FORCE_PRIVATE_PARTY_Active &&
	    // ---------------------------------
	    m_who_it_asked == CHAIRMAN_Prior &&
	    memb_what_was_done_temp == FORCE_PARTY_Activ)
		return NO;

// 69 ====================================================
	if (who_asked == OPERATOR_Prior &&
	    what_done == FORCE_PRIVATE_PARTY_Active &&
	    // ---------------------------------
	    m_who_it_asked == PARTY_Prior &&
	    memb_what_was_done_temp == FORCE_PARTY_Activ)
		return NO;

// 70 ====================================================
	if (who_asked == OPERATOR_Prior &&
	    what_done == FORCE_PRIVATE_PARTY_Active &&
	    // ---------------------------------
	    m_who_it_asked == PARTY_Prior &&
	    memb_what_was_done_temp == FORCE_PRIVATE_PARTY_Active)
		return YES;

// 71 ====================================================
	if (who_asked == OPERATOR_Prior &&
	    what_done == FORCE_CONF_Activ &&
	    // ---------------------------------
	    m_who_it_asked == OPERATOR_Prior &&
	    memb_what_was_done_temp == FORCE_PRIVATE_PARTY_Active)
		return NO;

// 72 ====================================================
	if (who_asked == CHAIRMAN_Prior &&
	    what_done == FORCE_CONF_Activ &&
	    // ---------------------------------
	    m_who_it_asked == OPERATOR_Prior &&
	    memb_what_was_done_temp == FORCE_PRIVATE_PARTY_Active)
		return YES;

// 73 ====================================================
	if (who_asked == PARTY_Prior &&
	    what_done == FORCE_CONF_Activ &&
	    // ---------------------------------
	    m_who_it_asked == OPERATOR_Prior &&
	    memb_what_was_done_temp == FORCE_PRIVATE_PARTY_Active)
		return YES;

// 74 ====================================================
	if (who_asked == CHAIRMAN_Prior &&
	    what_done == FORCE_PARTY_Activ &&
	    // ---------------------------------
	    m_who_it_asked == OPERATOR_Prior &&
	    memb_what_was_done_temp == FORCE_PRIVATE_PARTY_Active)
		return YES;

// 75 ====================================================
	if (who_asked == PARTY_Prior &&
	    what_done == FORCE_PARTY_Activ &&
	    // ---------------------------------
	    m_who_it_asked == OPERATOR_Prior &&
	    memb_what_was_done_temp == FORCE_PRIVATE_PARTY_Active)
		return YES;

// 76 ====================================================
	if (who_asked == OPERATOR_Prior &&
	    what_done == FORCE_PARTY_Activ &&
	    // ---------------------------------
	    m_who_it_asked == OPERATOR_Prior &&
	    memb_what_was_done_temp == FORCE_PRIVATE_PARTY_Active)
		return NO;

// 77 ====================================================
	if (who_asked == PARTY_Prior &&
	    what_done == FORCE_PRIVATE_PARTY_Active &&
	    // ---------------------------------
	    m_who_it_asked == OPERATOR_Prior &&
	    memb_what_was_done_temp == FORCE_PRIVATE_PARTY_Active)
		return YES;

// 78 ====================================================
	if (who_asked == OPERATOR_Prior &&
	    what_done == FORCE_PRIVATE_PARTY_Active &&
	    // ---------------------------------
	    m_who_it_asked == OPERATOR_Prior &&
	    memb_what_was_done_temp == FORCE_PRIVATE_PARTY_Active)
		return YES;

// 60a ====================================================
	if (who_asked == PARTY_Prior &&
	    what_done == AUTO_PARTY_Activ &&
	    // ---------------------------------
	    m_who_it_asked == PARTY_Prior &&
	    memb_what_was_done_temp == FORCE_PRIVATE_PARTY_Active)
		return YES;

// 60b ====================================================
	if (who_asked == OPERATOR_Prior &&
	    what_done == AUTO_PARTY_Activ &&
	    // ---------------------------------
	    m_who_it_asked == OPERATOR_Prior &&
	    memb_what_was_done_temp == FORCE_PRIVATE_PARTY_Active)
		return YES;

// 60c ====================================================
	if (who_asked == OPERATOR_Prior &&
	    what_done == AUTO_PARTY_Activ &&
	    // ---------------------------------
	    m_who_it_asked == PARTY_Prior &&
	    memb_what_was_done_temp == FORCE_PRIVATE_PARTY_Active)
		return YES;

// 60d ====================================================
	if (who_asked == PARTY_Prior &&
	    what_done == AUTO_PARTY_Activ &&
	    // ---------------------------------
	    m_who_it_asked == OPERATOR_Prior &&
	    memb_what_was_done_temp == FORCE_PRIVATE_PARTY_Active)
		return YES;

// 60e ====================================================
	if (who_asked == PARTY_Prior &&
	    what_done == AUTO_PARTY_Activ &&
	    // ---------------------------------
	    m_who_it_asked == OPERATOR_Prior &&
	    memb_what_was_done_temp == BLANK_PRIVATE_PARTY_Active)
		return YES;

// 60f ====================================================
	if (who_asked == OPERATOR_Prior &&
	    what_done == AUTO_PARTY_Activ &&
	    // ---------------------------------
	    m_who_it_asked == OPERATOR_Prior &&
	    memb_what_was_done_temp == BLANK_PRIVATE_PARTY_Active)
		return YES;

// 60g ====================================================
	if (who_asked == PARTY_Prior &&
	    what_done == AUTO_PARTY_Activ &&
	    // ---------------------------------
	    m_who_it_asked == PARTY_Prior &&
	    memb_what_was_done_temp == BLANK_PRIVATE_PARTY_Active)
		return YES;

// 60h ====================================================
	if (who_asked == OPERATOR_Prior &&
	    what_done == AUTO_PARTY_Activ &&
	    // ---------------------------------
	    m_who_it_asked == PARTY_Prior &&
	    memb_what_was_done_temp == BLANK_PRIVATE_PARTY_Active)
		return YES;

	else
		return NO;
}

//--------------------------------------------------------------------------
BYTE CVidSubImage::isForcedInConfLevel(void) const
{
	BYTE result = NO;
	if (m_pForcedToSee != NIL(char))
	{
		if (m_what_was_done == FORCE_CONF_Activ)
			result = YES;
		else if (m_what_was_done == FORCE_MVC_CONF_Activ)
			result = YES;
		else if (m_what_was_done == FORCE_CONTENT_Activ)
			result = YES;
		else
		{
			if (m_what_was_done != FORCE_PARTY_Activ && m_what_was_done!=FORCE_PRIVATE_PARTY_Active)
				DBGPASSERT(1);

			result = NO;
		}
	}
	else
	{
		if (m_what_was_done == FORCE_CONF_Activ ||
		    m_what_was_done == FORCE_MVC_CONF_Activ ||
		    m_what_was_done == FORCE_CONTENT_Activ ||
		    m_what_was_done == FORCE_PARTY_Activ)
			DBGPASSERT(1);

		result = NO;
	}

	return result;
}

//--------------------------------------------------------------------------
void CVidSubImage::CleanUp(void)
{
	ClearForce();
	SetImageId(0);
}

//--------------------------------------------------------------------------
void CVidSubImage::ClearForce(RequestPriority who_asked)
{
	// Default prior - Clear all force prior
	if (who_asked == AUTO_Prior)
	{
		SetForceAttributes(AUTO_Prior, DEFAULT_Activ);
		RemovePartyForceName();
	}
	else
	{
		// Clear only "who_asked" prior
		if (m_who_it_asked == who_asked)
		{
			SetForceAttributes(AUTO_Prior, DEFAULT_Activ);
			RemovePartyForceName();
		}
	}
}

//--------------------------------------------------------------------------
BYTE CVidSubImage::isBlanked(void) const
{
	BYTE result = NO;

	if (m_what_was_done == BLANK_PARTY_Activ ||
	    m_what_was_done == BLANK_CONF_Activ
	    || m_what_was_done == BLANK_PRIVATE_PARTY_Active)
	{
		if (m_pForcedToSee != NIL(char))
		{
			DBGPASSERT(1);
		}

		result = YES;
	}

	return result;
}

//--------------------------------------------------------------------------
BYTE CVidSubImage::isForced(void) const
{
	if (isForcedInPartLevel() ||
	    isForcedInConfLevel())
		return YES;
	else
		return NO;
}

//--------------------------------------------------------------------------
BYTE CVidSubImage::isForcedInPartLevel(void) const
{
	BYTE result = NO;
	if (m_pForcedToSee != NIL(char))  // Private layout //
	{
		if (m_what_was_done == FORCE_PARTY_Activ || m_what_was_done == FORCE_PRIVATE_PARTY_Active)
			result = YES;
		else
		{
			if (m_what_was_done != FORCE_CONF_Activ &&
			    m_what_was_done != FORCE_MVC_CONF_Activ &&
			    m_what_was_done != FORCE_CONTENT_Activ)
				DBGPASSERT(1);

			result = NO;
		}
	}
	else
	{
		if (m_what_was_done == FORCE_CONF_Activ ||
		    m_what_was_done == FORCE_MVC_CONF_Activ ||
		    m_what_was_done == FORCE_CONTENT_Activ ||
		    m_what_was_done == FORCE_PARTY_Activ)
			DBGPASSERT(1);

		result = NO;
	}

	return result;
}

//--------------------------------------------------------------------------
void CVidSubImage::RemovePartyForceName(void)
{
	if (m_pForcedToSee != NIL(char))
	{
		PDELETEA(m_pForcedToSee);
		m_pForcedToSee = NIL(char);
	}
}

//--------------------------------------------------------------------------
BYTE CVidSubImage::CanBeSetTo(const RequestPriority who_ask, const VideoActivities toDo, const CTaskApp* pSenderParty) const
{
	BYTE result;
	result = RelationTable(who_ask, toDo);
	if (result == YES)
	{
		// case Cancel_MCV for MCV force without MVC cap
		if (who_ask == m_who_it_asked && m_who_it_asked == PARTY_Prior &&
		    toDo == AUTO_CONF_Activ && m_what_was_done == FORCE_CONF_Activ)
		{
			if (pSenderParty == NIL(CTaskApp) ||
			    GetPartyForce() == NIL(const char))
			{
				PASSERT(1);
				return NO;
			}

			if (strcmp(GetPartyForce(), ((CParty*)pSenderParty)->GetName()) == 0)
			{
				result = YES;
			}
			else
				result = NO;
		}

		// case Cancel_MCV for MCV force with MVC cap
		if (who_ask == PARTY_Prior && m_who_it_asked == PARTY_Prior &&
		    toDo == AUTO_CONF_Activ && m_what_was_done == FORCE_MVC_CONF_Activ)
		{
			if (pSenderParty == NIL(CTaskApp))
			{
				PASSERT(1);
				return NO;
			}
			else if (GetPartyForce() == NIL(const char))
			{
				PASSERT(1);
				return NO;
			}

			if (strcmp(GetPartyForce(), ((CParty*)pSenderParty)->GetName()) != 0)
				result = NO;
		}

		// case Cancel_MCV for CONTENT force
		if (who_ask == PARTY_Prior && m_who_it_asked == PARTY_Prior &&
		    toDo == AUTO_CONF_Activ && m_what_was_done == FORCE_CONTENT_Activ)
		{
			if (pSenderParty == NIL(CTaskApp))
			{
				PASSERT(1);
				return NO;
			}
			else if (GetPartyForce() == NIL(const char))
			{
				PASSERT(1);
				return NO;
			}

			if (strcmp(GetPartyForce(), ((CParty*)pSenderParty)->GetName()) != 0)
				result = NO;
		}
	}

	return result;
}

//--------------------------------------------------------------------------
BYTE CVidSubImage::noImgSet() const
{
	//Read system flag set blank cell when the party disconnects
	CSysConfig* sysConfig = CProcessBase::GetProcess()->GetSysConfig();
	std::string key = "PRESERVE_PARTY_CELL_ON_FORCE_LAYOUT";
	BOOL isBlankCellEnabledWhenPartyDisconnect;
	sysConfig->GetBOOLDataByKey(key, isBlankCellEnabledWhenPartyDisconnect);

	if (isBlankCellEnabledWhenPartyDisconnect)
	{
		return (m_imageId == 0 && !isBlanked() && !GetForcedNotFoundCell());
	}
	else
	{
		return (m_imageId == 0 && !isBlanked());
	}
}

//--------------------------------------------------------------------------
BYTE operator==(const CVidSubImage& first, const CVidSubImage& second)
{
	BYTE rval            = YES;
	BYTE isFirstBlanked  = first.isBlanked();
	BYTE isSecondBlanked = second.isBlanked();
	BYTE isFirstEmpty    = first.noImgSet();
	BYTE isSecondEmpty   = second.noImgSet();

	if ((isFirstBlanked && isSecondBlanked) || (isFirstEmpty && isSecondEmpty)) // Both are Blanked or empty
		return YES;

	if ((isFirstBlanked != isSecondBlanked) || (isFirstEmpty != isSecondEmpty)) // Only one is Blanked/empty
		return NO;

	if (first.GetImageId() == second.GetImageId())                              // Both are not Blanked or  empty
		return YES;

	return NO;
}


//--------------------------------------------------------------------------
void CVidSubImage::SetLayoutTypeAndSizes(LayoutType newType, WORD place)
{
	if ((place == 0 && isLayoutWithSpeakerPicture(newType)) || (place == 1 && newType == CP_LAYOUT_2P8)|| (place == 1 && newType == CP_LAYOUT_2TOP_P8))
	{
		switch (newType)
		{
			case CP_LAYOUT_1P5: {
				m_sizeX = CIF_X_SIZE*2/3;
				m_sizeY = CIF_Y_SIZE*2/3;
				break;
			}
			case CP_LAYOUT_1P7: {
				m_sizeX = CIF_X_SIZE*3/4;
				m_sizeY = CIF_Y_SIZE*3/4;
				break;
			}
			case CP_LAYOUT_1P2VER: {
				m_sizeX = CIF_X_SIZE*1/2;
				m_sizeY = CIF_Y_SIZE;
				break;
			}
			case CP_LAYOUT_1P8CENT:
			case CP_LAYOUT_1P8UP:
			case CP_LAYOUT_1P8HOR_UP:
			case CP_LAYOUT_1P2HOR_UP:
			case CP_LAYOUT_1P2HOR:
			case CP_LAYOUT_1P2HOR_RIGHT_FLEX:
			case CP_LAYOUT_1P2HOR_LEFT_FLEX:
			case CP_LAYOUT_1P2HOR_UP_RIGHT_FLEX:
			case CP_LAYOUT_1P2HOR_UP_LEFT_FLEX: {
				m_sizeX = CIF_X_SIZE;
				m_sizeY = CIF_Y_SIZE*1/2;
				break;
			}
			case CP_LAYOUT_1P3HOR_UP:
			case CP_LAYOUT_1P3HOR: {
				m_sizeX = CIF_X_SIZE;
				m_sizeY = CIF_Y_SIZE*2/3;
				break;
			}
			case CP_LAYOUT_1P3VER: {
				m_sizeX = CIF_X_SIZE*2/3;
				m_sizeY = CIF_Y_SIZE;
				break;
			}
			case CP_LAYOUT_1P4HOR_UP:
			case CP_LAYOUT_1P4HOR: {
				m_sizeX = CIF_X_SIZE;
				m_sizeY = CIF_Y_SIZE*3/4;
				break;
			}
			case CP_LAYOUT_1P4VER: {
				m_sizeX = CIF_X_SIZE*3/4;
				m_sizeY = CIF_Y_SIZE;
				break;
			}
			/* for Layout_4x4 conferencing */
			case CP_LAYOUT_2P8:
			case CP_LAYOUT_2TOP_P8:
			case CP_LAYOUT_1P12: {
				m_sizeX = CIF_X_SIZE/2;
				m_sizeY = CIF_Y_SIZE/2;
				break;
			}

			case CP_LAYOUT_OVERLAY_ITP_1P4:
			case CP_LAYOUT_OVERLAY_ITP_1P3:
			case CP_LAYOUT_OVERLAY_ITP_1P2:
			case CP_LAYOUT_OVERLAY_1P3:
			case CP_LAYOUT_OVERLAY_1P2:
			case CP_LAYOUT_OVERLAY_1P1:
			{
				m_sizeX = CIF_X_SIZE/2;
				m_sizeY = CIF_X_SIZE/2;
				break;
			}

			case CP_LAYOUT_1TOP_LEFT_P8:
			{
				m_sizeX = CIF_X_SIZE/2;
				m_sizeY = (CIF_Y_SIZE/3)*2;
				break;
			}

			default:
				PASSERT(1);
				break;
		} // switch
	}
	else
	{
		SetSizesOfImage(m_sizeX, m_sizeY, newType);
	}
}

//--------------------------------------------------------------------------
void CVidSubImage::SetForceAttributes(const RequestPriority rp, const VideoActivities va)
{
	m_who_it_asked  = rp;
	m_what_was_done = va;
}

//--------------------------------------------------------------------------
BYTE CVidSubImage::IsAutoScan(void) const
{
	return (m_what_was_done == AUTO_SCAN_Active);
}
//--------------------------------------------------------------------------
void CVidSubImage::SetForcedNotFoundCell(bool val)
{
	m_forcedNotFoundCell=val;
}
//--------------------------------------------------------------------------

BYTE CVidSubImage::IsEqualOrLargeSize(const CVidSubImage& rSubImage)const
{
	WORD currentSize = m_sizeX*m_sizeY;
	WORD otherSize   = rSubImage.GetSizeX() * rSubImage.GetSizeY();
	return (currentSize >= otherSize);
}

