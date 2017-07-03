#include "IpmiSensorDescrToType.h"
#include "IpmiSensorEnums.h"
#include "FaultsDefines.h"
#include "sensor_read.h"
#include "IpmiConsts.h"
#include <string.h>
#include "SharedDefines.h"
#include "IpmiEntitySlotIDs.h"

DescrSensorEntity const s_items[] =
{
      { "CPU Temp", IPMI_SENSOR_TYPE_TEMPERATURE, IPMI_ENTITY_TYPE_PROCESSOR, 50.0, 0, CNTL_BOARD_NAME, METEOR_TEMPERATURE, AA_TEMPERATURE_MAJOR_PROBLEM, 2, EVENT_READ_TYPE_COLOR 
        , {(float)-11.0, (float)-8.0, (float)-5.0, (float)80.0, (float)91.0, (float)100.0}
      }
    , { "HDD Temp", IPMI_SENSOR_TYPE_TEMPERATURE, IPMI_ENTITY_TYPE_DISK, 30.0, 1, CNTL_BOARD_NAME, METEOR_TEMPERATURE, AA_TEMPERATURE_MAJOR_PROBLEM, 2, EVENT_READ_TYPE_COLOR 
        , {(float)-9.0, (float)-7.0, (float)-5.0, (float)55.0, (float)85.0, (float)100.0}
      }
    , { "System Temp", IPMI_SENSOR_TYPE_TEMPERATURE, IPMI_ENTITY_TYPE_SYSTEM_BOARD, 31.0, 2, CNTL_BOARD_NAME, METEOR_TEMPERATURE, AA_TEMPERATURE_MAJOR_PROBLEM, 2, EVENT_READ_TYPE_COLOR 
        , {(float)-9.0, (float)-7.0, (float)-5.0, (float)75.0, (float)77.0, (float)79.0}
      }
    , { "Peripheral Temp", IPMI_SENSOR_TYPE_TEMPERATURE, IPMI_ENTITY_TYPE_PERIPHERAL, 37.0, 3, CNTL_BOARD_NAME, METEOR_TEMPERATURE, AA_TEMPERATURE_MAJOR_PROBLEM, 2, EVENT_READ_TYPE_COLOR 
        , {(float)-9.0, (float)-7.0, (float)-5.0, (float)75.0, (float)77.0, (float)79.0}
      }
     #if 0
    , { "PCH Temp", IPMI_SENSOR_TYPE_TEMPERATURE, IPMI_ENTITY_TYPE_PROCESSOR_IO_MODULE, 50.0, 4, CNTL_BOARD_NAME, METEOR_NONE, AA_TEMPERATURE_MAJOR_PROBLEM, 2, EVENT_READ_TYPE_COLOR 
        , {(float)-11.0, (float)-8.0, (float)-5.0, (float)90.0, (float)95.0, (float)100.0}
      }
     , { "FAN1", IPMI_SENSOR_TYPE_FAN, IPMI_ENTITY_TYPE_SYSTEM_CHASSIS, 3200.0, 5, "FANS", METEOR_NONE, AA_FAN_MAJOR_PROBLEM, 2, EVENT_READ_TYPE_COLOR 
        , {(float)300.00, (float)450.00, (float)600.000, (float)18975.0, (float)19050.0, (float)19125.0}
      }
    , { "FAN2", IPMI_SENSOR_TYPE_FAN, IPMI_ENTITY_TYPE_SYSTEM_CHASSIS, 3200.0, 6, "FANS", METEOR_NONE, AA_FAN_MAJOR_PROBLEM, 2, EVENT_READ_TYPE_COLOR 
        , {(float)300.00, (float)450.00, (float)600.000, (float)18975.0, (float)19050.0, (float)19125.0}
      }
     #endif
    , { "FAN3", IPMI_SENSOR_TYPE_FAN, IPMI_ENTITY_TYPE_SYSTEM_CHASSIS, 3200.0, 5, "FANS", METEOR_NONE, AA_FAN_MAJOR_PROBLEM, 2, EVENT_READ_TYPE_COLOR 
        , {(float)300.00, (float)450.00, (float)600.000, (float)18975.0, (float)19050.0, (float)19125.0}
      }
    , { "FAN4", IPMI_SENSOR_TYPE_FAN, IPMI_ENTITY_TYPE_SYSTEM_CHASSIS, 3200.0, 6, "FANS", METEOR_NONE, AA_FAN_MAJOR_PROBLEM, 2, EVENT_READ_TYPE_COLOR 
        , {(float)300.00, (float)450.00, (float)600.000, (float)18975.0, (float)19050.0, (float)19125.0}
      }
    , { "FAN5", IPMI_SENSOR_TYPE_FAN, IPMI_ENTITY_TYPE_SYSTEM_CHASSIS, 3200.0, 7, "FANS", METEOR_NONE, AA_FAN_MAJOR_PROBLEM, 2, EVENT_READ_TYPE_COLOR 
        , {(float)300.00, (float)450.00, (float)600.000, (float)18975.0, (float)19050.0, (float)19125.0}
      }
    , { "VTT", IPMI_SENSOR_TYPE_VOLTAGE, IPMI_ENTITY_TYPE_PROCESSOR_FR_BUS, 1.1, 10, CNTL_BOARD_NAME, METEOR_NONE, AA_VOLTAGE_PROBLEM, 1, EVENT_READ_TYPE_COLOR 
        , {(float)0.816, (float)0.864, (float)0.912, (float)1.344, (float)1.392, (float)1.440}
      }
    , { "Vcore", IPMI_SENSOR_TYPE_VOLTAGE, IPMI_ENTITY_TYPE_PROCESSOR, 0.85, 11, CNTL_BOARD_NAME, METEOR_VOLTAGE, AA_VOLTAGE_PROBLEM, 1, EVENT_READ_TYPE_COLOR 
        , {(float)0.480, (float)0.512, (float)0.544, (float)1.488, (float)1.520, (float)1.552}
      }
    , { "VDIMM AB", IPMI_SENSOR_TYPE_VOLTAGE, IPMI_ENTITY_TYPE_MEMORY_MODULE, 1.35, 12, CNTL_BOARD_NAME, METEOR_VOLTAGE, AA_VOLTAGE_PROBLEM, 1, EVENT_READ_TYPE_COLOR 
        , {(float)1.104, (float)1.152, (float)1.200, (float)1.648, (float)1.696, (float)1.744}
      }
    , { "VDIMM CD", IPMI_SENSOR_TYPE_VOLTAGE, IPMI_ENTITY_TYPE_MEMORY_MODULE, 1.35, 13, CNTL_BOARD_NAME, METEOR_VOLTAGE, AA_VOLTAGE_PROBLEM, 1, EVENT_READ_TYPE_COLOR 
        , {(float)1.104, (float)1.152, (float)1.200, (float)1.648, (float)1.696, (float)1.744}
      }
    , { "+1.1 V", IPMI_SENSOR_TYPE_VOLTAGE, IPMI_ENTITY_TYPE_POWER_SUPPLY_MODULE, 1.1, 14, CNTL_BOARD_NAME, METEOR_VOLTAGE, AA_VOLTAGE_PROBLEM, 1, EVENT_READ_TYPE_COLOR 
        , {(float)0.880, (float)0.928, (float)0.976, (float)1.216, (float)1.264, (float)1.312}
      }
    , { "+1.5 V", IPMI_SENSOR_TYPE_VOLTAGE, IPMI_ENTITY_TYPE_POWER_SUPPLY_MODULE, 1.5, 15, CNTL_BOARD_NAME, METEOR_VOLTAGE, AA_VOLTAGE_PROBLEM, 1, EVENT_READ_TYPE_COLOR 
        , {(float)1.248, (float)1.296, (float)1.344, (float)1.648, (float)1.696, (float)1.744}
      }
    , { "3.3V", IPMI_SENSOR_TYPE_VOLTAGE, IPMI_ENTITY_TYPE_POWER_SUPPLY_MODULE, 3.3, 16, CNTL_BOARD_NAME, METEOR_VOLTAGE, AA_VOLTAGE_PROBLEM, 1, EVENT_READ_TYPE_COLOR 
        , {(float)2.640, (float)2.784, (float)2.928, (float)3.648, (float)3.792, (float)3.936}
      }
    , { "+3.3VSB", IPMI_SENSOR_TYPE_VOLTAGE, IPMI_ENTITY_TYPE_POWER_SUPPLY_MODULE, 3.3, 17, CNTL_BOARD_NAME, METEOR_VOLTAGE, AA_VOLTAGE_PROBLEM, 1, EVENT_READ_TYPE_COLOR 
        , {(float)2.640, (float)2.784, (float)2.928, (float)3.648, (float)3.792, (float)3.936}
      }
    , { "5V", IPMI_SENSOR_TYPE_VOLTAGE, IPMI_ENTITY_TYPE_POWER_SUPPLY_MODULE, 5.0, 18, CNTL_BOARD_NAME, METEOR_VOLTAGE, AA_VOLTAGE_PROBLEM, 1, EVENT_READ_TYPE_COLOR 
        , {(float)4.096, (float)4.288, (float)4.480, (float)5.504, (float)5.696, (float)6.912}
      }
    , { "+5VSB", IPMI_SENSOR_TYPE_VOLTAGE, IPMI_ENTITY_TYPE_POWER_SUPPLY_MODULE, 5.0, 19, CNTL_BOARD_NAME, METEOR_VOLTAGE, AA_VOLTAGE_PROBLEM, 1, EVENT_READ_TYPE_COLOR 
        , {(float)4.096, (float)4.288, (float)4.480, (float)5.504, (float)5.696, (float)6.912}
      }
    , { "12V", IPMI_SENSOR_TYPE_VOLTAGE, IPMI_ENTITY_TYPE_POWER_SUPPLY_MODULE, 12.0, 20, CNTL_BOARD_NAME, METEOR_VOLTAGE, AA_VOLTAGE_PROBLEM, 1, EVENT_READ_TYPE_COLOR 
        , {(float)10.176, (float)10.494, (float)10.812, (float)13.250, (float)13.568, (float)13.886}
      }
    , { "VBAT", IPMI_SENSOR_TYPE_VOLTAGE, IPMI_ENTITY_TYPE_BATTERY, 3.3, 21, CNTL_BOARD_NAME, METEOR_NONE, AA_VOLTAGE_PROBLEM, 1, EVENT_READ_TYPE_COLOR 
        , {(float)2.400, (float)2.544, (float)2.688, (float)3.312, (float)3.455, (float)3.600}
      }
#if 0
    , { "Chassis Intru", IPMI_SENSOR_TYPE_CHASSIS, IPMI_ENTITY_TYPE_SYSTEM_CHASSIS, 0, 22, CNTL_BOARD_NAME, METEOR_NONE, AA_OTHER_PROBLEM, 1, EVENT_READ_TYPE_COLOR 
        , {(float)-9.0, (float)-7.0, (float)-5.0, (float)55.0, (float)85.0, (float)100.0}
      }
    , { "PS Status", IPMI_SENSOR_TYPE_POWER_UNIT, IPMI_ENTITY_TYPE_CHASSIS_BACK_PANEL, 0, 23, "PWR", METEOR_NONE, AA_OTHER_PROBLEM, 1, EVENT_READ_TYPE_COLOR 
        , {(float)-9.0, (float)-7.0, (float)-5.0, (float)55.0, (float)85.0, (float)100.0}
      }
#endif
    , { "CPU Usage", IPMI_SENSOR_TYPE_SYSTEM_EVENT, IPMI_ENTITY_TYPE_SYSTEM_BOARD, 0.0, 24, CNTL_BOARD_NAME, METEOR_NONE, SYSTEM_CPU_USAGE_ALERT, 1, EVENT_READ_TYPE_NO_COLOR 
        , {(float)0.0, (float)1.0, (float)2.0, (float)950.0, (float)990.0, (float)1001.0}
      }
    , { "Memory Usage", IPMI_SENSOR_TYPE_SYSTEM_EVENT, IPMI_ENTITY_TYPE_SYSTEM_BOARD, 0.0, 25, CNTL_BOARD_NAME, METEOR_NONE, LOW_SYSTEM_MEMORY_ALERT, 1, EVENT_READ_TYPE_NO_COLOR 
        , {(float)0.0, (float)1.0, (float)2.0, (float)950.0, (float)990.0, (float)1001.0}
      }

    , { "Inlet Temp", IPMI_SENSOR_TYPE_TEMPERATURE, IPMI_ENTITY_TYPE_SYSTEM_CHASSIS, 25.0, 55, CNTL_BOARD_NAME, METEOR_TEMPERATURE, AA_TEMPERATURE_MAJOR_PROBLEM, 2, EVENT_READ_TYPE_COLOR 
        , {(float)-12.0, (float)-7.0, (float)3.0, (float)42.0, (float)47.0, (float)75.0}
      }
    , { "Exhaust Temp", IPMI_SENSOR_TYPE_TEMPERATURE, IPMI_ENTITY_TYPE_SYSTEM_CHASSIS, 35.0, 56, CNTL_BOARD_NAME, METEOR_TEMPERATURE, AA_TEMPERATURE_MAJOR_PROBLEM, 2, EVENT_READ_TYPE_COLOR 
        , {(float)1.0, (float)3.0, (float)8.0, (float)70.0, (float)75.0, (float)100.0}
      }
    , { "Fan1A RPM", IPMI_SENSOR_TYPE_FAN, IPMI_ENTITY_TYPE_SYSTEM_CHASSIS, 3200.0, 57, "FANS", METEOR_NONE, AA_FAN_MAJOR_PROBLEM, 2, EVENT_READ_TYPE_COLOR 
        , {(float)100.0, (float)720.0, (float)840.0, (float)18975.0, (float)19050.0, (float)19125.0}
      }
    , { "Fan2A RPM", IPMI_SENSOR_TYPE_FAN, IPMI_ENTITY_TYPE_SYSTEM_CHASSIS, 3200.0, 58, "FANS", METEOR_NONE, AA_FAN_MAJOR_PROBLEM, 2, EVENT_READ_TYPE_COLOR 
        , {(float)100.0, (float)720.0, (float)840.0, (float)18975.0, (float)19050.0, (float)19125.0}
      }
    , { "Fan3A RPM", IPMI_SENSOR_TYPE_FAN, IPMI_ENTITY_TYPE_SYSTEM_CHASSIS, 3200.0, 59, "FANS", METEOR_NONE, AA_FAN_MAJOR_PROBLEM, 2, EVENT_READ_TYPE_COLOR 
        , {(float)100.0, (float)720.0, (float)840.0, (float)18975.0, (float)19050.0, (float)19125.0}
      }
    , { "Fan4A RPM", IPMI_SENSOR_TYPE_FAN, IPMI_ENTITY_TYPE_SYSTEM_CHASSIS, 3200.0, 60, "FANS", METEOR_NONE, AA_FAN_MAJOR_PROBLEM, 2, EVENT_READ_TYPE_COLOR 
        , {(float)100.0, (float)720.0, (float)840.0, (float)18975.0, (float)19050.0, (float)19125.0}
      }
    , { "Fan5A RPM", IPMI_SENSOR_TYPE_FAN, IPMI_ENTITY_TYPE_SYSTEM_CHASSIS, 3200.0, 61, "FANS", METEOR_NONE, AA_FAN_MAJOR_PROBLEM, 2, EVENT_READ_TYPE_COLOR 
        , {(float)100.0, (float)720.0, (float)840.0, (float)18975.0, (float)19050.0, (float)19125.0}
      }
    , { "Fan6A RPM", IPMI_SENSOR_TYPE_FAN, IPMI_ENTITY_TYPE_SYSTEM_CHASSIS, 3200.0, 62, "FANS", METEOR_NONE, AA_FAN_MAJOR_PROBLEM, 2, EVENT_READ_TYPE_COLOR 
        , {(float)100.0, (float)720.0, (float)840.0, (float)18975.0, (float)19050.0, (float)19125.0}
      }
    , { "Fan7A RPM", IPMI_SENSOR_TYPE_FAN, IPMI_ENTITY_TYPE_SYSTEM_CHASSIS, 3200.0, 63, "FANS", METEOR_NONE, AA_FAN_MAJOR_PROBLEM, 2, EVENT_READ_TYPE_COLOR 
        , {(float)100.0, (float)720.0, (float)840.0, (float)18975.0, (float)19050.0, (float)19125.0}
      }
    , { "Fan1B RPM", IPMI_SENSOR_TYPE_FAN, IPMI_ENTITY_TYPE_SYSTEM_CHASSIS, 3200.0, 64, "FANS", METEOR_NONE, AA_FAN_MAJOR_PROBLEM, 2, EVENT_READ_TYPE_COLOR 
        , {(float)100.0, (float)720.0, (float)840.0, (float)18975.0, (float)19050.0, (float)19125.0}
      }
    , { "Fan2B RPM", IPMI_SENSOR_TYPE_FAN, IPMI_ENTITY_TYPE_SYSTEM_CHASSIS, 3200.0, 65, "FANS", METEOR_NONE, AA_FAN_MAJOR_PROBLEM, 2, EVENT_READ_TYPE_COLOR 
        , {(float)100.0, (float)720.0, (float)840.0, (float)18975.0, (float)19050.0, (float)19125.0}
      }
    , { "Fan3B RPM", IPMI_SENSOR_TYPE_FAN, IPMI_ENTITY_TYPE_SYSTEM_CHASSIS, 3200.0, 66, "FANS", METEOR_NONE, AA_FAN_MAJOR_PROBLEM, 2, EVENT_READ_TYPE_COLOR 
        , {(float)100.0, (float)720.0, (float)840.0, (float)18975.0, (float)19050.0, (float)19125.0}
      }
    , { "Fan4B RPM", IPMI_SENSOR_TYPE_FAN, IPMI_ENTITY_TYPE_SYSTEM_CHASSIS, 3200.0, 67, "FANS", METEOR_NONE, AA_FAN_MAJOR_PROBLEM, 2, EVENT_READ_TYPE_COLOR 
        , {(float)100.0, (float)720.0, (float)840.0, (float)18975.0, (float)19050.0, (float)19125.0}
      }
    , { "Fan5B RPM", IPMI_SENSOR_TYPE_FAN, IPMI_ENTITY_TYPE_SYSTEM_CHASSIS, 3200.0, 68, "FANS", METEOR_NONE, AA_FAN_MAJOR_PROBLEM, 2, EVENT_READ_TYPE_COLOR 
        , {(float)100.0, (float)720.0, (float)840.0, (float)18975.0, (float)19050.0, (float)19125.0}
      }
    , { "Fan6B RPM", IPMI_SENSOR_TYPE_FAN, IPMI_ENTITY_TYPE_SYSTEM_CHASSIS, 3200.0, 69, "FANS", METEOR_NONE, AA_FAN_MAJOR_PROBLEM, 2, EVENT_READ_TYPE_COLOR 
        , {(float)100.0, (float)720.0, (float)840.0, (float)18975.0, (float)19050.0, (float)19125.0}
      }
    , { "Fan7B RPM", IPMI_SENSOR_TYPE_FAN, IPMI_ENTITY_TYPE_SYSTEM_CHASSIS, 3200.0, 70, "FANS", METEOR_NONE, AA_FAN_MAJOR_PROBLEM, 2, EVENT_READ_TYPE_COLOR 
        , {(float)100.0, (float)720.0, (float)840.0, (float)18975.0, (float)19050.0, (float)19125.0}
      }
    , { "Pwr Consumption", IPMI_SENSOR_TYPE_POWER_UNIT, IPMI_ENTITY_TYPE_SYSTEM_CHASSIS, 100.0, 71, "PWR", METEOR_NONE, AA_PWR_MAJOR_PROBLEM, 2, EVENT_READ_TYPE_COLOR 
        , {(float)5.0, (float)10.0, (float)15.0, (float)896.0, (float)980.0, (float)1124.0}
      }
    , { "Current 1", IPMI_SENSOR_TYPE_CURRENT, IPMI_ENTITY_TYPE_SYSTEM_CHASSIS, 0.40, 72, "PWR", METEOR_NONE, AA_PWR_MAJOR_PROBLEM, 2, EVENT_READ_TYPE_COLOR 
        , {(float)0, (float)0, (float)0, (float)5.0, (float)10.0, (float)15.0}
      }
    , { "Current 2", IPMI_SENSOR_TYPE_CURRENT, IPMI_ENTITY_TYPE_SYSTEM_CHASSIS, 0.40, 73, "PWR", METEOR_NONE, AA_PWR_MAJOR_PROBLEM, 2, EVENT_READ_TYPE_COLOR 
        , {(float)0, (float)0, (float)0, (float)5.0, (float)10.0, (float)15.0}
      }
    , { "Voltage 1", IPMI_SENSOR_TYPE_VOLTAGE, IPMI_ENTITY_TYPE_SYSTEM_CHASSIS, 220.0, 74, "PWR", METEOR_VOLTAGE, AA_PWR_MAJOR_PROBLEM, 2, EVENT_READ_TYPE_COLOR 
        , {(float)25.0, (float)40.0, (float)55.0, (float)480.0, (float)540.0, (float)600.0}
      }
    , { "Voltage 2", IPMI_SENSOR_TYPE_VOLTAGE, IPMI_ENTITY_TYPE_SYSTEM_CHASSIS, 220.0, 75, "PWR", METEOR_VOLTAGE, AA_PWR_MAJOR_PROBLEM, 2, EVENT_READ_TYPE_COLOR 
        , {(float)25.0, (float)40.0, (float)55.0, (float)480.0, (float)540.0, (float)600.0}
      }
     , { "DSP 1 Voltage", IPMI_SENSOR_TYPE_VOLTAGE, IPMI_ENTITY_TYPE_PROCESSOR_BOARD, 220.0, 100, "DSP Card 1", METEOR_VOLTAGE, AA_PWR_MAJOR_PROBLEM, 2, EVENT_READ_TYPE_COLOR 
        , {(float)0, (float)0, (float)0, (float)480.0, (float)540.0, (float)600.0}
     }
     ,{ "DSP 1 Temp", IPMI_SENSOR_TYPE_TEMPERATURE, IPMI_ENTITY_TYPE_PROCESSOR_BOARD, 30.0, 101,  "DSP Card 1", METEOR_TEMPERATURE, AA_TEMPERATURE_MAJOR_PROBLEM, 2, EVENT_READ_TYPE_COLOR 
        , {(float)-15.0, (float)-10.0, (float)-5.0, (float)75.0, (float)85.0, (float)100.0}
     }
       , { "DSP 2 Voltage", IPMI_SENSOR_TYPE_VOLTAGE, IPMI_ENTITY_TYPE_PROCESSOR_BOARD, 220.0, 102, "DSP Card 2", METEOR_VOLTAGE, AA_PWR_MAJOR_PROBLEM, 2, EVENT_READ_TYPE_COLOR 
        , {(float)0, (float)0, (float)0, (float)480.0, (float)540.0, (float)600.0}
     }
     ,{ "DSP 2 Temp", IPMI_SENSOR_TYPE_TEMPERATURE, IPMI_ENTITY_TYPE_PROCESSOR_BOARD, 30.0, 103,  "DSP Card 2", METEOR_TEMPERATURE, AA_TEMPERATURE_MAJOR_PROBLEM, 2, EVENT_READ_TYPE_COLOR 
        , {(float)-15.0, (float)-10.0, (float)-5.0, (float)75.0, (float)85.0, (float)100.0}
     }
       , { "DSP 3 Voltage", IPMI_SENSOR_TYPE_VOLTAGE, IPMI_ENTITY_TYPE_PROCESSOR_BOARD, 220.0, 104, "DSP Card 3", METEOR_VOLTAGE, AA_PWR_MAJOR_PROBLEM, 2, EVENT_READ_TYPE_COLOR 
        , {(float)0, (float)0, (float)0, (float)480.0, (float)540.0, (float)600.0}
     }
     ,{ "DSP 3 Temp", IPMI_SENSOR_TYPE_TEMPERATURE, IPMI_ENTITY_TYPE_PROCESSOR_BOARD, 30.0, 105,  "DSP Card 3", METEOR_TEMPERATURE, AA_TEMPERATURE_MAJOR_PROBLEM, 2, EVENT_READ_TYPE_COLOR 
        , {(float)-15.0, (float)-10.0, (float)-5.0, (float)75.0, (float)85.0, (float)100.0}
     }
	 ,{ "PS1 Status", IPMI_SENSOR_TYPE_POWER_SUPPLY, IPMI_ENTITY_TYPE_CHASSIS_BACK_PANEL, 0, 23, "PWR", METEOR_NONE, AA_PWR_MAJOR_PROBLEM, 2, EVENT_READ_TYPE_COLOR 
        , {(float)-9.0, (float)-7.0, (float)-5.0, (float)55.0, (float)85.0, (float)100.0}
      }
	 ,{ "PS2 Status", IPMI_SENSOR_TYPE_POWER_SUPPLY, IPMI_ENTITY_TYPE_CHASSIS_BACK_PANEL, 0, 23, "PWR", METEOR_NONE, AA_PWR_MAJOR_PROBLEM, 2, EVENT_READ_TYPE_COLOR 
        , {(float)-9.0, (float)-7.0, (float)-5.0, (float)55.0, (float)85.0, (float)100.0}
     }
	 ,{ "PS Status", IPMI_SENSOR_TYPE_POWER_SUPPLY, IPMI_ENTITY_TYPE_CHASSIS_BACK_PANEL, 0, 23, "PWR", METEOR_NONE, AA_PWR_MAJOR_PROBLEM, 2, EVENT_READ_TYPE_COLOR 
        , {(float)-9.0, (float)-7.0, (float)-5.0, (float)55.0, (float)85.0, (float)100.0}
     }
};

