#include "sensor_read.h"
#include "sensor_read_cache.h"
#include "FileGuard.h"
#include "copy_string.h"
#include "IpmiSensorEnums.h"
#include "GetSysTemperature.h"
#include "IpmiSensorDescrToType.h"
#include "dsp_monitor_getter.h"
#include "ProcessBase.h"
#include "LineTokenizer.h"
#include "strip_string.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>

#define FPGA_TEMP_FILE_PATH     ((std::string)(MCU_TMP_DIR+"/FPGATEMP"))
#define FPGA_TEMP_CACHE_FILE_PATH     ((std::string)(MCU_TMP_DIR+"/FPGATEMP.2"))

namespace
{
    struct CpuValSensor
    {
        int val;
        sensor_t sensor;
    } const s_cpuTemps[] =
    {
        { 0
            , { "CPU Temp", 25.0, "degrees C", "low", (float)-11.0, (float)-8.0, (float)-5.0, (float)80.0, (float)91.0, (float)100.0, EVENT_READ_TYPE_COLOR
                , {(float)-11.0, (float)-8.0, (float)-5.0, (float)80.0, (float)91.0, (float)100.0}
            }
        }
      , { 1
            , { "CPU Temp", 33.0, "degrees C", "medium", (float)-11.0, (float)-8.0, (float)-5.0, (float)80.0, (float)91.0, (float)100.0, EVENT_READ_TYPE_COLOR
                , {(float)-11.0, (float)-8.0, (float)-5.0, (float)80.0, (float)91.0, (float)100.0}
            }
        }
      , { 2
            , { "CPU Temp", 72.0, "degrees C", "high", (float)-11.0, (float)-8.0, (float)-5.0, (float)80.0, (float)91.0, (float)100.0, EVENT_READ_TYPE_COLOR
                , {(float)-11.0, (float)-8.0, (float)-5.0, (float)80.0, (float)91.0, (float)100.0}
            }
        }
      , { 4
            , { "CPU Temp", 93.0, "degrees C", "overheated", (float)-11.0, (float)-8.0, (float)-5.0, (float)80.0, (float)91.0, (float)100.0, EVENT_READ_TYPE_COLOR
                , {(float)-11.0, (float)-8.0, (float)-5.0, (float)80.0, (float)91.0, (float)100.0}
            }
        }
    };

    sensor_t CpuTempValToSensor(int val)
    {
        for (int i=0; i<int(sizeof(s_cpuTemps)/sizeof(0[s_cpuTemps])); ++i)
        {
            CpuValSensor const & elem = s_cpuTemps[i];
            if (elem.val==val)
            {
                return elem.sensor;
            }
        }

        return s_cpuTemps[1].sensor;
    }

    sensor_t GenerateHDDSensorResult(int val)
    {
        sensor_t result = {"HDD Temp", val, "degrees C", "ok", -9.0, -7.0, -5.0, 55.0, 85.0, 100.0, EVENT_READ_TYPE_COLOR
            , {(float)-9.0, (float)-7.0, (float)-5.0, (float)55.0, (float)85.0, (float)100.0}
        };
        return result;
    }

    sensor_t GenerateDSPTempSensorResult(int val)
    {
        sensor_t result = {"DSP  Temp", val, "degrees C", "ok", -15.0, -10.0, -5.0, 75.0, 85.0, 100.0, EVENT_READ_TYPE_COLOR
            , {(float)-15.0, (float)-10.0, (float)-5.0, (float)75.0, (float)85.0, (float)100.0}
        };
        return result;
    }

    sensor_t GenerateDSPVoltageSensorResult(int val)
    {
        sensor_t result = {"DSP  Voltage", val, "degrees C", "ok",25.0,40.0,  55.0,480.0,540.0, 600.0, EVENT_READ_TYPE_COLOR
            , {(float)25.0, (float)40.0, (float)55.0, (float)480.0, (float)540.0, (float)600.0}
        };
        return result;
    }
    inline int IsInvalidFloatReading(float val)
    {
        return val==INVALID_FLOAT_READING;
    }

