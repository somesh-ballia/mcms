#ifndef CARDRESOURCECONFIG_H_
#define CARDRESOURCECONFIG_H_

#include "PObject.h"
#include "SharedMcmsCardsStructs.h"
#include "SystemResources.h"
#include "HelperFuncs.h"

////////////////////////////////////////////////////////////////////////////
//                        CCardResourceConfig
////////////////////////////////////////////////////////////////////////////
class CCardResourceConfig : public CPObject
{
	CLASS_TYPE_1(CCardResourceConfig, CPObject)

public:
	                  CCardResourceConfig(eCardType cardType);
	virtual          ~CCardResourceConfig(){ }
	const char*       NameOf() const                                                       { return "CCardResourceConfig"; }

	static void       SetNumAudPortsLeftToConfig( WORD numAudPortsLeftToConfig )           { m_num_aud_ports_left_to_config = numAudPortsLeftToConfig; }
	static WORD       GetNumAudPortsLeftToConfig()                                         { return m_num_aud_ports_left_to_config; }

	static void       SetNumVidHD720PortsLeftToConfig( WORD NumVidHD720PortsLeftToConfig ) { m_num_vid_hd720_ports_left_to_config = NumVidHD720PortsLeftToConfig; }
	static WORD       GetNumVidHD720PortsLeftToConfig()                                    { return m_num_vid_hd720_ports_left_to_config; }

	void              SetNumConfiguredUnits(DWORD num_configured_ARTs, DWORD num_configured_VIDEOs);

	STATUS            Config(RSRCALLOC_UNIT_CONFIG_PARAMS_S* pConfigParams, CM_UNITS_CONFIG_S* pRetConfigParams, WORD card_type);
	void              DivideArtAndVideoUnitsAccordingToProportion(DWORD numUnits, DWORD& numArtUnitsPerBoard, DWORD& numVideoUnitsPerBoard);

	virtual DWORD     GetNumConfiguredARTPorts() const   = 0;
	virtual DWORD     GetNumConfiguredVideoPorts() const = 0;

private:
	virtual float     GetAudBasePrml() const = 0;
	virtual float     GetVidBasePrml() const = 0;

	virtual float     GetConfigProportion() = 0;
	eUnitType         DecideWhatToConfigureNext();
	virtual STATUS    FindConfigureArtUnit(RSRCALLOC_UNIT_CONFIG_PARAMS_S* pConfigParams, CM_UNITS_CONFIG_S* pRetConfigParams, WORD card_type)   = 0;
	virtual STATUS    FindConfigureVideoUnit(RSRCALLOC_UNIT_CONFIG_PARAMS_S* pConfigParams, CM_UNITS_CONFIG_S* pRetConfigParams, WORD card_type) = 0;


protected:
	CSystemResources* m_pSystemResources;

	float             m_num_Needed_ART_prml;
	float             m_num_Needed_VID_prml;

	DWORD             m_num_configured_ARTs;
	DWORD             m_num_configured_VIDEOs;

	DWORD             m_num_configured_ARTs_Other_Cards;
	DWORD             m_num_configured_VIDEOs_Other_Cards;

	static WORD       m_num_aud_ports_left_to_config;       // This is per system (hence it is static member - shared counter for all cards)
	static WORD       m_num_vid_hd720_ports_left_to_config; // This is per system (hence it is static member - shared counter for all cards)

	eCardType         m_cardType;
};


////////////////////////////////////////////////////////////////////////////
//                        CCardResourceConfigBreeze
////////////////////////////////////////////////////////////////////////////
class CCardResourceConfigBreeze : public CCardResourceConfig
{
	CLASS_TYPE_1(CCardResourceConfigBreeze, CCardResourceConfig)

public:
	                  CCardResourceConfigBreeze(eCardType cardType) : CCardResourceConfig(cardType){ };
	virtual          ~CCardResourceConfigBreeze(){ }

	static BOOL       ShouldBeARTUnit(int unitID);
	static BOOL       CanBeUsedForARTRecovery(int unitID);
	static BOOL       CanUnitBeArtSwapRecovery(int unitID);
	static BOOL       IsTurboVideoUnit(int unitID);
	static void       SetConfigProportion( float configProp ) { m_configProportion = configProp; }
	virtual float     GetConfigProportion()                   { return m_configProportion; }

	virtual float     GetAudBasePrml() const;
	virtual float     GetVidBasePrml() const                  { return VID_TOTAL_HD720_30FS_PROMILLES_BREEZE; }

	virtual DWORD     GetNumConfiguredARTPorts() const;
	virtual DWORD     GetNumConfiguredVideoPorts() const;

private:

	virtual STATUS    FindConfigureArtUnit(RSRCALLOC_UNIT_CONFIG_PARAMS_S* pConfigParams, CM_UNITS_CONFIG_S* pRetConfigParams, WORD card_type);
	virtual STATUS    FindConfigureVideoUnit(RSRCALLOC_UNIT_CONFIG_PARAMS_S* pConfigParams, CM_UNITS_CONFIG_S* pRetConfigParams, WORD card_type);
	STATUS            FindConfigureArtUnit(RSRCALLOC_UNIT_CONFIG_PARAMS_S* pConfigParams, CM_UNITS_CONFIG_S* pRetConfigParams, WORD card_type, bool useTurboUnit, BoardID& boardId, UnitID& unitId, bool& isController);
	bool              FindConfigureArtUnitForACPreferred(RSRCALLOC_UNIT_CONFIG_PARAMS_S* pConfigParams, CM_UNITS_CONFIG_S* pRetConfigParams, UnitID preferredUnitId, BoardID& boardId, UnitID& unitId, bool& isController, STATUS& status);

	static float      m_configProportion;
};


////////////////////////////////////////////////////////////////////////////
//                        CCardResourceConfigMpmRx
////////////////////////////////////////////////////////////////////////////
class CCardResourceConfigMpmRx : public CCardResourceConfig
{
	CLASS_TYPE_1(CCardResourceConfigMpmRx, CCardResourceConfig)

public:
	                  CCardResourceConfigMpmRx(eCardType cardType) : CCardResourceConfig(cardType){ };
	virtual          ~CCardResourceConfigMpmRx(){ }

	static void       SetConfigProportion( float configProp ) { m_configProportion = configProp; }
	virtual float     GetConfigProportion()                   { return m_configProportion; }

	virtual float     GetAudBasePrml() const                  { return ART_PROMILLES_AUDIO_OR_SVC_ONLY_MPMRX; }
	virtual float     GetVidBasePrml() const                  { return VID_TOTAL_HD720_30FS_PROMILLES_MPMRX; }

	virtual DWORD     GetNumConfiguredARTPorts() const;
	virtual DWORD     GetNumConfiguredVideoPorts() const;

private:

	virtual STATUS    FindConfigureArtUnit(RSRCALLOC_UNIT_CONFIG_PARAMS_S* pConfigParams, CM_UNITS_CONFIG_S* pRetConfigParams, WORD card_type);
	virtual STATUS    FindConfigureVideoUnit(RSRCALLOC_UNIT_CONFIG_PARAMS_S* pConfigParams, CM_UNITS_CONFIG_S* pRetConfigParams, WORD card_type);

	static float      m_configProportion;
};


////////////////////////////////////////////////////////////////////////////
//                        CCardResourceConfigSoft
////////////////////////////////////////////////////////////////////////////
class CCardResourceConfigSoft : public CCardResourceConfig
{
	CLASS_TYPE_1(CCardResourceConfigSoft, CCardResourceConfig)

public:
	                  CCardResourceConfigSoft(eCardType cardType) : CCardResourceConfig(cardType){ }
	virtual          ~CCardResourceConfigSoft(){ }

	virtual float     GetConfigProportion()                   { return m_configProportion; }
	static void       SetConfigProportion( float configProp ) { m_configProportion = configProp; }

	virtual float     GetAudBasePrml() const;
	virtual float     GetVidBasePrml() const;

	virtual DWORD     GetNumConfiguredARTPorts() const;
	virtual DWORD     GetNumConfiguredVideoPorts() const;

private:
	virtual STATUS    FindConfigureArtUnit(RSRCALLOC_UNIT_CONFIG_PARAMS_S* pConfigParams, CM_UNITS_CONFIG_S* pRetConfigParams, WORD card_type);
	virtual STATUS    FindConfigureVideoUnit(RSRCALLOC_UNIT_CONFIG_PARAMS_S* pConfigParams, CM_UNITS_CONFIG_S* pRetConfigParams, WORD card_type);

	static float      m_configProportion;
};


////////////////////////////////////////////////////////////////////////////
//                        CCardResourceConfigNinja
////////////////////////////////////////////////////////////////////////////
class CCardResourceConfigNinja : public CCardResourceConfig
{
	CLASS_TYPE_1(CCardResourceConfigNinja, CCardResourceConfig)
public:
	                  CCardResourceConfigNinja(eCardType cardType) : CCardResourceConfig(cardType){ }
	virtual          ~CCardResourceConfigNinja(){ }

	virtual float     GetConfigProportion()                   { return m_configProportion; }
	static void       SetConfigProportion( float configProp ) { m_configProportion = configProp; }

	virtual float     GetAudBasePrml() const;
	virtual float     GetVidBasePrml() const;

	virtual DWORD     GetNumConfiguredARTPorts() const;
	virtual DWORD     GetNumConfiguredVideoPorts() const;

private:
	virtual STATUS    FindConfigureArtUnit(RSRCALLOC_UNIT_CONFIG_PARAMS_S* pConfigParams, CM_UNITS_CONFIG_S* pRetConfigParams, WORD card_type);
	virtual STATUS    FindConfigureVideoUnit(RSRCALLOC_UNIT_CONFIG_PARAMS_S* pConfigParams, CM_UNITS_CONFIG_S* pRetConfigParams, WORD card_type);

	static float      m_configProportion;
};


#endif /*CARDRESOURCECONFIG_H_*/