int IsDescrMatched(char const * lhs, char const * rhs)
{
    return 0==strcasecmp(lhs, rhs);
}

int IpmiSensorDescrToEntityType(char const * descr)
{
    size_t i=0;
    for (i=0; i<sizeof(s_items)/sizeof(0[s_items]); ++i)
    {
        if (IsDescrMatched(descr, s_items[i].descr))
        {
            return s_items[i].entityType;
        }
    }
    
    return IPMI_ENTITY_TYPE_UNSPECIFIED;
}

int IpmiSensorDescrToSensorType(char const * descr)
{
	size_t i=0;
    for (; i<sizeof(s_items)/sizeof(0[s_items]); ++i)
    {
        if (IsDescrMatched(descr, s_items[i].descr))
        {
            return s_items[i].sensorType;
        }
    }
    
    return 0;;
}

int IpmiSensorDescrToSensorNumber(char const * descr)
{
	size_t i=0;
    for (; i<sizeof(s_items)/sizeof(0[s_items]); ++i)
    {
        if (IsDescrMatched(descr, s_items[i].descr))
        {
            return s_items[i].index;
        }
    }
    
    return 0;
}

float IpmiSensorDescrToNominalVal(char const * descr)
{
	size_t i=0;
    for (; i<sizeof(s_items)/sizeof(0[s_items]); ++i)
    {
        if (IsDescrMatched(descr, s_items[i].descr))
        {
            return s_items[i].nominalVal;
        }
    }
    
    return 0.0;
}