    float GetSensorReading(string const & str)
    {
        if (str==INVALID_READING_STR)
        {
            return INVALID_FLOAT_READING;
        }
        else
        {
            char * endptr;
            return strtod(str.c_str(), &endptr);
        }
    }
}

void FillThresholdsWithRef(sensor_t * temp, sensor_thresholds_ref_t const & ref)
{
    int const ifLowerNA = (1
            | (INVALID_FLOAT_READING==temp->LowerNonRecoverable)
            | (INVALID_FLOAT_READING==temp->LowerCritical)
            | (INVALID_FLOAT_READING==temp->LowerNonCritical)
            );
    if (ifLowerNA)
    {
        temp->LowerNonRecoverable = ref.LowerNonRecoverable;
        temp->LowerCritical = ref.LowerCritical;
        temp->LowerNonCritical = ref.LowerNonCritical;
    }

    int const ifUpperNA = (1
            | (INVALID_FLOAT_READING==temp->UpperNonRecoverable)
            | (INVALID_FLOAT_READING==temp->UpperCritical)
            | (INVALID_FLOAT_READING==temp->UpperNonCritical)
            );
    if (ifUpperNA)
    {
        temp->UpperNonRecoverable = ref.UpperNonRecoverable;
        temp->UpperCritical = ref.UpperCritical;
        temp->UpperNonCritical = ref.UpperNonCritical;
    }
}

void ValidateCurrentValue(sensor_t * temp)
{
    if (INVALID_FLOAT_READING==temp->CurrentVal)
    {
        temp->CurrentVal = (float)0.0;
    }
}

int ParseSensorReadingLine(char const * line, sensor_t * temp, DescrSensorEntity & ref)
{
    LineTokenizer lt(std::string(line), "|", LineTokenizer::STRIP_SPACE_YES);
    if (lt.GetFieldNum()<10) return -1;

    memset(temp, 0x0, sizeof(sensor_t));
    
    int i = 0;

    CopyString(temp->SensorName, lt.GetField(i));
    //inplace_trim(temp->SensorName);
    ++i;
    if (!IsSensorValid(temp->SensorName, ref))
    {
        return -1;
    }

    temp->CurrentVal = GetSensorReading(lt.GetField(i));
    ++i;

    CopyString(temp->Unit, lt.GetField(i));
    //inplace_trim(temp->Unit);
    ++i;

    CopyString(temp->Status, lt.GetField(i));
    //inplace_trim(temp->Status);
    ++i;
    
    temp->LowerNonRecoverable = GetSensorReading(lt.GetField(i));
    ++i;
    
    temp->LowerCritical = GetSensorReading(lt.GetField(i));
    ++i;
    
    temp->LowerNonCritical = GetSensorReading(lt.GetField(i));
    ++i;
    
    temp->UpperNonCritical = GetSensorReading(lt.GetField(i));
    ++i;
    
    temp->UpperCritical = GetSensorReading(lt.GetField(i));
    ++i;
    
    temp->UpperNonRecoverable = GetSensorReading(lt.GetField(i));
    ++i;

    temp->eventReadType = ref.eventReadType;

	eProductType curProductType = CProcessBase::GetProcess()->GetProductType();
    if (0==strcmp(temp->SensorName, "CPU Temp") && (eProductTypeNinja != curProductType))
    {
#if 0
        return -1;
#else
        int const curVal = (int)temp->CurrentVal;
        *temp = CpuTempValToSensor(curVal);
#endif
    }

    return 0;
}

