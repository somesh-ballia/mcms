#include "CardContent.h"
#include "SensorReadingsToCardEntry.h"
#include "IpmiSensorDescrToType.h"
#include "copy_string.h"
#include "IpmiEntitySlotIDs.h"
#include "IpmiConsts.h"
#include <string.h>
#include "ProcessBase.h"
#include "dsp_monitor_getter.h"
namespace
{
    CardContent GetControlBoardContent(char const * type, int index)
    {
        CardContent content;
        memset(&content, 0, sizeof(content));
        CopyString(content.cardType, type);
        content.slotID = CNTL_SLOT_ID;

        return content;
    }

    CardContent GetFansContent(char const * type, int index)
    {
        CardContent content;
        memset(&content, 0, sizeof(content));
        CopyString(content.cardType, type);
        content.slotID = FANS_SLOT_ID;

        return content;
    }

    CardContent GetPowerContent(char const * type, int index)
    {
        CardContent content;
        memset(&content, 0, sizeof(content));
        CopyString(content.cardType, type);
        content.slotID = PWRS_SLOT_ID;

        return content;
    }

     CardContent GetRiserContent(char const * type, int index)
    {
        CardContent content;
        memset(&content, 0, sizeof(content));
        CopyString(content.cardType, type);
        content.slotID = RISER_SLOT_ID;

        return content;
    }
    CardContent GetDspCard0Content(char const * type, int index)
    {
        CardContent content;
        memset(&content, 0, sizeof(content));
        CopyString(content.cardType, type);
        content.slotID = DSP_CARD_SLOT_ID_0;

        return content;
    }
      CardContent GetDspCard1Content(char const * type, int index)
    {
        CardContent content;
        memset(&content, 0, sizeof(content));
        CopyString(content.cardType, type);
        content.slotID =  DSP_CARD_SLOT_ID_1;

        return content;
    }
    
      CardContent GetDspCard2Content(char const * type, int index)
    {
        CardContent content;
        memset(&content, 0, sizeof(content));
        CopyString(content.cardType, type);
        content.slotID =  DSP_CARD_SLOT_ID_2;

        return content;
    }
    
    bool IsReadingFromCardGeneric(char const * cardType, char const * refType)
    {
        return 0==strcmp(cardType, refType);
    }
    bool IsReadingFromCntl(char const * cardType, char const * refType)
    {
        //(void)refType;
        return (0==strcmp(CNTL_BOARD_NAME, cardType)) && (0==strcmp(CNTL_BOARD_NAME, refType));
    }
}

typedef CardContent (*GetCardContentFunc)(char const * type, int index);
typedef bool (*IsCardReadingsPredicate)(char const * cardType, char const * refType);

struct CardContentEntry
{
    char const * cardType;
    int index;
    GetCardContentFunc func;
    IsCardReadingsPredicate isCardReadings;
};

static CardContentEntry const s_entries[] =
{
      {CNTL_BOARD_NAME, 0, GetControlBoardContent, IsReadingFromCntl}
    , {"FANS", 0, GetFansContent, IsReadingFromCardGeneric}
    , {"PWR", 0, GetPowerContent, IsReadingFromCardGeneric}
   // , {NETRA_DSP_BOARD_NAME, 1, GetDspCard0Content, IsReadingFromCardGeneric}
   // , {NETRA_DSP_BOARD_NAME, 1, GetDspCard1Content, IsReadingFromCardGeneric}
   // , {NETRA_DSP_BOARD_NAME, 1, GetDspCard2Content, IsReadingFromCardGeneric}
};

static IsCardReadingsPredicate GetCardFilterPredicate(CardContent const & card)
{
    int const count = sizeof(s_entries)/sizeof(s_entries[0]);
    for (int i=0; i<count; ++i)
    {
        CardContentEntry const & entry = s_entries[i];
        if (0==strcmp(card.cardType, entry.cardType))
        {
            return entry.isCardReadings;
        }
    }
    return IsReadingFromCardGeneric;
}

static unsigned long g_DspLocationBitmap = 0;
static int g_RtmIsdnDsp = 0;