int CardTypeToSlotID(char const * cardType)
{
    int slotID = -1;
    if(0 == strcmp(cardType, CNTL_BOARD_NAME))  slotID = CNTL_SLOT_ID;
    else if(0 == strcmp(cardType, "FANS"))  slotID = FANS_SLOT_ID;
    else if(0 == strcmp(cardType, "PWR"))  slotID = PWRS_SLOT_ID;
    else if(0 == strcmp(cardType, NETRA_DSP_BOARD1_NAME))  slotID = DSP_CARD_SLOT_ID_0;
    else if(0 == strcmp(cardType, NETRA_DSP_BOARD2_NAME))  slotID = DSP_CARD_SLOT_ID_1;
    else if(0 == strcmp(cardType, NETRA_DSP_BOARD3_NAME))  slotID = DSP_CARD_SLOT_ID_2;
    else if(0 == strcmp(cardType, NETRA_RTM_ISDN_NAME))  slotID = ISDN_CARD_SLOT_ID;
    
    return slotID;
}

int IpmiSensorDescrToSlotID(char const * descr)
{
	size_t i=0;
    for (; i<sizeof(s_items)/sizeof(0[s_items]); ++i)
    {
        if (IsDescrMatched(descr, s_items[i].descr))
        {
            return CardTypeToSlotID(s_items[i].cardType);
        }
    }
    
    return -1;
}


