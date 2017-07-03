#include "CpuTemperatureControl.h"
#include "TraceStream.h"
#include "ConfigManagerApi.h"
#include <string.h>
#include <limits>
#include <stdlib.h>
#include "OsFileIF.h"

std::string CCpuTemperatureControl::DEFAULT_TEMP = "15";

//////////////////  RMX 1500 

CCpuTemperatureControlRMX1500::CCpuTemperatureControlRMX1500(eProductType prodcutType, eCpuManufacturerType cpuManufacturerType) : 
	CCpuTemperatureControl(prodcutType, cpuManufacturerType)
{
	m_initializeStatus = Initialize();
}


STATUS CCpuTemperatureControlRMX1500::Initialize() 
{
	if (IsTarget())
	{
		if (m_cpuManufacturerType == eCpuManufacturerTypePortwell)
		{
			 m_sensorReg = "0x4c";		 
			 m_register_addressReg =  "0x01";
			 return STATUS_OK;
		}
		else if (m_cpuManufacturerType == eCpuManufacturerTypeFintec) 
		{
			 m_sensorReg = "0x2d";		 
			 m_register_addressReg =  "0x14";
			 return STATUS_OK;
		}
		else 
		{
			FTRACESTR(eLevelInfoNormal)
					<< "GetCpuTemperature, chip is different from Fintec and Portwell: " << (int)m_cpuManufacturerType;
			
			return STATUS_FAIL;
		}
	}
	else
	{
		return STATUS_OK;
	}
}



STATUS CCpuTemperatureControlRMX1500::GetCpuTemperature(std::string& temp)
{
	if (!IsTarget())
	{		
		temp = DEFAULT_TEMP;
		return STATUS_OK;
	}	
	if (m_initializeStatus == STATUS_FAIL)
	{		
		FTRACESTR(eLevelInfoNormal)	<< "CCpuTemperatureControlRMX1500::GetCpuTemperature: Failed to get temperator";		
		temp = DEFAULT_TEMP;
		return STATUS_FAIL;
	}	
	CConfigManagerApi api;	
	STATUS res = api.ReadValueFromRegisterBySensor(temp, m_sensorReg, m_register_addressReg);
	
	if (res != STATUS_OK)
	{
		FTRACESTR(eLevelInfoNormal)
				<< "GetCpuTemperature, eProductTypeRMX1500 , STATUS = "
				<< res;
		temp = DEFAULT_TEMP;
		return res;
	}
	
	char* p;
	char temp_decimal_str[256];
	long n = strtol(temp.c_str(), &p, 16);
	sprintf(temp_decimal_str, "%ld", n);
	temp = temp_decimal_str;
	
	FTRACESTR(eLevelInfoNormal) << "GetCpuTemperature, end 1500 , temp = "
			<< temp << " res = " << res << " n = " << n
			<< " temp_decimal_str = " << temp_decimal_str;
	
	return STATUS_OK;
}


//////////////////  RMX 4000

STATUS CCpuTemperatureControlRMX4000::Initialize() 
{	    
    if (IsTarget())
    {
    	m_prefix_path = "/sys/class/hwmon/hwmon";
    }   
    
    else {
    	m_prefix_path = MCU_TMP_DIR+"/class/hwmon/hwmon";
    }
    return STATUS_OK;
}

STATUS CCpuTemperatureControlRMX4000::GetCpuTemperature(std::string& temp)
{
	int max_temp = 0, tempValue;
	
	STATUS status = STATUS_FAIL;


	// patch - CNTL's CPU Temperature reading is incorrect after upgrading system. temperature is now read from other places
    	// after system upgrade... (temp1_input temp2_input)
	int arrIndexFiles[2] = {2,3};
	for (unsigned int i = 0; i< sizeof(arrIndexFiles)/sizeof(int); ++i)
	{
		for (unsigned int j = 0; j< 2; ++j)
		{
			if (STATUS_OK == GetTemperatureFromSpecificTempFile(j, arrIndexFiles[i], temp))
			{
				status = STATUS_OK;
				tempValue = atoi(temp.c_str());
				if (tempValue > max_temp)
				{
					max_temp = tempValue;
				}
			}
		}

	}

    if (status != STATUS_OK)
    {
    	FTRACESTR(eLevelInfoNormal) << "trying to get temp from original location";
		for (int i = 0; i < 4; i++)
		{
			if (STATUS_OK == GetReadTemperature(i, temp))
			{
				status = STATUS_OK;
				tempValue = atoi(temp.c_str());
				if (tempValue > max_temp)
				{
					max_temp = tempValue;
				}
			}
		}
    }

    if (status == STATUS_OK)
    {
		std::ostringstream tempStream;
		tempStream << max_temp;
		temp = tempStream.str();    	
    	// FTRACESTR(eLevelInfoNormal) << "GetCpuTemperature, end 4000 , temp = " << max_temp;

    }
    else 
    {    	
    	FTRACESTR(eLevelInfoNormal) << "GetCpuTemperature, end 4000 . Failed to get temperature from files prefix: " << m_prefix_path;
    	temp = DEFAULT_TEMP;
    }
    
    return status;
}

