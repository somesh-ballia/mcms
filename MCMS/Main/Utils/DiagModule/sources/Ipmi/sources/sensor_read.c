#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "sensor_read.h"
#include "sensor_read_cache.h"
#include "IpmiSensorEnums.h"
#include "GetSysTemperature.h"
#include "IpmiSensorDescrToType.h"
#include "tools.h"

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
	size_t i=0;
    for (; i<sizeof(s_cpuTemps)/sizeof(0[s_cpuTemps]); ++i)
    {
        if (s_cpuTemps[i].val==val)
        {
            return s_cpuTemps[i].sensor;
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

float GetSensorReading(const char * str)
{
    if (0 == strcmp(str,INVALID_READING_STR))
    {
        return INVALID_FLOAT_READING;
    }
    else
    {
        char * endptr;
        return strtod(str, &endptr);
    }
}


void FillThresholdsWithRef(sensor_t * temp, const sensor_thresholds_ref * ref)
{
    int const ifLowerNA = (1
            | (INVALID_FLOAT_READING==temp->LowerNonRecoverable)
            | (INVALID_FLOAT_READING==temp->LowerCritical)
            | (INVALID_FLOAT_READING==temp->LowerNonCritical)
            );
    if (ifLowerNA)
    {
        temp->LowerNonRecoverable = ref->LowerNonRecoverable;
        temp->LowerCritical = ref->LowerCritical;
        temp->LowerNonCritical = ref->LowerNonCritical;
    }

    int const ifUpperNA = (1
            | (INVALID_FLOAT_READING==temp->UpperNonRecoverable)
            | (INVALID_FLOAT_READING==temp->UpperCritical)
            | (INVALID_FLOAT_READING==temp->UpperNonCritical)
            );
    if (ifUpperNA)
    {
        temp->UpperNonRecoverable = ref->UpperNonRecoverable;
        temp->UpperCritical = ref->UpperCritical;
        temp->UpperNonCritical = ref->UpperNonCritical;
    }
}

void ValidateCurrentValue(sensor_t * temp)
{
    if (INVALID_FLOAT_READING==temp->CurrentVal)
    {
        temp->CurrentVal = (float)0.0;
    }
}

int ParseSensorReadingLine(char * line, sensor_t * temp, DescrSensorEntity * ref)
{
    char ** strArray = NULL;
    int ret = -1;
    int count = LineSplitTrim(line, &strArray,  "|");
    if (count < 10) goto done;;

    memset(temp, 0x0, sizeof(sensor_t));

    int i = 0;

    strncpy(temp->SensorName, NVSTR(strArray[i]), sizeof(temp->SensorName) - 1);
    //inplace_trim(temp->SensorName);
    ++i;
    if (!IsSensorValid(temp->SensorName, ref))
    {
         goto done;
    }

    temp->CurrentVal = GetSensorReading(NVSTR(strArray[i]));
    ++i;

    strncpy(temp->Unit, NVSTR(strArray[i]), sizeof(temp->Unit) - 1);
    //inplace_trim(temp->Unit);
    ++i;

    strncpy(temp->Status, NVSTR(strArray[i]), sizeof(temp->Status) - 1);
    //inplace_trim(temp->Status);
    ++i;
    
    temp->LowerNonRecoverable = GetSensorReading(NVSTR(strArray[i]));
    ++i;
    
    temp->LowerCritical = GetSensorReading(NVSTR(strArray[i]));
    ++i;
    
    temp->LowerNonCritical = GetSensorReading(NVSTR(strArray[i]));
    ++i;
    
    temp->UpperNonCritical = GetSensorReading(NVSTR(strArray[i]));
    ++i;
    
    temp->UpperCritical = GetSensorReading(NVSTR(strArray[i]));
    ++i;
    
    temp->UpperNonRecoverable = GetSensorReading(NVSTR(strArray[i]));
    ++i;

    temp->eventReadType = ref->eventReadType;

/*   Ninja can get CPU Temp from sensor
    if (0==strcmp(temp->SensorName, "CPU Temp"))
    {
#if 0
        return -1;
#else
        int const curVal = (int)temp->CurrentVal;
        *temp = CpuTempValToSensor(curVal);
#endif
    }
*/

    ret = 0;
	
done:
	
    if(NULL != strArray) LineSplitFree(strArray, count);
    return ret;
}

void sensor_array_add(sensor_t ** sensorArray, int * count, int * size, const sensor_t * sensor)
{
	static const unsigned int step = 150;
	sensor_t * array = NULL;
	if(NULL == (*sensorArray))
	{
		array = malloc(step * sizeof(sensor_t));
		if(NULL == array)
		{
			return;
		}
		*sensorArray = array;
		*count = 0;
		*size = step;
	}

	//resize
	if(*count >= *size)
	{
		array = malloc((*size + step) * sizeof(sensor_t));
		if(NULL == array)
		{
			return;
		}
		memcpy(array, sensorArray, (*size) * sizeof(sensor_t));
		free(*sensorArray);
		*sensorArray = array;
		*size = *size + step;
	}
	
	(*sensorArray)[*count] = *sensor;
	(*count)++;
}

int sensor_read_ipmi(sensor_t ** sensorArray, int * count, int * size)
{
    int ret = -1;
    FILE * fp = NULL;
    *sensorArray = NULL;
    {
        LockSensorsCache();
        fp = fopen(SENSOR_READINGS_CACHE_FILE_SECOND,  "r");

        if (!fp)
        {
            fprintf(stderr, "OPEN ERROR\n");
            goto done;
        }

        char line[200];
        while(fgets(line, 200, fp) != NULL)
        {
            sensor_t elem;
            sensor_t * const temp = &elem;
            DescrSensorEntity ref;

            if (0!=ParseSensorReadingLine(line, temp, &ref))
            {
                continue;
            }
            
            FillThresholdsWithRef(temp, &(ref.ref));
            ValidateCurrentValue(temp);
            //float const nominal = IpmiSensorDescrToNominalVal(temp->SensorName);
            //ValidateSensorReading(nominal, *temp);

            sensor_array_add(sensorArray, count, size, &elem);
        }
    }
/*  Ninja has not hdd temp sensor
    {
        int hddTemp;
        GetHDDTemperature(&hddTemp);

        int const temp = hddTemp;
        sensor_t const hddSensor = GenerateHDDSensorResult(temp);

	  sensor_array_add(sensorArray, count, size, &hddSensor);
    }
*/
      // ydong TODO:
      // Get DSP card info temp and Voltage from VMP
      /*
      DSPMonitorBoardList cards;
      GetDspMonitorStatus(cards);
      int iCardsCount = cards.len;
      for(int i =0 ; i< iCardsCount; ++i)
      {
          sensor_t  dspSensor1 = GenerateDSPVoltageSensorResult(atoi(cards.status[i].voltage));  
          memset(dspSensor1.SensorName,0,16);
          sprintf(dspSensor1.SensorName,"DSP %d Voltage ",cards.status[i].boardId + 1);
          sensors.push_back(dspSensor1);

          sensor_t  dspSensor2 = GenerateDSPTempSensorResult(atoi(cards.status[i].temperature));
          memset(dspSensor2.SensorName,0,16);
          sprintf(dspSensor2.SensorName,"DSP %d Temp ",cards.status[i].boardId + 1);
          sensors.push_back(dspSensor2);
      }
      */
      // Generate sensor result
      
done:
	
    if(fp) fclose(fp);
    UnlockSensorsCache();
    return 0;
}

SensorReadFunc sensor_read = sensor_read_ipmi;
#if 0
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
#endif
int GetSensorReadingState1(sensor_t * s)
{
    float const val = s->CurrentVal;
    if ((!strstr(s->SensorName, "Current")) && val<=s->LowerNonRecoverable)
    {
        return IPMI_SENSOR_STATE1_LOWER_NON_RECOVERABLE_GOING_LOW;
    }
    else if ((!strstr(s->SensorName, "Current")) && val<=s->LowerCritical)
    {
        return IPMI_SENSOR_STATE1_LOWER_CRITICAL_GOING_LOW;
    }
    else if ((!strstr(s->SensorName, "Current")) && val<=s->LowerNonCritical)
    {
        return IPMI_SENSOR_STATE1_LOWER_MAJOR_GOING_LOW;
    }
    else if(!strncmp(s->SensorName,"PS",2))
    {
        if(0 != strncasecmp(s->Status, "0x01", 4))
        	return IPMI_SENSOR_STATE1_LOWER_MAJOR_GOING_LOW;
        else if(0 == strncasecmp(s->Status, "0x01", 4))
        	return IPMI_SENSOR_STATE1_NORMAL;
        else 
        	return IPMI_SENSOR_STATE1_UPPER_NON_RECOVERABLE_GOING_HIGH;
    }
    else if (val<s->UpperNonCritical)
    {
        return IPMI_SENSOR_STATE1_NORMAL;
    }
    else if (val<s->UpperCritical)
    {
        return IPMI_SENSOR_STATE1_UPPER_MAJOR_GOING_HIGH;
    }
    else if (val<s->UpperNonRecoverable)
    {
        return IPMI_SENSOR_STATE1_UPPER_CRITICAL_GOING_HIGH;
    }
    else
    {
        return IPMI_SENSOR_STATE1_UPPER_NON_RECOVERABLE_GOING_HIGH;
    }
}

int GetUsageReadingState1(int usage)
{
    if (usage>998)
    {
        return IPMI_SENSOR_STATE1_UPPER_NON_RECOVERABLE_GOING_HIGH;
    }
    else if (usage>980)
    {
        return IPMI_SENSOR_STATE1_UPPER_CRITICAL_GOING_HIGH;
    }
    else if (usage>900)
    {
        return IPMI_SENSOR_STATE1_UPPER_MAJOR_GOING_HIGH;
    }
    else
    {
        return IPMI_SENSOR_STATE1_NORMAL;
    }
}