int sensor_read_ipmi_with_log(vector<sensor_t> & sensors, std::string * ipmiSensorLog = NULL)
{
    sensors.clear();
    if(ipmiSensorLog) ipmiSensorLog->clear();

#if 0
    //return 0;
#else

    {
        SensorLockGuard guardLock;
        FILE * const fp = fopen(SENSOR_READINGS_CACHE_FILE_SECOND.c_str(),  "r");

        if (!fp)
        {
            fprintf(stderr, "OPEN ERROR\n");
            return -1;
        }

        FCloseFile guardFile(fp);

        char line[200] = {0};
        while(fgets(line, 200, fp) != NULL)
        {
            sensor_t elem;
            sensor_t * const temp = &elem;
            DescrSensorEntity ref;
            if(ipmiSensorLog) ipmiSensorLog->append(line);
            
            if (0!=ParseSensorReadingLine(line, temp, ref))
            {
                memset(line, 0, sizeof(line));
                continue;
            }
            
            FillThresholdsWithRef(temp, ref.ref);
            ValidateCurrentValue(temp);
            //float const nominal = IpmiSensorDescrToNominalVal(temp->SensorName);
            //ValidateSensorReading(nominal, *temp);

            sensors.push_back(elem);
            memset(line, 0, sizeof(line));
        }
    }
    
#endif
    eProductType curProductType = CProcessBase::GetProcess()->GetProductType();
    if(eProductTypeNinja != curProductType)	
    {
        std::string hddTemp;
        GetHDDTemperature(hddTemp);

        int const temp = atoi(hddTemp.c_str());
        sensor_t const hddSensor = GenerateHDDSensorResult(temp);

        sensors.push_back(hddSensor);
    }

    if(eProductTypeNinja == curProductType)
    {
       // To do
      // Get DSP card info temp and Voltage from VMP
      DSPMonitorBoardList cards;
      GetDspMonitorStatus(cards);
      int iCardsCount = cards.len;
      for(int i =0 ; i< iCardsCount; ++i)
      {
          if(cards.status[i].boardId < 0 || cards.status[i].boardId > (DSP_CARD_SLOT_ID_2 - DSP_CARD_SLOT_ID_0))  //skip non-Netra DSP,  ex. RTM ISDN(3)
            continue;
          if(0 == strncmp(cards.status[i].temperature,NDM_TEMPE_DEFAULT,4 ))  //skip invalid card  NDM_TEMPE_DEFAULT
           continue;
          sensor_t  dspSensor2 = GenerateDSPTempSensorResult(atoi(cards.status[i].temperature));
          memset(dspSensor2.SensorName,0,sizeof(dspSensor2.SensorName));
          snprintf(dspSensor2.SensorName, sizeof(dspSensor2.SensorName), "DSP %d Temp",cards.status[i].boardId + 1);
          sensors.push_back(dspSensor2);
          /*
          sensor_t  dspSensor1 = GenerateDSPVoltageSensorResult(atoi(cards.status[i].voltage));  
          memset(dspSensor1.SensorName,0, sizeof(dspSensor1.SensorName));
          snprintf(dspSensor1.SensorName, sizeof(dspSensor1.SensorName), "DSP %d Voltage",cards.status[i].boardId + 1);
          sensors.push_back(dspSensor1);
          */
      }
      // Generate sensor result

         // Get FPGA Temp
        {
        	std::string cmd = "cp "+FPGA_TEMP_FILE_PATH+" "+FPGA_TEMP_CACHE_FILE_PATH;
            FILE * const fp = popen(cmd.c_str(), "r");
            PCloseFile guardFile(fp);
        }

        {
        	std::string fname = FPGA_TEMP_CACHE_FILE_PATH;
            FILE * const fp = fopen(fname.c_str(),  "r");

            if (NULL != fp)
            {
                FCloseFile guardFile(fp);

                char line[200] = {0};
                while(fgets(line, 200, fp) != NULL)
                {
                    sensor_t elem;
                    sensor_t * const temp = &elem;
                    DescrSensorEntity ref;
                    if(ipmiSensorLog) ipmiSensorLog->append(line);
                    
                    if (0!=ParseSensorReadingLine(line, temp, ref))
                    {
                        memset(line, 0, sizeof(line));
                        continue;
                    }
                    
                    FillThresholdsWithRef(temp, ref.ref);
                    ValidateCurrentValue(temp);

                    sensors.push_back(elem);
                    memset(line, 0, sizeof(line));
                }
            }
            else
            {
                fprintf(stderr, "OPEN ERROR\n");
            }
        }
      
    }
    return 0;
}