STATUS CCpuTemperatureControlRMX4000::GetReadTemperature( int hwmon_index, std::string& temp)
{
	std::string        answer1, answer;
	std::ostringstream full_path;

	full_path << m_prefix_path <<hwmon_index<<"/device/";
	for (unsigned int j = 1; j< 3; ++j)
	{
	//	path_file << full_path.str() << "temp1_label";
		//temp_input << full_path.str() << "temp1_input";
		std::ostringstream path_file;
		std::ostringstream temp_input;
		path_file << full_path.str() << "temp"<<j<<"_label";
		temp_input << full_path.str() << "temp"<<j<<"_input";

		if (IsFileExists(path_file.str().c_str())  &&
				ReadFileToString(path_file.str().c_str(),std::numeric_limits<unsigned int>::max(), answer) == STATUS_OK)
		{
			if ((answer.find("Core 0") != string::npos) ||
					(answer.find("Core 1") != string::npos))
			{
				if (IsFileExists(temp_input.str().c_str()) &&
						ReadFileToString(temp_input.str().c_str(), std::numeric_limits<unsigned int>::max(), answer1) == STATUS_OK)
				{
					if (answer1.length() >= 1 && answer1[0] != '-')
					{
						temp = answer1.substr(0, 2);
						FTRACEINTO << "GetTemperatureReading - path_file  " << path_file.str() << " temp_input " << " answer " << answer << " Reading: " << 	answer1 << " Temp: "<< temp;
						return STATUS_OK;
					}
					else
					{
						FTRACEINTO << "GetTemperatureReading invalid temperature in  " << path_file.str() << " temp: " << " answer1 " << answer1;
					}
				}
			}
		}
		FTRACEINTO << "GetTemperatureReading Failed - path_file  " << path_file.str() << " temp_input " << " answer " << answer << " Reading: " << 	answer1 << " Temp: "<< temp;
	}
	

	return STATUS_FAIL;
}


// get temperature from other files after new system
STATUS CCpuTemperatureControlRMX4000::GetTemperatureFromSpecificTempFile(int hwmon_index, int indTempFile, std::string& temp)
{
	STATUS status = STATUS_FAIL;

	std::ostringstream temp_input;
	std::string        answer;

	temp_input << m_prefix_path << hwmon_index << "/device/" << "temp" << indTempFile << "_input";

	if (IsFileExists(temp_input.str().c_str()) &&
		ReadFileToString(temp_input.str().c_str(), std::numeric_limits<unsigned int>::max(), answer) == STATUS_OK)
	{
		if (answer.length() >=1 && answer[0] != '-')
		{
			temp = answer.substr(0, 2);
			status = STATUS_OK;
		}
		else
		{
			FTRACEINTO << "GetTemperatureReading invalid temperature in  " << temp_input.str() << " temp: " << " answer " << answer;
		}
	}
	else
	{
		FTRACEINTO << "GetTemperatureReading file not exists " << temp_input.str();
	}
	return status;
}

//////////////////  RMX 2000

STATUS CCpuTemperatureControlRMX2000::GetTempFromFileIfExist(const std::string& fileName, BOOL is_hwmon0_file, std::string& temp)
{		
	std::string answer;
		
    FTRACESTR(eLevelInfoNormal)    << "GetTempFromFileIfExist: checking   " << fileName;
    STATUS status = STATUS_FAIL;
        
	if (IsFileExists(fileName))
	{		
		if (ReadFileToString(fileName.c_str(),std::numeric_limits<unsigned int>::max(), answer) == STATUS_OK)
		{
			if(answer.length() >= 1 && answer[0] != '-')
			{
				// FTRACESTR(eLevelInfoNormal)    << "Read file " << fileName << " content " << answer;
				if (!is_hwmon0_file)
				{
					temp = answer.substr(0, 2);
					status = STATUS_OK;

				}
				else
				{
					int ans_atoi = atoi(answer.c_str());

					if (40000 != ans_atoi && ans_atoi !=0)
					{
						temp = answer.substr(0, 2);
						status = STATUS_OK;
					}

				}
			}
			else
			{
			    FTRACESTR(eLevelInfoNormal)    << "GetTempFromFileIfExist: Invalid temperature in file:  " << answer;

			}
		}
	}	
	
	if (status == STATUS_OK)
	{				
		m_tempFileName =  fileName;
	    m_is_hwmon0_file = is_hwmon0_file;	    
	}
	return status;
}