BOOL IsSensorValid(char const * descr, DescrSensorEntity * ref)
{
	size_t i=0;
    for (; i<sizeof(s_items)/sizeof(0[s_items]); ++i)
    {
        if (IsDescrMatched(descr, s_items[i].descr))
        {
            (*ref) = s_items[i];
            return TRUE;
        }
    }

    return FALSE;
}

CardAndMeteorType GetCardAndMeteorType(char const * desc)
{
    struct CardAndMeteorType result = { "", METEOR_NONE };
	size_t i=0;
    for (; i<sizeof(s_items)/sizeof(0[s_items]); ++i)
    {
        if (IsDescrMatched(desc, s_items[i].descr))
        {
            result.cardType = s_items[i].cardType;
            result.meteorType = s_items[i].meteorType;
        }
    }

    return result;
}

inline int IpmiState1ToAlarmOffset(int state)
{
    switch (state)
    {
    case IPMI_SENSOR_STATE1_NORMAL:
        return ALARM_OFFSET_NORMAL;

    case IPMI_SENSOR_STATE1_LOWER_MAJOR_GOING_LOW:
    case IPMI_SENSOR_STATE1_UPPER_MAJOR_GOING_HIGH:
        return ALARM_OFFSET_MAJOR;

    case IPMI_SENSOR_STATE1_LOWER_CRITICAL_GOING_LOW:
    case IPMI_SENSOR_STATE1_UPPER_CRITICAL_GOING_HIGH:
        return ALARM_OFFSET_CRITICAL;

    case IPMI_SENSOR_STATE1_LOWER_NON_RECOVERABLE_GOING_LOW:
    case IPMI_SENSOR_STATE1_UPPER_NON_RECOVERABLE_GOING_HIGH:
        return ALARM_OFFSET_NON_RECOVERABLE;

    default:
        return ALARM_OFFSET_MAJOR;
        break;
    }
}

#if 0
AlarmInfo GetAlarmInfo(sensor_s const & s)
{
    AlarmInfo result = {-1, 0};
    int const state1 = GetSensorReadingState1(s);

    int index = -1;
    for (int i=0; i<int(sizeof(s_items)/sizeof(0[s_items])); ++i)
    {
        DescrSensorEntity const & item = s_items[i];
        if (IsDescrMatched(s.SensorName, item.descr))
        {
            index = i;
            break;
        }
    }

    if (-1!=index)
    {
        DescrSensorEntity const & refSensor = s_items[index];
        int alarmOffset = IpmiState1ToAlarmOffset(state1);
        if (alarmOffset>refSensor.faultOffsetMax)
        {
            alarmOffset = refSensor.faultOffsetMax;
        }

        {
            result.base = refSensor.faultBase;
            result.offset = alarmOffset;
        }
    }

    return result;
}
#endif
