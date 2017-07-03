//
//************************************************************************
//   (c) Polycom
//
//  Name:   McmsNetworkProcess.h
//
//  Description:
/*
 *
 *
 *	 Flags:
 *
 *   Documentation List :
 *   HLD-
 *   	1. http://isrportal07/sites/rd/carmel/Restricted%20Documents/SW%20Department/Management/V100/ReWrite/Startup%20New%20design.pptx
 *   TestList
 *
 */
//
//  Revision History:
//
//  Date            Author           Functional/Interface Changes
//  -----------    -------------    ----------------------------------------
//  July 18, 2013    stanny			  created class
//************************************************************************
#ifndef MCMSNETWORK_PROCESS_H_
#define MCMSNETWORK_PROCESS_H_


#include "ProcessBase.h"
#include "Macros.h"

class CMcmsNetworkProcess : public CProcessBase, CNonCopyable
{
  CLASS_TYPE_1(CMcmsNetworkProcess, CProcessBase)
  friend class CTestMcmsNetworkProcess;

 public:
  CMcmsNetworkProcess();
  virtual               ~CMcmsNetworkProcess();
  virtual eProcessType   GetProcessType() {return eProcessMcmsNetwork;}
  virtual BOOL           UsingSockets()   {return NO;}
  virtual TaskEntryPoint GetManagerEntryPoint();
  BOOL GivesAwayRootUser() {return FALSE;}
  void AddExtraStatusesStrings(); //override base virtual function
  BOOL                 HasWatchDogTask() {return FALSE;}
  BOOL                 HasMonitorTask() {return FALSE;}
};

#endif