STATUS CCpuTemperatureControlRMX2000::GetTempFirstAndDecideOnOptimization(std::string& temp)
{
	STATUS status = STATUS_FAIL; 
	if(IsTarget())
	{
		if (GetTempFromFileIfExist("/sys/devices/platform/coretemp.0/temp1_input", FALSE, temp) == STATUS_OK)
		{			
			status = STATUS_OK;
		}
		else if (GetTempFromFileIfExist("/sys/class/hwmon/hwmon0/device/temp1_input", FALSE, temp) == STATUS_OK)
		{			
			status=  STATUS_OK;
		}
		else if (GetTempFromFileIfExist("/sys/class/hwmon/hwmon0/temp1_input", TRUE, temp) == STATUS_OK)
		{
			status =  STATUS_OK;
		}
		else if (GetTempFromFileIfExist("/sys/class/hwmon/hwmon2/device/temp1_input", FALSE, temp) == STATUS_OK)
		{
			status =  STATUS_OK;
		}
		else if (GetTempFromFileIfExist("/sys/class/hwmon/hwmon2/device/temp2_input", FALSE, temp) == STATUS_OK)
		{
			status =  STATUS_OK;
		}
		else if (GetTempFromFileIfExist("/sys/class/hwmon/hwmon0/device/temp2_input", FALSE, temp) == STATUS_OK)
		{
			status =  STATUS_OK;
		}
		else if (GetTempFromFileIfExist("/sys/class/hwmon/hwmon1/device/temp1_input", FALSE, temp) == STATUS_OK)
		{
			status =  STATUS_OK;
		}
		else if (GetTempFromFileIfExist("/sys/class/hwmon/hwmon1/device/temp2_input", FALSE, temp) == STATUS_OK)
		{
			status =  STATUS_OK;
		}
	}	
	else 
	{				
		std::string fname = MCU_TMP_DIR+"/class/hwmon/hwmon0/device/temp1_input";
		if (GetTempFromFileIfExist(fname.c_str(), FALSE, temp) == STATUS_OK)
		{
			status = STATUS_OK;
		}		
	}
	if (status == STATUS_OK)
	{
		FTRACESTR(eLevelInfoNormal)    << "GetTempFirstAndDecideOnOptimization:  file is " << m_tempFileName << " m_is_hwmon0_file "  << m_is_hwmon0_file << "current temp" << temp;
	}
	else 
	{
		FTRACESTR(eLevelInfoNormal)    << "GetTempFirstAndDecideOnOptimization:  failed o get temperature file";		
	}
	temp = DEFAULT_TEMP;
	return status;
}

STATUS CCpuTemperatureControlRMX2000::GetCpuTemperature(std::string& temp)
{
	std::string answer;
	// we should get here and succeed to get the temperature all the times except the first time.
	if (m_initializeStatus == STATUS_OK)
	{		
		if (ReadFileToString(m_tempFileName.c_str(),std::numeric_limits<unsigned int>::max(), answer) == STATUS_OK)
		{		
			if (!m_is_hwmon0_file)
			{
				temp = answer.substr(0, 2);				
 				// FTRACESTR(eLevelInfoNormal)   << "CCpuTemperatureControlRMX2000::GetCpuTemperature:  " << temp;			
				return STATUS_OK;			
			}
			else 			
			{
				int ans_atoi = atoi(answer.c_str());
				if (40000 != ans_atoi && ans_atoi !=0)
				{			
					// FTRACESTR(eLevelInfoNormal)   << "CCpuTemperatureControlRMX2000::GetCpuTemperature (checked 4000):  " << temp;					
					temp = answer.substr(0, 2);
					return STATUS_OK;
				}
				else
				{					
					FTRACESTR(eLevelInfoNormal)   << "CCpuTemperatureControlRMX2000::GetCpuTemperature no 4000:  " << ans_atoi;
				}
			}
		}
	}
	//  Failed in initialization before 
	if (m_initializeStatus == STATUS_OK)
	{
		FTRACESTR(eLevelInfoNormal)   << "CpuTemperatureControlRMX2000::GetCpuTemperature: Failed to get temperature from file: " << m_tempFileName;
	}
	else 
	{
		FTRACESTR(eLevelInfoNormal)   << "CpuTemperatureControlRMX2000::GetCpuTemperature: Initialization was previously failed " ;
	}	
	// we failed so we try to initialize again
	m_initializeStatus = GetTempFirstAndDecideOnOptimization(temp);
	return m_initializeStatus;	
}
//////////////////  CPU TYPE

CCpuTemperatureControlCpuType::CCpuTemperatureControlCpuType(eProductType prodcutType, eCpuManufacturerType cpuManufacturerType) :
	CCpuTemperatureControl(prodcutType, cpuManufacturerType)
{
	m_initializeStatus = STATUS_OK;
}

STATUS CCpuTemperatureControlCpuType::GetCpuTemperature(std::string& temp)
{

	CConfigManagerApi api;
	std::string answer;
	STATUS res;

	res = api.ReadTempFromAdvantechUtil(answer);
	if(answer == "Fail" || res != STATUS_OK)
	{
		temp = DEFAULT_TEMP;
		return STATUS_FAIL;
	}

	temp = answer.substr(0, 2);
	return STATUS_OK;
}