static unsigned long GetDspLocationBitmap()
{
    if (g_DspLocationBitmap == 0)
    {
        std::string ans;
        std::string cmd = "sudo /usr/rmx1000/bin/ninja_dsp_check";

        STATUS status = SystemPipedCommand(cmd.c_str(), ans);

        if (status == STATUS_OK)
        {
            g_DspLocationBitmap = strtoul(ans.c_str(), NULL, 0);
            FPTRACE2INT(eLevelInfoNormal,"GetDspLocationBitmap: Current dsp location bitmap is :  ", g_DspLocationBitmap);
        }
        else
        {
            FPTRACE(eLevelInfoNormal,"GetDspLocationBitmap: Current dsp location bitmap failed :  ");
        }
    }  

    return g_DspLocationBitmap;
}

static int GetRtmIsdnDspStatus()
{
    unsigned long dspLocationBitmap = 0;
    int hasRtmIsdnDsp = 0;
    BOOL isDspLocated = FALSE;

    dspLocationBitmap = GetDspLocationBitmap();

    isDspLocated = (dspLocationBitmap >> 18) % 2;

    if(isDspLocated) hasRtmIsdnDsp = 1;
    else hasRtmIsdnDsp = 0;

    if(hasRtmIsdnDsp != g_RtmIsdnDsp)
    {
        FPTRACE2INT(eLevelInfoNormal,"GetRtmIsdnDspStatus: g_RtmIsdnDsp is  ", g_RtmIsdnDsp);
        g_RtmIsdnDsp = hasRtmIsdnDsp;
    }
    

    return g_RtmIsdnDsp;
}

static int McuMngrCollectCardContents(vector<CardContent> & cards)
{
    int const count = sizeof(s_entries)/sizeof(0[s_entries]);
    // Get product type 
    eProductType curProductType = CProcessBase::GetProcess()->GetProductType();
    int iCardsCount  =0;
    DSPMonitorBoardList dspcards;
      // Get DSP card number and slotID  
    if(eProductTypeNinja == curProductType)
    { 
        GetDspMonitorStatus(dspcards);
        iCardsCount = dspcards.len;
        FPTRACE2INT(eLevelInfoNormal, "McuMngrCollectCardContents DSP Card Number:",iCardsCount);
    }
    //cards.resize(count + iCardsCount);
    int i=0,j=1;
    for (; i< (int)(sizeof(s_entries)/sizeof(0[s_entries])); ++i)
    {
        CardContentEntry const & entry = s_entries[i];
        //cards[i] = (*entry.func)(entry.cardType, entry.index);
        cards.push_back((*entry.func)(entry.cardType, entry.index));
    }
    if((eProductTypeNinja == curProductType)  &&  (iCardsCount > 0))
    {
        CardContent content;
        memset(&content, 0, sizeof(content));
        //CopyString(content.cardType, NETRA_DSP_BOARD_NAME);
        for(; j <= iCardsCount;++j)
        {  
          FPTRACE2INT(eLevelInfoNormal, "McuMngrCollectCardContents DSP Card Slot id:",dspcards.status[j-1].boardId);
          // Accord the temperature to decide the card if valid
           if(0 == strncmp(dspcards.status[j-1].temperature,NDM_TEMPE_DEFAULT,4 ))  //skip invalid card  NDM_TEMPE_DEFAULT
            {
              	FPTRACE2(eLevelInfoNormal,"McuMngrCollectCardContents  Curent DSP temperature: ",dspcards.status[j-1].temperature);
               continue;
            }
           if(0 ==  dspcards.status[j-1].boardId)
           {
                 content.slotID = DSP_CARD_SLOT_ID_0;
                 CopyString(content.cardType, "DSP Card 1");
           }
           else if(1 ==  dspcards.status[j-1].boardId)
           {
                 content.slotID = DSP_CARD_SLOT_ID_1;
                 CopyString(content.cardType, "DSP Card 2");
           }
           else if(2 ==  dspcards.status[j-1].boardId)
           {
                 content.slotID = DSP_CARD_SLOT_ID_2;
                 CopyString(content.cardType, "DSP Card 3");
           }
           else
              continue;
           cards.push_back(content);
        }      
    }

    //ISDN Card
    if(eProductTypeNinja == curProductType && 1 == GetRtmIsdnDspStatus())
    {
        CardContent content;
        memset(&content, 0, sizeof(content));
        content.slotID = ISDN_CARD_SLOT_ID;
        //FPTRACE2INT(eLevelInfoNormal, "McuMngrCollectCardContents ISDN Card SlotID: ", ISDN_CARD_SLOT_ID);

        CopyString(content.cardType, ISDN_DSP_BOARD_NAME);
        cards.push_back(content);
    }
    
    vector<sensor_t> readings;
    sensor_read(readings);
    for (int i=0; i<(int)cards.size(); ++i)
    {
        SensorReadingsToCardEntry(cards[i], readings);
    }
    
    return count + iCardsCount;
}