int sensor_read_ipmi(vector<sensor_t> & sensors)
{
	return sensor_read_ipmi_with_log(sensors, NULL);
}

SensorReadFunc sensor_read = sensor_read_ipmi;
SensorReadFuncWithLog sensor_read_with_log = sensor_read_ipmi_with_log;

void ValidateSensorReading(float nominal, sensor_s & reading)
{
    if (0==nominal)
    {
        nominal = reading.CurrentVal;
    }

    int const FACTOR_DIV = 4;
    float const interval = nominal/FACTOR_DIV;
    std::vector<float*> thresholds;
    float standard = nominal;

    {
        thresholds.push_back(&reading.LowerNonRecoverable);
        thresholds.push_back(&reading.LowerCritical);
        thresholds.push_back(&reading.LowerNonCritical);
        thresholds.push_back(&standard);
        thresholds.push_back(&reading.UpperNonCritical);
        thresholds.push_back(&reading.UpperCritical);
        thresholds.push_back(&reading.UpperNonRecoverable);
    }

    int validValStart = -1;
    int validValEnd = -1;
    for (int i=0; i<int(thresholds.size()); ++i)
    {
        float const val = *(thresholds[i]);
        if (IsInvalidFloatReading(val)) continue;

        if (-1==validValStart)
        {
            validValStart = i;
            validValEnd = i;
        }
        else
        {
            validValEnd = i;
        }
    }

    if (-1==validValStart)
    {
        validValStart = 2;
        validValEnd = 3;
        *(thresholds[validValStart]) = nominal - interval;
        *(thresholds[validValEnd]) = nominal + interval;
    }

    for (int i=validValStart-1; i>=0; --i)
    {
        *(thresholds[i]) = *(thresholds[i+1]) - interval;
    }

    for (int i=validValEnd+1; i<int(thresholds.size()); ++i)
    {
        *(thresholds[i]) = *(thresholds[i-1]) + interval;
    }
}

int GetSensorReadingState1(sensor_s const & s)
{
    float const val = s.CurrentVal;
    if ((!strstr(s.SensorName, "Current")) && val<=s.LowerNonRecoverable)
    {
        return IPMI_SENSOR_STATE1_LOWER_NON_RECOVERABLE_GOING_LOW;
    }
    else if ((!strstr(s.SensorName, "Current")) && val<=s.LowerCritical)
    {
        return IPMI_SENSOR_STATE1_LOWER_CRITICAL_GOING_LOW;
    }
    else if ((!strstr(s.SensorName, "Current")) && val<=s.LowerNonCritical)
    {
        return IPMI_SENSOR_STATE1_LOWER_MAJOR_GOING_LOW;
    }
	else if(!strncmp(s.SensorName,"PS",2))
	{
	  if(0 != strncasecmp(s.Status, "0x01", 4))
	  	return IPMI_SENSOR_STATE1_LOWER_MAJOR_GOING_LOW;
	  else if(0 == strncasecmp(s.Status, "0x01", 4))
	  	return IPMI_SENSOR_STATE1_NORMAL;
	  else 
	  	return IPMI_SENSOR_STATE1_UPPER_NON_RECOVERABLE_GOING_HIGH;
	}
    else if (val<s.UpperNonCritical)
    {
        return IPMI_SENSOR_STATE1_NORMAL;
    }
    else if (val<s.UpperCritical)
    {
        return IPMI_SENSOR_STATE1_UPPER_MAJOR_GOING_HIGH;
    }
    else if (val<s.UpperNonRecoverable)
    {
        return IPMI_SENSOR_STATE1_UPPER_CRITICAL_GOING_HIGH;
    }
    else
    {
        return IPMI_SENSOR_STATE1_UPPER_NON_RECOVERABLE_GOING_HIGH;
    }
}

