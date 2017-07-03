// RtmIsdnMngrProcess.h

#ifndef RTM_ISDN_MNGR_PROCESS_H_
#define RTM_ISDN_MNGR_PROCESS_H_

#include "ProcessBase.h"

class CRtmIsdnServiceList;
class CRtmIsdnService;
class CRtmIsdnSpanMapList;
class CRtmIsdnSpanMap;
class CSlotsNumberingConversionTableWrapper;

class CRtmIsdnMngrProcess : public CProcessBase
{
  CLASS_TYPE_1(CRtmIsdnMngrProcess, CProcessBase)
public:
  friend class CTestRtmIsdnMngrProcess;

  CRtmIsdnMngrProcess();
  virtual ~CRtmIsdnMngrProcess();
  virtual eProcessType                   GetProcessType() {return eProcessRtmIsdnMngr;}
  virtual BOOL                           UsingSockets()   {return NO;}
  virtual TaskEntryPoint                 GetManagerEntryPoint();
  virtual void                           AddExtraStringsToMap();

  CRtmIsdnServiceList*                   GetServiceListOriginal() const;
  CRtmIsdnServiceList*                   GetServiceListUpdated() const;
  CRtmIsdnSpanMapList*                   GetSpanMapList() const;

  void                                   InitMediaBoardsIds();
  int                                    Get1stMediaBoardId();
  int                                    Get2ndMediaBoardId();
  int                                    Get3rdMediaBoardId();
  int                                    Get4thMediaBoardId();

  void                                   InitRtmIsdnBoardsIds();
  int                                    Get1stRtmIsdnBoardId();
  int                                    Get2ndRtmIsdnBoardId();
  int                                    Get3rdRtmIsdnBoardId();
  int                                    Get4thRtmIsdnBoardId();

  void                                   SetSlotsNumberingConversionTable(const SLOTS_NUMBERING_CONVERSION_TABLE_S* pTable);
  CSlotsNumberingConversionTableWrapper* GetSlotsNumberingConversionTable();
  void                                   PrintSlotsNumberingConversionTableData(const std::string theCaller);

private:
  CRtmIsdnServiceList* m_pServiceListOriginal;
  CRtmIsdnServiceList* m_pServiceListUpdated;
  CRtmIsdnSpanMapList* m_pSpanMapList;

  int m_1stMediaBoardId;
  int m_2ndMediaBoardId;
  int m_3rdMediaBoardId;
  int m_4thMediaBoardId;

  int m_1stRtmIsdnBoardId;
  int m_2ndRtmIsdnBoardId;
  int m_3rdRtmIsdnBoardId;
  int m_4thRtmIsdnBoardId;

  CSlotsNumberingConversionTableWrapper* m_pSlotsNumConversionTable;
};

#endif