CollectCardContentsType CollectCardContents = McuMngrCollectCardContents;

IpmiCardStatus GetCardStatus(sensor_t const & reading)
{
    if ((0==strcasecmp("ok", reading.Status)) || ((!strncmp(reading.SensorName,"PS",2)) && (0==strncasecmp("0x01", reading.Status, 4))))
        return IPMI_CARD_STATUS_NORMAL;
    else if (0==strcmp("", reading.Status))
        return IPMI_CARD_STATUS_NORMAL;
    else if ((0==strcasecmp("na", reading.Status)) || ((!strncmp(reading.SensorName,"PS",2)) && (0!=strncasecmp("0x01", reading.Status, 4))))
        return IPMI_CARD_STATUS_MAJOR;
    else return IPMI_CARD_STATUS_NORMAL;
}

IpmiMeteorStatus GetMeteorStatus(sensor_t const & reading)
{
    if ((reading.CurrentVal>=reading.LowerNonCritical) && (reading.CurrentVal<=reading.UpperNonCritical))
        return IPMI_METEOR_STATUS_NORMAL;
    else if ((reading.CurrentVal>=reading.LowerCritical) && (reading.CurrentVal<=reading.UpperCritical))
        return IPMI_METEOR_STATUS_MAJOR;
    else
        return IPMI_METEOR_STATUS_CRITICAL;
}

void UpdateCardStatus(CardContent & card, sensor_s const & reading, MeteorType type)
{
    IpmiCardStatus const cardStatus = GetCardStatus(reading);
    IpmiMeteorStatus const meteorStatus = GetMeteorStatus(reading);
    if (cardStatus>card.status)
    {
        card.status = cardStatus;
    }

    if (meteorStatus>IPMI_METEOR_STATUS_NORMAL)
    {
        if (IPMI_CARD_STATUS_NORMAL==card.status)
        {
            card.status = IPMI_CARD_STATUS_MAJOR;
        }

        switch (type)
        {
        case METEOR_TEMPERATURE:
            card.temperature = meteorStatus;
            break;
        case METEOR_VOLTAGE:
            card.voltage = meteorStatus;
            break;
        default:
            break;
        }
    }	

	//BRIDGE-12211, the Card status is Major when one of its unit is faulty
/*   //Disabled by BRIDGE-13675
	const WORD slotID = card.slotID;
	if(strstr(card.cardType, "DSP Card"))
	{
		if(DSP_CARD_SLOT_ID_0 <= slotID && slotID <= DSP_CARD_SLOT_ID_2)
		{
			DSPMonitorDspList dsp_unit_list;
			GetDspMonitorStatus(dsp_unit_list,slotID - DSP_CARD_SLOT_ID_0);
			for(int i= 0 ; i < dsp_unit_list.len; ++i)
			{
				if(dsp_unit_list.status[i].isFaulty)
				{
					card.status = IPMI_CARD_STATUS_MAJOR;
					break;
				}
			}
		}
	}
*/
}

void SensorReadingsToCardEntry(CardContent & card, vector<sensor_t> const & readings)
{
    vector<sensor_t> filteredReadings;
    vector<CardAndMeteorType> types;

    {
        IsCardReadingsPredicate predicate = GetCardFilterPredicate(card);
        int const count = readings.size();
        for (int i=0; i<count; ++i)
        {
            sensor_t const & entry = readings[i];
            CardAndMeteorType const type = GetCardAndMeteorType(entry.SensorName);
            if (0==strcmp("", type.cardType)) continue;
            if (!(*predicate)(card.cardType, type.cardType)) continue;

            filteredReadings.push_back(entry);
            types.push_back(type);
        }
    }

    {
        int const count = types.size();
        for (int i=0; i<count; ++i)
        {
            UpdateCardStatus(card, filteredReadings[i], types[i].meteorType);
        }
    }
}

