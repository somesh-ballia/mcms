#ifndef _CPU_TEMPERATURE_CONTROL_H___
#define _CPU_TEMPERATURE_CONTROL_H___

#include <string>
#include "StatusesGeneral.h"
#include "SharedDefines.h"
#include "DefinesGeneral.h"
#include "SystemFunctions.h"


class CCpuTemperatureControl
{
		
public:
	
	CCpuTemperatureControl(eProductType prodcutType, eCpuManufacturerType cpuManufacturerType) :
		m_prodcutType(prodcutType), m_cpuManufacturerType(cpuManufacturerType)
	{	
	}
	
	virtual STATUS GetCpuTemperature(std::string& temp) = 0;
	
	STATUS GetInitializeStatus()
	{
		return m_initializeStatus;
	}
	
	virtual std::string GetName() = 0;

	virtual ~CCpuTemperatureControl(){;}
	
protected: 	
	STATUS m_initializeStatus ;	
			
	eProductType 			m_prodcutType;
	eCpuManufacturerType 	m_cpuManufacturerType;
	
	static std::string DEFAULT_TEMP;
	
};

class CCpuTemperatureControlRMX1500 : public CCpuTemperatureControl
{
	
public:
	CCpuTemperatureControlRMX1500(eProductType prodcutType, eCpuManufacturerType cpuManufacturerType); 
	
	virtual STATUS GetCpuTemperature(std::string& temp);

	std::string GetName() 
	{
		return "CCpuTemperatureControlRMX1500";
	}
	
private:
	STATUS Initialize() ;
	
	 std::string 	m_sensorReg;
	 std::string 	m_register_addressReg;	 
};


class CCpuTemperatureControlRMX4000: public CCpuTemperatureControl
{
	
public:
	CCpuTemperatureControlRMX4000(eProductType prodcutType, eCpuManufacturerType cpuManufacturerType) : 
		CCpuTemperatureControl(prodcutType, cpuManufacturerType)
	{
		m_initializeStatus = Initialize();
	}
		
	virtual STATUS GetCpuTemperature(std::string& temp);
	

	std::string GetName()
	{
		return "CCpuTemperatureControlRMX4000";
	}
	
private:
	
	STATUS Initialize() ;
	STATUS GetReadTemperature( int hwmon_index, std::string& temp);
	STATUS GetTemperatureFromSpecificTempFile(int hwmon_index, int indTempFile, std::string& temp);

	
	std::string m_prefix_path;
	
};

class CCpuTemperatureControlRMX2000 : public CCpuTemperatureControl
{
	
public:
	CCpuTemperatureControlRMX2000(eProductType prodcutType, eCpuManufacturerType cpuManufacturerType) : 
		CCpuTemperatureControl(prodcutType, cpuManufacturerType)	
	{
		std::string temp;
		m_initializeStatus = GetTempFirstAndDecideOnOptimization(temp);
	}	
	
	virtual STATUS GetCpuTemperature(std::string& temp);
		
	std::string GetName() 
	{
		return "CCpuTemperatureControlRMX2000";
	}
	
private:
	
	
	STATUS GetTempFirstAndDecideOnOptimization(std::string& temp);		
	STATUS GetTempFromFileIfExist(const std::string& fileName, BOOL is_hwmon0_file, std::string& temp);
	
	
	std::string		m_tempFileName;
	BOOL 			m_is_hwmon0_file;	
};


class CCpuTemperatureControlUnknown : public CCpuTemperatureControl
{
public:
	CCpuTemperatureControlUnknown(eProductType prodcutType, eCpuManufacturerType cpuManufacturerType) : 
		CCpuTemperatureControl(eProductTypeUnknown, eCpuManufacturerTypeUnknown)		
	{
		m_initializeStatus = STATUS_OK;
	}
	
	virtual STATUS GetCpuTemperature(std::string& temp)
	{
		temp =  DEFAULT_TEMP;
		return STATUS_OK;
	}
	std::string GetName() 
	{
		return "CCpuTemperatureControlUnknown";
	}

};

class CCpuTemperatureControlCpuType : public CCpuTemperatureControl
{

public:
	CCpuTemperatureControlCpuType(eProductType prodcutType, eCpuManufacturerType cpuManufacturerType);

	virtual STATUS GetCpuTemperature(std::string& temp);

	std::string GetName()
	{
		return "CCpuTemperatureControlCpuType";
	}

private:
	STATUS Initialize();
};


#endif
