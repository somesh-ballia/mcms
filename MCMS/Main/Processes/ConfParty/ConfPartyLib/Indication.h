#ifndef _CIndicationIcon_H_
#define _CIndicationIcon_H_

#include "VideoStructs.h"
#include "PObject.h"
#include <memory.h>

inline iconTypeEnum& operator ++(iconTypeEnum& e)
{
	return enum_increment(e, eIconNetworkQuality, MAX_NUM_TYPES_OF_ICON);
}

// a simple C++ wrapper around API's ICON_ATTR_S structure
class CIndicationIcon : public CPObject {
	CLASS_TYPE_1(CIndicationIcon, CPObject)

public:

	virtual const char* NameOf() const { return "CIndicationIcon"; }

	explicit CIndicationIcon(ICON_ATTR_S& attr) : icon_(attr) {}

	bool             isActive() const     { return icon_.bActive; }
	iconLocationEnum location() const     { return static_cast<iconLocationEnum>(icon_.nLocation); }
	size_t           indexInStrip() const { return icon_.nIndexIntoStrip; }
	int              data() const         { return icon_.nIconData; }

	void             off()                { icon_.bActive = false; icon_.nIconData = 0;}
	bool             on(int data = 0)
	{
		bool bChanged = (data != icon_.nIconData);
		icon_.nIconData = data;
		icon_.bActive = (data != 0);
		return bChanged;
	}

	void             active()             {icon_.bActive = true;}

	CIndicationIcon& location(iconLocationEnum iconLocation) { icon_.nLocation = iconLocation; return *this; }
	CIndicationIcon& indexInStrip(size_t index)              { icon_.nIndexIntoStrip = index; return *this; }

private:

	ICON_ATTR_S& icon_;
};

// a simple C++ wrapper around the API's ICONS_DISPLAY_S structure
class CLayoutIndications : public CPObject {
	CLASS_TYPE_1(CLayoutIndications, CPObject)

public:

	virtual const char* NameOf() const { return "CLayoutIndications"; }

	CLayoutIndications() { memset(&indications_, 0, sizeof(indications_)); }

	CLayoutIndications& locations(iconLocationEnum selfLocation, iconLocationEnum cellLocation, WORD stripIndex[MAX_NUM_TYPES_OF_ICON])
	{
		for (iconTypeEnum i = eIconNetworkQuality; i < MAX_NUM_TYPES_OF_ICON; ++i)
		{
			indication(i).location(selfLocation).indexInStrip(stripIndex[i]);

			for (size_t c = 0; c < MAX_NUMBER_OF_CELLS_IN_LAYOUT; ++c)
			{
				indication(i, c).location(cellLocation).indexInStrip(i);
			}
		}

		return *this;
	}

	
	void dumpIndications()
	{
		CLargeString cstr;
		cstr << "\n ------- Layout Icon Info ---------- : ";
		for(int i = 0; i < MAX_NUM_TYPES_OF_ICON; i++)
		{
			if(eIconNetworkQuality == i)
				cstr << "\n *** Network Quality Icon:";
			else if(eIconRecording == i)
				cstr << "\n *** Recording Icon:";
			else if(eIconAudioPartiesCount == i)
				cstr << "\n *** Audio Participants Icon:";
			
				cstr <<"\n bActive: " << indications_.atLayoutIconParams[i].bActive;
				cstr <<"	nLocation: " << indications_.atLayoutIconParams[i].nLocation;
				cstr <<"	nIndexIntoStrip: " << indications_.atLayoutIconParams[i].nIndexIntoStrip;
				cstr <<"	nIconData: " << indications_.atLayoutIconParams[i].nIconData;	
		}
		PTRACE2(eLevelInfoNormal,"CLayoutIndications::dumpIndications ",cstr.GetString());
	}

	class CellIndex {
	public:

		CellIndex(size_t index)
			: index_(index)
		{
			if (index >= MAX_NUMBER_OF_CELLS_IN_LAYOUT)
				throw int(index);
		}

	public:
		operator size_t() const { return index_; }

	private:

		size_t index_;
	};

	CIndicationIcon indication(iconTypeEnum iconType, CellIndex cell) { return CIndicationIcon(get(iconType, cell)); }
	CIndicationIcon indication(iconTypeEnum iconType) { return CIndicationIcon(get(iconType)); }

	operator const ICONS_DISPLAY_S&() const { return indications_; }

private:

	ICONS_DISPLAY_S indications_;

private:

	ICON_ATTR_S& get(iconTypeEnum iconType) { return indications_.atLayoutIconParams[iconType]; }
	ICON_ATTR_S& get(iconTypeEnum iconType, size_t cell) { return indications_.atCellIconParams[cell][iconType]; }
};

#endif // _CIndicationIcon_H_
